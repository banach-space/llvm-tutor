//=============================================================================
// FILE:
//    MergeBB.cpp
//
// DESCRIPTION:
//  Merges identical basic blocks into one. As an example, consider the CFG
//  below. After the transformation, BB1 is merged into BB2.
//  ----------------------------------------------------------------------------
//  CFG BEFORE:                         CFG AFTER:
//  ----------------------------------------------------------------------------
//  [BB0] [other BBs]                   [BB0] [other BBs]                      |
//     \     |                             \     |                             |
//    [BB1][BB2] [other BBs]                ---[BB2] [other BBs]               |
//       \   |    /                              |    /                        |
//      [  BBsucc  ]                         [  BBsucc  ]                      |
//       /   |   \                            /   |   \                        V
//  ----------------------------------------------------------------------------
//  Only qualifying basic blocks are merged. The edge(s) from (potentially
//  multiple) BB0 to BB1, must be one of the following instructions:
//    * conditional branch
//    * unconditional branch, and
//    * switch
//  For the edges from BB1 to BBsucc and BB2 to BBsucc, only unconditional
//  branch instructions are allowed. Finally, BB1 is identical to BB2 iff all
//  instructions in BB1 are identical to the instructions in BB2. For finer
//  details please consult the implementation.
//
//  This pass will to some extent revert the modifications introduced by
//  DuplicateBB. The qualifying clones (lt-clone-1-BBId and lt-clone-2-BBid)
//  *will indeed* be merged, but the lt-if-then-else and lt-tail blocks (also
//  introduced by DuplicateBB) will be updated, but not removed. Please keep
//  this in mind when running the passes in a chain.
//
// USAGE:
//    $ opt -load-pass-plugin <BUILD_DIR>/lib/libMergeBB.so `\`
//      -passes=merge-bb -S <bitcode-file>
//
// License: MIT
//=============================================================================
#include "MergeBB.h"

#include "llvm/IR/IntrinsicInst.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"

#include "llvm/Transforms/Utils/BasicBlockUtils.h"

#include "llvm/ADT/Statistic.h"
#include "llvm/Support/Debug.h"

using namespace llvm;

#define DEBUG_TYPE "MergeBB"

STATISTIC(NumDedupBBs, "Number of basic blocks merged");
STATISTIC(OverallNumOfUpdatedBranchTargets, "Number of updated branch targets");

//-----------------------------------------------------------------------------
// MergeBB Implementation
//-----------------------------------------------------------------------------
bool MergeBB::canRemoveInst(const Instruction *Inst) {
  assert(Inst->hasOneUse() && "Inst needs to have exactly one use");

  auto *PNUse = dyn_cast<PHINode>(*Inst->user_begin());
  auto *Succ = Inst->getParent()->getTerminator()->getSuccessor(0);
  auto *User = cast<Instruction>(*Inst->user_begin());

  bool SameParentBB = (User->getParent() == Inst->getParent());
  bool UsedInPhi = (PNUse && PNUse->getParent() == Succ &&
                    PNUse->getIncomingValueForBlock(Inst->getParent()) == Inst);

  return UsedInPhi || SameParentBB;
}

bool MergeBB::canMergeInstructions(ArrayRef<Instruction *> Insts) {
  const Instruction *Inst1 = Insts[0];
  const Instruction *Inst2 = Insts[1];

  if (!Inst1->isSameOperationAs(Inst2))
    return false;

  // Each instruction must have exactly zero or one use.
  bool HasUse = !Inst1->user_empty();
  for (auto *I : Insts) {
    if (HasUse && !I->hasOneUse())
      return false;
    if (!HasUse && !I->user_empty())
      return false;
  }

  // Not all instructions that have one use can be merged. Make sure that
  // instructions that have one use can be safely deleted.
  if (HasUse) {
    if (!canRemoveInst(Inst1) || !canRemoveInst(Inst2))
      return false;
  }

  // Make sure that Inst1 and Inst2 have identical operands.
  assert(Inst2->getNumOperands() == Inst1->getNumOperands());
  auto NumOpnds = Inst1->getNumOperands();
  for (unsigned OpndIdx = 0; OpndIdx != NumOpnds; ++OpndIdx) {
    if (Inst2->getOperand(OpndIdx) != Inst1->getOperand(OpndIdx))
      return false;
  }

  return true;
}

