#include "llvm/IR/PassManager.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"

using namespace llvm;

namespace {

class DerivedInductionVarElimPass : public PassInfoMixin<DerivedInductionVarElimPass> {
public:
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &FAM) {
    auto &LI = FAM.getResult<LoopAnalysis>(F);
    auto &SE = FAM.getResult<ScalarEvolutionAnalysis>(F);

    errs() << "===== Derived Induction Variable Elimination =====\n";
    for (auto *L : LI)
      processLoop(L, SE);

    return PreservedAnalyses::none();
  }

private:
  void processLoop(Loop *L, ScalarEvolution &SE) {
    if (!L)
      return;

    errs() << "Analyzing loop in function: " 
           << L->getHeader()->getParent()->getName() << "\n";

    SmallVector<PHINode *, 4> PHIs;
    for (auto &I : *L->getHeader())
      if (auto *PN = dyn_cast<PHINode>(&I))
        PHIs.push_back(PN);

    if (PHIs.empty())
      return;

    // Try to find a primary induction variable (canonical IV)
    PHINode *PrimaryIV = nullptr;
    const SCEVAddRecExpr *PrimaryAR = nullptr;

    for (auto *PN : PHIs) {
      const SCEV *S = SE.getSCEV(PN);
      if (const auto *AR = dyn_cast<SCEVAddRecExpr>(S)) {
        if (AR->isAffine()) {
          PrimaryIV = PN;
          PrimaryAR = AR;
          errs() << "Primary IV: " << *PN << "\n";
          break;
        }
      }
    }

    if (!PrimaryIV)
      return;

    for (auto *PN : PHIs) {
      if (PN == PrimaryIV)
        continue;

      const SCEV *S = SE.getSCEV(PN);
      const auto *DAR = dyn_cast<SCEVAddRecExpr>(S);
      if (!DAR || !DAR->isAffine())
        continue;

      // Check if this derived IV has same step as primary
      const SCEV *StepSCEV = DAR->getStepRecurrence(SE);
      const SCEV *PrimaryStepSCEV = PrimaryAR->getStepRecurrence(SE);

      // Structural equality of steps (conservative)
      if (StepSCEV->getSCEVType() != PrimaryStepSCEV->getSCEVType())
        continue;
      if (StepSCEV->getType() != PrimaryStepSCEV->getType())
        continue;

      // Compare step expressions by textual form
      std::string StepStr, PrimaryStr;
      {
        llvm::raw_string_ostream RSO1(StepStr), RSO2(PrimaryStr);
        StepSCEV->print(RSO1);
        PrimaryStepSCEV->print(RSO2);
      }
      if (StepStr != PrimaryStr)
        continue;

      // Compute delta = StartD - StartP
      const SCEV *StartD = DAR->getStart();
      const SCEV *StartP = PrimaryAR->getStart();
      const SCEV *Delta = SE.getMinusSCEV(StartD, StartP);

      // Only handle constant deltas
      if (!isa<SCEVConstant>(Delta))
        continue;

      const auto *DeltaC = cast<SCEVConstant>(Delta);
      const APInt &DeltaVal = DeltaC->getAPInt();
      auto *PrimaryTy = PrimaryIV->getType();

      // Create the replacement expression: PrimaryIV + Delta
      ConstantInt *CI = cast<ConstantInt>(ConstantInt::get(PrimaryTy, DeltaVal));

      for (auto &U : PN->uses()) {
        if (auto *UserI = dyn_cast<Instruction>(U.getUser())) {
          IRBuilder<> Builder(UserI);
          Value *NewVal = Builder.CreateAdd(PrimaryIV, CI, "repl.iv");
          UserI->replaceUsesOfWith(PN, NewVal);
        }
      }

      errs() << "Replaced derived IV: " << *PN << " with primary + " << *CI << "\n";
    }
  }
};

} // namespace

//===----------------------------------------------------------------------===//
// Pass registration
//===----------------------------------------------------------------------===//

llvm::PassPluginLibraryInfo getDerivedInductionVarElimPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "DerivedInductionVarElim", LLVM_VERSION_STRING,
          [](PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](StringRef Name, FunctionPassManager &FPM,
                   ArrayRef<PassBuilder::PipelineElement>) {
                  if (Name == "derived-indvar-elim") {
                    FPM.addPass(DerivedInductionVarElimPass());
                    return true;
                  }
                  return false;
                });
          }};
}

extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo llvmGetPassPluginInfo() {
  return getDerivedInductionVarElimPluginInfo();
}

