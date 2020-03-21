//==============================================================================
//  FILE:
//    DuplicateBB.cpp
//
//  DESCRIPTION:
//    For the input function F, DuplicateBB first creates a list of BasicBlocks
//    that are suitable for cloning and then clones them. A BasicBlock BB is
//    suitable for cloning iff its set of RIVs (Reachable Integer Values) is
//    non-empty.
//
//    If BB is suitable for cloning, two new basic blocks are created that
//    replace BB: BB-if-then and BB-else. In order to decide which new
//    BasicBlock to jump to, an if-then-else construct is inserted where
//    originally BB would start:
//      if (var == 0)
//        goto BB-if-then
//      else
//        goto BB-else
//    `var` is a randomly chosen variable from the RIV set for BB. If `var` is
//    a GlobalValue (i.e. global variable), then BB won't be duplicated
//    (because global variables are often constant, and constant values lead to
//    trivial `if` conditions, e.g. if (some_const_val == 0)).
//
//  ALGORITHM:
//    --------------------------------------------------------------------------
//    The following CFG graph presents function 'F' before and after applying
//    DuplicateBB. BB0 is the entry block. Assume that:
//      * F takes no arguments (i.e. BB0 is not duplicated)
//      * BB1 and BB1 are suitable for cloning
//      * var_BB1 and var_BB2 are integers from the sets of RIVs for BB1 and
//        BB2, respectively
//    --------------------------------------------------------------------------
//    F - BEFORE     F - AFTER
//    --------------------------------------------------------------------------
//      BB0             BB0
//       |               |
//       v               v
//      BB1     if (var_BB1 == 0)     <---- inserted by DuplicateBB
//       |              / \
//       |            /     \
//       |     BB1-if-then  BB1-else  <---- clones of BB1
//       |            \     /
//       |              \ /
//       |               v
//       v             TAIL           <---- PHIs to merge BB1-if-then & BB1-else
//      BB2      if (var_BB2 == 0)    <---- inserted by DuplicateBB
//       |              / \
//       |            /     \
//       |      BB2-then   BB2-else   <---- clones of BB2
//       |            \     /
//       |              \ /
//       |               v
//       v             TAIL           <---- PHIs to merge BB2-if-then & BB2-else
//     TERM            TERM           <---- terminator instruction
//    --------------------------------------------------------------------------
//
// License: MIT
//==============================================================================
#include "DuplicateBB.h"

#include "llvm/ADT/Statistic.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/Transforms/Utils/Cloning.h"

#include <random>

#define DEBUG_TYPE "duplicate-bb"

STATISTIC(DuplicateBBCount, "The # of duplicated blocks");

using namespace llvm;

//------------------------------------------------------------------------------
// DuplicateBB Implementation
//------------------------------------------------------------------------------
DuplicateBB::BBToSingleRIVMap
DuplicateBB::findBBsToDuplicate(Function &F, const RIV::Result &RIVResult) {
  BBToSingleRIVMap BlocksToDuplicate;

  // Get a random number generator. This will be used to choose a context
  // value for the injected `if-then-else` construct.
  // FIXME Switch to 'F.getParent()->createRNG("DuplicateBB")' once possible.
  // This patch implements the necessary API:
  //    * https://reviews.llvm.org/rG73713f3e5ef2ecf1e5afafa89f76ab89cc06b18e
  // It should be available in LLVM 11.
  std::random_device RD;
  std::mt19937_64 RNG(RD());

  for (BasicBlock &BB : F) {
    // Basic blocks which are landing pads are used for handling exceptions.
    // That's out of scope of this pass.
    if (BB.isLandingPad())
      continue;

    // Get the set of RIVs for this block
    auto const &ReachableValues = RIVResult.lookup(&BB);
    size_t ReachableValuesCount = ReachableValues.size();

    // Are there any RIVs for this BB? We need at least one to be able to
    // duplicate this BB.
    if (0 == ReachableValuesCount) {
      LLVM_DEBUG(errs() << "No context values for this BB\n");
      continue;
    }

    // Get a random context value from the RIV set
    auto Iter = ReachableValues.begin();
    std::uniform_int_distribution<> Dist(0, ReachableValuesCount - 1);
    std::advance(Iter, Dist(RNG));

    if (dyn_cast<GlobalValue>(*Iter)) {
      LLVM_DEBUG(errs() << "Random context value is a global variable. "
                        << "Skipping this BB\n");
      continue;
    }

    LLVM_DEBUG(errs() << "Random context value: " << **Iter << "\n");

    // Store the binding between the current BB and the context variable that
    // will be used for the `if-then-else` construct.
    BlocksToDuplicate.emplace_back(&BB, *Iter);
  }

  return BlocksToDuplicate;
}