// Get the number of non-debug instructions in BB
static unsigned getNumNonDbgInstrInBB(BasicBlock *BB) {
  unsigned Count = 0;
  for (Instruction &Instr : *BB)
    if (!isa<DbgInfoIntrinsic>(Instr))
      Count++;
  return Count;
}

unsigned MergeBB::updateBranchTargets(BasicBlock *BBToErase, BasicBlock *BBToRetain) {
  SmallVector<BasicBlock *, 8> BBToUpdate(predecessors(BBToErase));

  LLVM_DEBUG(dbgs() << "DEDUP BB: merging duplicated blocks ("
                    << BBToErase->getName() << " into " << BBToRetain->getName()
                    << ")\n");

  unsigned UpdatedTargetsCount = 0;
  for (BasicBlock *BB0 : BBToUpdate) {
    // The terminator is either a branch (conditional or unconditional) or a
    // switch statement. One of its targets should be BBToErase. Replace
    // that target with BBToRetain.
    Instruction *Term = BB0->getTerminator();
    for (unsigned OpIdx = 0, NumOpnds = Term->getNumOperands();
         OpIdx != NumOpnds; ++OpIdx) {
      if (Term->getOperand(OpIdx) == BBToErase) {
        Term->setOperand(OpIdx, BBToRetain);
        UpdatedTargetsCount++;
      }
    }
  }

  return UpdatedTargetsCount;
}

bool MergeBB::mergeDuplicatedBlock(BasicBlock *BB1,
                                   SmallPtrSet<BasicBlock *, 8> &DeleteList) {
  // Do not optimize the entry block
  if (BB1 == &BB1->getParent()->getEntryBlock())
    return false;

  // Only merge CFG edges of unconditional branch
  BranchInst *BB1Term = dyn_cast<BranchInst>(BB1->getTerminator());
  if (!(BB1Term && BB1Term->isUnconditional()))
    return false;

  // Do not optimize non-branch and non-switch CFG edges (to keep things
  // relatively simple)
  for (auto *B : predecessors(BB1))
    if (!(isa<BranchInst>(B->getTerminator()) ||
          isa<SwitchInst>(B->getTerminator())))
      return false;

  BasicBlock *BBSucc = BB1Term->getSuccessor(0);

  BasicBlock::iterator II = BBSucc->begin();
  const PHINode *PN = dyn_cast<PHINode>(II);
  Value *InValBB1 = nullptr;
  Instruction *InInstBB1 = nullptr;
  BBSucc->getFirstNonPHI();
  if (nullptr != PN) {
    // Do not optimize if multiple PHI instructions exist in the successor (to
    // keep things relatively simple)
    if (++II != BBSucc->end() && isa<PHINode>(II))
      return false;

    InValBB1 = PN->getIncomingValueForBlock(BB1);
    InInstBB1 = dyn_cast<Instruction>(InValBB1);
  }

  unsigned BB1NumInst = getNumNonDbgInstrInBB(BB1);
  for (auto *BB2 : predecessors(BBSucc)) {
    // Do not optimize the entry block
    if (BB2 == &BB2->getParent()->getEntryBlock())
      continue;

    // Only merge CFG edges of unconditional branch
    BranchInst *BB2Term = dyn_cast<BranchInst>(BB2->getTerminator());
    if (!(BB2Term && BB2Term->isUnconditional()))
      continue;

    // Do not optimize non-branch and non-switch CFG edges (to keep things
    // relatively simple)
    for (auto *B : predecessors(BB2))
      if (!(isa<BranchInst>(B->getTerminator()) ||
            isa<SwitchInst>(B->getTerminator())))
        continue;

    // Skip basic blocks that have already been marked for merging
    if (DeleteList.end() != DeleteList.find(BB2))
      continue;

    // Make sure that BB2 != BB1
    if (BB2 == BB1)
      continue;

    // BB1 and BB2 are definitely different if the number of instructions is
    // not identical
    if (BB1NumInst != getNumNonDbgInstrInBB(BB2))
      continue;

    // Control flow can be merged if incoming values to the PHI node
    // at the successor are same values or both defined in the BBs to merge.
    // For the latter case, canMergeInstructions executes further analysis.
    if (nullptr != PN) {
      Value *InValBB2 = PN->getIncomingValueForBlock(BB2);
      Instruction *InInstBB2 = dyn_cast<Instruction>(InValBB2);

      bool areValuesSimilar = (InValBB1 == InValBB2);
      bool bothValuesDefinedInParent =
          ((InInstBB1 && InInstBB1->getParent() == BB1) &&
           (InInstBB2 && InInstBB2->getParent() == BB2));
      if (!areValuesSimilar && !bothValuesDefinedInParent)
        continue;
    }

    // Finally, check that all instructions in BB1 and BB2 are identical
    LockstepReverseIterator LRI(BB1, BB2);
    while (LRI.isValid() && canMergeInstructions(*LRI)) {
      --LRI;
    }

    // Valid iterator  means that a mismatch was found in middle of BB
    if (LRI.isValid())
      continue;

    // It is safe to de-duplicate - do so.
    unsigned UpdatedTargets = updateBranchTargets(BB1, BB2);
    assert(UpdatedTargets && "No branch target was updated");
    OverallNumOfUpdatedBranchTargets += UpdatedTargets;
    DeleteList.insert(BB1);
    NumDedupBBs++;

    return true;
  }

  return false;
}

