/* SimpleLICM.cpp
 *
 * This pass hoists loop-invariant code before the loop when it is safe to do so.
 * Compatible with New Pass Manager.
 */

#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/CFG.h"

#include "llvm/Pass.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"

#include "llvm/Analysis/LoopAnalysisManager.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/LoopPass.h"
#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/Analysis/ValueTracking.h"

#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/Transforms/Utils/LoopUtils.h"
#include "llvm/Transforms/Utils/ValueMapper.h"

using namespace llvm;

struct SimpleLICM : public PassInfoMixin<SimpleLICM> {

  // helper funct checking loop invariance
  bool isLoopInvariant(Instruction &I, Loop *L, const SmallPtrSetImpl<Instruction *> &InvariantSet) {
    if (isa<PHINode>(&I))
      return false;

    if (!isSafeToSpeculativelyExecute(&I))
      return false;

    // Check all operands
    for (Value *Op : I.operands()) {
      if (Instruction *OpInst = dyn_cast<Instruction>(Op)) {
        if (L->contains(OpInst) && !InvariantSet.contains(OpInst))
          return false;
      }
    }

    // If all checks pass, it's loop invariant
    return true;
  }

  PreservedAnalyses run(Loop &L, LoopAnalysisManager &AM,
                        LoopStandardAnalysisResults &AR,
                        LPMUpdater &) {
    DominatorTree &DT = AR.DT;

    BasicBlock *Preheader = L.getLoopPreheader();
    if (!Preheader) {
      errs() << "No preheader, skipping loop\n";
      return PreservedAnalyses::all();
    }

    SmallPtrSet<Instruction *, 8> InvariantSet;
    bool Changed = true;

    // worklist alg to identify loop invariant instructions
    while (Changed) {
      Changed = false;
      for (BasicBlock *BB : L.blocks()) {
        for (Instruction &I : *BB) {
	   errs() << "Examining: " << I << "\n";  // Debug printtttt
          // Skip if already marked invariant
          if (InvariantSet.contains(&I))
            continue;

          if (isLoopInvariant(I, &L, InvariantSet)) {
            errs() << "  -> Marked invariant!\n";  // Debug print
	    InvariantSet.insert(&I);
            Changed = true;
          }
        }
      }
    }

    // hoist instr
    for (Instruction *I : InvariantSet) {
      if (isSafeToSpeculativelyExecute(I) && dominatesAllLoopExits(I, &L, DT)) {
        errs() << "Hoisting: " << *I << "\n";
        I->moveBefore(Preheader->getTerminator());
      }
    }

    return PreservedAnalyses::none();
  }

  bool dominatesAllLoopExits(Instruction *I, Loop *L, DominatorTree &DT) {
    SmallVector<BasicBlock *, 8> ExitBlocks;
    L->getExitBlocks(ExitBlocks);
    for (BasicBlock *EB : ExitBlocks) {
      if (!DT.dominates(I, EB))
        return false;
    }
    return true;
  }
};

llvm::PassPluginLibraryInfo getSimpleLICMPluginInfo() {
  errs() << "SimpleLICM plugin: getSimpleLICMPluginInfo() called\n";
  return {LLVM_PLUGIN_API_VERSION, "simple-licm", LLVM_VERSION_STRING,
          [](PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](StringRef Name, LoopPassManager &LPM,
                   ArrayRef<PassBuilder::PipelineElement>) {
                  if (Name == "simple-licm") {
                    LPM.addPass(SimpleLICM());
                    return true;
                  }
                  return false;
                });
          }};
}

extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
  errs() << "SimpleLICM plugin: llvmGetPassPluginInfo() called\n";
  return getSimpleLICMPluginInfo();
}

