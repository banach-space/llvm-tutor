#include "llvm/IR/PassManager.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/LoopAnalysisManager.h"
#include "llvm/Analysis/LoopPass.h"
#include "llvm/Transforms/Utils/LoopUtils.h"
#include "llvm/Transforms/Scalar/LoopPassManager.h"
#include "llvm/Analysis/InstructionSimplify.h"
#include "llvm/Analysis/ValueTracking.h"     //  Correct header for isSafeToSpeculativelyExecute
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Support/raw_ostream.h"


using namespace llvm;

namespace {

struct SimpleLICM : PassInfoMixin<SimpleLICM> {
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

    // Identify loop-invariant instructions
    while (Changed) {
      Changed = false;
      for (BasicBlock *BB : L.blocks()) {
        for (Instruction &I : *BB) {
          errs() << "Examining: " << I << "\n";
          if (InvariantSet.contains(&I))
            continue;

          if (isLoopInvariant(I, &L, InvariantSet)) {
            errs() << "  -> Marked invariant!\n";
            InvariantSet.insert(&I);
            Changed = true;
          }
        }
      }
    }

    // Hoist loop-invariant instructions
    for (Instruction *I : InvariantSet) {
      if (isSafeToSpeculativelyExecute(I)) {
        errs() << "Hoisting: " << *I << "\n";
	I->moveBefore(Preheader->getTerminator());
	
      }
    }

    return PreservedAnalyses::none();
  }

  bool isLoopInvariant(Instruction &I, Loop *L,
                       const SmallPtrSetImpl<Instruction *> &Inv) {
    if (I.mayHaveSideEffects() || I.isTerminator())
      return false;
    for (Use &U : I.operands()) {
      if (auto *OpI = dyn_cast<Instruction>(U.get())) {
        if (L->contains(OpI) && !Inv.contains(OpI))
          return false;
      }
    }
    return true;
  }
};

} // end anonymous namespace

// Register the pass for opt (LLVM 21+)
llvm::PassPluginLibraryInfo getSimpleLICMPluginInfo() {
  return {
      LLVM_PLUGIN_API_VERSION, "SimpleLICM", LLVM_VERSION_STRING,
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

// This function is required by opt to load the pass dynamically.
extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
  return getSimpleLICMPluginInfo();
}