void DuplicateBB::cloneBB(BasicBlock &BB, Value *ContextValue,
                          ValueToPhiMap &ReMapper) {
  // Don't duplicate Phi nodes - start right after them
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

  assert(Tail == ElseTerm->getSuccessor(0) && "Inconsistent CFG");

  // Give the new basic blocks some meaningful names. This is not required, but
  // makes the output easier to read.
  std::string DuplicatedBBId = std::to_string(DuplicateBBCount);
  ThenTerm->getParent()->setName("lt-clone-1-" + DuplicatedBBId);
  ElseTerm->getParent()->setName("lt-clone-2-" + DuplicatedBBId);
  Tail->setName("lt-tail-" + DuplicatedBBId);
  ThenTerm->getParent()->getSinglePredecessor()->setName("lt-if-then-else-" +
                                                         DuplicatedBBId);

  // Variables to keep track of the new bindings
  ValueToValueMapTy TailVMap, ThenVMap, ElseVMap;

  // The list of instructions in Tail that don't produce any values and thus
  // can be removed
  SmallVector<Instruction *, 8> ToRemove;

  // Iterate through the original basic block and clone every instruction into
  // the 'if-then' and 'else' branches. Update the bindings/uses on the fly
  // (through ThenVMap, ElseVMap, TailVMap). At this stage, all instructions
  // apart from PHI nodes, are stored in Tail.
  for (auto IIT = Tail->begin(), IE = Tail->end(); IIT != IE; ++IIT) {
    Instruction &Instr = *IIT;
    assert(!isa<PHINode>(&Instr) && "Phi nodes have already been filtered out");

    // Skip terminators - duplicating them wouldn't make sense unless we want
    // to delete Tail completely.
    if (Instr.isTerminator()) {
      RemapInstruction(&Instr, TailVMap, RF_IgnoreMissingLocals);
      continue;
    }

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
      continue;
    }

    // Instruction that produce a value should not require a slot in the
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

  // Purge instructions that don't produce any value
  for (auto *I : ToRemove)
    I->eraseFromParent();

  ++DuplicateBBCount;
}

PreservedAnalyses DuplicateBB::run(llvm::Function &F,
                                   llvm::FunctionAnalysisManager &FAM) {
  BBToSingleRIVMap Targets = findBBsToDuplicate(F, FAM.getResult<RIV>(F));

  // This map is used to keep track of the new bindings. Otherwise, the
  // information from RIV will become obsolete.
  ValueToPhiMap ReMapper;

  // Duplicate
  for (auto &BB_Ctx : Targets) {
    cloneBB(*std::get<0>(BB_Ctx), std::get<1>(BB_Ctx), ReMapper);
  }

  return (Targets.empty() ? llvm::PreservedAnalyses::none()
                          : llvm::PreservedAnalyses::all());
}

bool LegacyDuplicateBB::runOnFunction(llvm::Function &F) {
  // Find BBs to duplicate
  DuplicateBB::BBToSingleRIVMap Targets =
      Impl.findBBsToDuplicate(F, getAnalysis<LegacyRIV>().getRIVMap());

  // This map is used to keep track of the new bindings. Otherwise, the
  // information from RIV will become obsolete.
  DuplicateBB::ValueToPhiMap ReMapper;

  // Duplicate
  for (auto &BB_Ctx : Targets) {
    Impl.cloneBB(*std::get<0>(BB_Ctx), std::get<1>(BB_Ctx), ReMapper);
  }

  return (Targets.empty() ? false : true);
}

//------------------------------------------------------------------------------
// New PM Registration
//------------------------------------------------------------------------------
llvm::PassPluginLibraryInfo getDuplicateBBPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "duplicate-bb", LLVM_VERSION_STRING,
          [](PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](StringRef Name, FunctionPassManager &FPM,
                   ArrayRef<PassBuilder::PipelineElement>) {
                  if (Name == "duplicate-bb") {
                    FPM.addPass(DuplicateBB());
                    return true;
                  }
                  return false;
                });
          }};
}

extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
  return getDuplicateBBPluginInfo();
}

//------------------------------------------------------------------------------
// Legacy PM Registration
//------------------------------------------------------------------------------
// This method defines how this pass interacts with other passes
void LegacyDuplicateBB::getAnalysisUsage(AnalysisUsage &Info) const {
  Info.addRequired<LegacyRIV>();
}

char LegacyDuplicateBB::ID = 0;
static RegisterPass<LegacyDuplicateBB> X(/*PassArg=*/"legacy-duplicate-bb",
                                         /*Name=*/"Duplicate Basic Blocks Pass",
                                         /*CFGOnly=*/false,
                                         /*is_analysis=*/false);
