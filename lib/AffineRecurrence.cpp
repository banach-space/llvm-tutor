/* AffineRecurrence.cpp 
 *
 * This pass detects affine recurrences using ScalarEvolution.
 *
 * Compatible with New Pass Manager
*/

#include "llvm/IR/PassManager.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Value.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"

using namespace llvm;

namespace {

class AffineRecurrence 
    : public PassInfoMixin<AffineRecurrence> {
public:
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &AM) {
    auto &LI = AM.getResult<LoopAnalysis>(F);
    auto &SE = AM.getResult<ScalarEvolutionAnalysis>(F);

    for (Loop *L : LI) {
      errs() << "Analyzing loop in function " << F.getName() << ":\n";

      for (auto *BB : L->getBlocks()) {
        for (auto &I : *BB) {
//          if (!I.getType()->isIntegerTy())
//            continue;
          const SCEV *S = SE.getSCEV(&I);

          // Detect affine AddRec expressions
          if (auto *AR = dyn_cast<SCEVAddRecExpr>(S)) {
            const SCEV *Step = AR->getStepRecurrence(SE);
            const SCEV *Start = AR->getStart();

            // Check if it's affine
            if (AR->isAffine()) {
              errs() << "  Affine recurrence: " << I.getName()
                     << " = {" << *Start << ",+," << *Step << "}<"
                     << L->getHeader()->getName() << ">\n";
            }
          }
        }
      }
    }

    return PreservedAnalyses::all();
  }
};

} // namespace

// Register the pass
llvm::PassPluginLibraryInfo getAffineRecurrencePluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "AffineRecurrence", LLVM_VERSION_STRING,
          [](PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](StringRef Name, FunctionPassManager &FPM,
                   ArrayRef<PassBuilder::PipelineElement>) {
                  if (Name == "affine-recurrence") {
                    FPM.addPass(AffineRecurrence());
                    return true;
                  }
                  return false;
                });
          }};
}

extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
  return getAffineRecurrencePluginInfo();
}