PreservedAnalyses MergeBB::run(llvm::Function &Func,
                               llvm::FunctionAnalysisManager &) {
  bool Changed = false;
  SmallPtrSet<BasicBlock *, 8> DeleteList;
  for (auto &BB : Func) {
    Changed |= mergeDuplicatedBlock(&BB, DeleteList);
  }

  for (BasicBlock *BB : DeleteList) {
    DeleteDeadBlock(BB);
  }

  return (Changed ? llvm::PreservedAnalyses::none()
                  : llvm::PreservedAnalyses::all());
}

//-----------------------------------------------------------------------------
// New PM Registration
//-----------------------------------------------------------------------------
llvm::PassPluginLibraryInfo getMergeBBPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "MergeBB", LLVM_VERSION_STRING,
          [](PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](StringRef Name, FunctionPassManager &FPM,
                   ArrayRef<PassBuilder::PipelineElement>) {
                  if (Name == "merge-bb") {
                    FPM.addPass(MergeBB());
                    return true;
                  }
                  return false;
                });
          }};
}

extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
  return getMergeBBPluginInfo();
}

//------------------------------------------------------------------------------
// Helper data structures
//------------------------------------------------------------------------------
LockstepReverseIterator::LockstepReverseIterator(BasicBlock *BB1In,
                                                 BasicBlock *BB2In)
    : BB1(BB1In), BB2(BB2In), Fail(false) {
  Insts.clear();

  Instruction *InstBB1 = getLastNonDbgInst(BB1);
  if (nullptr == InstBB1)
    Fail = true;

  Instruction *InstBB2 = getLastNonDbgInst(BB2);
  if (nullptr == InstBB2)
    Fail = true;

  Insts.push_back(InstBB1);
  Insts.push_back(InstBB2);
}

Instruction *LockstepReverseIterator::getLastNonDbgInst(BasicBlock *BB) {
  Instruction *Inst = BB->getTerminator();

  do {
    Inst = Inst->getPrevNode();
  } while (Inst && isa<DbgInfoIntrinsic>(Inst));

  return Inst;
}

void LockstepReverseIterator::operator--() {
  if (Fail)
    return;

  for (auto *&Inst : Insts) {
    do {
      Inst = Inst->getPrevNode();
    } while (Inst && isa<DbgInfoIntrinsic>(Inst));

    if (!Inst) {
      // Already at the beginning of BB
      Fail = true;
      return;
    }
  }
}
