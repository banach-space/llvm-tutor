//==============================================================================
//  FILE:
//    DuplicateBB.cpp
//
//  DESCRIPTION:
//    Obfuscation through duplication of Basic blocks guarded This clones some
//    basic blocks, guarded by a condition that depends on the context. Each
//    branch should then be obfuscated in different ways.
//
//  ALGORITHM:
//    ----------------------------------------------------------
//    The following CFG graphs present function 'F' before and after applying
//    DuplicateBB. BB0 is the entry block. Assume that there are no global
//    variables and F takes no arguments, hence BB0 is not split.
//    ----------------------------------------------------------
//    F - BEFORE         F - AFTER
//    ----------------------------------------------------------
//      BB0                BB0
//       |                  |
//       v                  v
//      BB1             bool cond. 1   <----  some condition
//       |                 / \
//       |               /     \
//       |         BB1-then   BB1-else <---- clones of BB1
//       |               \     /
//       |                 \ /
//       |                  v
//       v                TAIL         <---- PHI nodes that merge two branches
//      BB2             bool cond. 2   <---- another bool cond.
//       |                 / \
//       |               /     \
//       |         BB2-then   BB2-else <---- clones of BB2
//       |               \     /
//       |                 \ /
//       |                  v
//       v                TAIL         <---- PHI nodes that merge two branches
//     TERM               TERM         <---- terminator instruction
//    ----------------------------------------------------------
//
// License: MIT
//==============================================================================
#include "DuplicateBB.h"

#include "llvm/ADT/Statistic.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/Module.h"
#include "llvm/Pass.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/Transforms/Utils/Cloning.h"

#include "RIV.h"
#include "Ratio.h"

#define DEBUG_TYPE "duplicate-bb"

STATISTIC(DuplicateBBCount, "The # of duplicated blocks");

// Pass Option declaration
static llvm::cl::opt<Ratio> DuplicateBBRatio{
    "duplicate-bb-ratio",
    llvm::cl::desc("Only apply the duplicate basic block "
                   "pass on <ratio> of the basic blocks"),
    llvm::cl::value_desc("ratio"), llvm::cl::init(1.), llvm::cl::Optional};

using namespace llvm;
using lt::DuplicateBB;

namespace lt {
char DuplicateBB::ID = 0;

// Register the pass - required for (among others) opt
static RegisterPass<lt::DuplicateBB>
    X("duplicate-bb", "Duplicate Basic Blocks Pass",
      false, // does modify the CFG => false
      false  // not a pure analysis pass => false
    );
} // namespace lt

// Called once for each module (i.e. before the pass is run on any on any
// function)
bool DuplicateBB::doInitialization(Module &M) {
  RNG = M.createRNG(this);
  return false;
}

bool DuplicateBB::runOnFunction(Function &F) {
  // Radio and Dist are used to decide whether to duplicate the current BB - it
  // allows control from the user (Ratio is the CL argument).
  double const Ratio = DuplicateBBRatio.getValue().getRatio();
  std::uniform_real_distribution<double> Dist(0., 1.);

  // Get the result of RIV
  auto const &RIVResult = getAnalysis<RIV>().getRIVMap();

  // The list of BBs to duplicate. For each BB, also stores a context variable
  // that can be used for the boolean condition for the 'if-then-else'
  // construct. It's a randomly picked variable that's reachable in the current
  // BB (that's why RIV is needed here). The actual doesn't matter.
  std::vector<std::tuple<BasicBlock *, Value *>> Targets;

  // Run over all BBs in F and find the one that are suitable for duplication.
  for (BasicBlock &BB : F) {
    // Basic blocks which are landing pads are used for handling exceptions.
    // That's out of scope of this pass.
    if (BB.isLandingPad())
      continue;

    if (Dist(*RNG) <= Ratio) {
      // Are there any integer values reachable from this BB?
      auto const &ReachableValues = RIVResult.lookup(&BB);
      size_t ReachableValuesCount = ReachableValues.size();
      if (0 != ReachableValuesCount) {
        // Yes, pick a random one.
        std::uniform_int_distribution<size_t> Dist(0, ReachableValuesCount - 1);
        auto Iter = ReachableValues.begin();
        std::advance(Iter, Dist(*RNG));
        LLVM_DEBUG(errs() << "picking: " << **Iter
                          << " as random context value\n");
        // Store the binding and a BB to duplicate and the context variable
        // used to hide it
        Targets.emplace_back(&BB, *Iter);

        ++DuplicateBBCount;
      } else {
        LLVM_DEBUG(errs() << "no context value found\n");
      }
    }
  }

  // This pass modifies values and so it needs to keep track of the new
  // bindings. Otherwise, the information from RIV will be obsolete.
  std::map<Value *, Value *> ReMapper;

  // Finally, duplicate
  for (auto &BB_Ctx : Targets) {
    duplicate(*std::get<0>(BB_Ctx), std::get<1>(BB_Ctx), ReMapper);
  }

  // Has anything been modified?
  return !Targets.empty();
}

