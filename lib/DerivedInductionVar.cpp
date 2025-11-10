#include "llvm/IR/PassManager.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Constants.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"

#include <optional>
using std::optional;
using std::nullopt;

using namespace llvm;

namespace {

class DerivedInductionVarPass : public PassInfoMixin<DerivedInductionVarPass> {
public:
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &FAM) {
    auto &LI = FAM.getResult<LoopAnalysis>(F);
    auto &SE = FAM.getResult<ScalarEvolutionAnalysis>(F);

    errs() << "===== Derived Induction Variable Analysis =====\n";
    for (auto *L : LI) {
      analyzeLoopAndInner(L, SE);
    }

    return PreservedAnalyses::all();
  }

private:
  void analyzeLoopAndInner(Loop *L, ScalarEvolution &SE) {
    if (!L)
      return;

    if (L->getSubLoops().empty())
      errs() << "Analyzing innermost loop at depth " << L->getLoopDepth()
             << " in function.\n";
    else
      errs() << "Analyzing outer loop with " << L->getSubLoops().size()
             << " inner loops.\n";

    analyzeLoop(L, SE);

    // recursively handle inner loops
    for (auto *SubLoop : L->getSubLoops()) {
      analyzeLoopAndInner(SubLoop, SE);
    }
  }

  void analyzeLoop(Loop *L, ScalarEvolution &SE) {
    errs() << "  Loop header: ";
    if (L->getHeader())
      errs() << L->getHeader()->getName();
    errs() << "\n";

    #
	for (auto &I : L->getHeader()->instructionsWithoutDebug()) {
    	Instruction *Inst = &I;
    	if (!isa<PHINode>(Inst))
        	continue;
    	auto *PN = cast<PHINode>(Inst);

    	const SCEV *S = SE.getSCEV(PN);
    	if (const auto *AR = dyn_cast<SCEVAddRecExpr>(S)) {
        	if (AR->isAffine()) {
            	errs() << "    Found affine recurrence IV: " << *PN << "\n";
            	errs() << "      Expression: " << *AR << "\n";
            	errs() << "      Start = " << *AR->getStart() << "\n";
            	errs() << "      Step = " << *AR->getStepRecurrence(SE) << "\n";
        	}
    	   }
	}


#
  }
};

} // namespace

//===----------------------------------------------------------------------===//
// Pass registration
//===----------------------------------------------------------------------===//

llvm::PassPluginLibraryInfo getDerivedInductionVarPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "DerivedInductionVar", LLVM_VERSION_STRING,
          [](PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](StringRef Name, FunctionPassManager &FPM,
                   ArrayRef<PassBuilder::PipelineElement>) {
                  if (Name == "derived-indvar") {
                    FPM.addPass(DerivedInductionVarPass());
                    return true;
                  }
                  return false;
                });
          }};
}

extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
  return getDerivedInductionVarPluginInfo();
}