void DuplicateBB::duplicate(BasicBlock &BB, Value *ContextValue,
                            std::map<Value *, Value *> &ReMapper) {
  // Don't duplicate phi nodes - start right after them
  Instruction *BBHead = BB.getFirstNonPHI();

  // Create the condition for 'if-then-else'
  IRBuilder<> Builder(BBHead);
  Value *Cond = Builder.CreateIsNull(
      ReMapper.count(ContextValue) ? ReMapper[ContextValue] : ContextValue);

  // Create and insert the 'if-else' blocks. At this point both blocks are
  // trivial and contain only one terminator instruction branching to BB's
  // tail, which contains all the instructions from BBHead onwards.
  Instruction *ThenTerm = nullptr;
  Instruction *ElseTerm = nullptr;
  SplitBlockAndInsertIfThenElse(Cond, &*BBHead, &ThenTerm, &ElseTerm);

  BasicBlock *Tail = ThenTerm->getSuccessor(0);
  assert(Tail == ElseTerm->getSuccessor(0));

  // To keep track of the new bindings.
  ValueToValueMapTy TailVMap, ThenVMap, ElseVMap;

  // The list of instructions in Tail that don't produce any values and can be
  // removed.
  SmallVector<Instruction *, 8> ToRemove;

  // Iterate through the original basic block and clone every instruction into
  // the 'then' and 'else' branches. Update the bindings/uses as on the fly
  // (through ThenVMap, ElseVMap, TailVMap). At this stage, all the instruction
  // apart from PHI nodes, are stored in Tail.
  for (auto IIT = Tail->begin(), IE = Tail->end(); IIT != IE; ++IIT) {
    Instruction &Instr = *IIT;
    assert(!isa<PHINode>(&Instr) && "phi nodes have already been filtered out");

    // Skip terminators - duplicating them wouldn't make sense unless we want
    // to delete Tail completely.
    if (Instr.isTerminator()) {
      RemapInstruction(&Instr, TailVMap, RF_IgnoreMissingLocals);
      continue;
    }

    // once the instruction is cloned, its operand still hold reference to
    // the original basic block
    // we want them to refer to the cloned one! The mappings are used for
    // this
    // Clone the instructions.
    Instruction *ThenClone = Instr.clone(), *ElseClone = Instr.clone();

    // Operands of ThenClone still hold references to the original BB.
    // Update/remap them.
    RemapInstruction(ThenClone, ThenVMap, RF_IgnoreMissingLocals);
    ThenClone->insertBefore(ThenTerm);
    ThenVMap[&Instr] = ThenClone;

    // Operands of ElseClone still hold references to the original BB.
    // Update/remap them.
    RemapInstruction(ElseClone, ElseVMap, RF_IgnoreMissingLocals);
    ElseClone->insertBefore(ElseTerm);
    ElseVMap[&Instr] = ElseClone;

    // Instructions that don't produce values can be safely removed from Tail
    if (ThenClone->getType()->isVoidTy()) {
      ToRemove.push_back(&Instr);
    } else {
      // instruction that produce a value should not require a slot in the
      // TAIL *but* they can be used from the context, so just always
      // generate a PHI, and let further optimization do the cleaning
      PHINode *Phi = PHINode::Create(ThenClone->getType(), 3);
      Phi->addIncoming(ThenClone, ThenTerm->getParent());
      Phi->addIncoming(ElseClone, ElseTerm->getParent());
      TailVMap[&Instr] = Phi;

      ReMapper[&Instr] = Phi;

      // Instructions are modified as we go, use the iterator version of
      // ReplaceInstWithInst.
      ReplaceInstWithInst(Tail->getInstList(), IIT, Phi);
    }
  }

  // Purge instructions that don't produce any value
  for (auto *I : ToRemove)
    I->eraseFromParent();
}

// Some guidance for PassManager:
//    * addRequired<RIV>() - needs the results from RIV Pass
// More info:
// http://llvm.org/docs/WritingAnLLVMPass.html#specifying-interactions-between-passes
void DuplicateBB::getAnalysisUsage(AnalysisUsage &Info) const {
  Info.addRequired<RIV>();
}
