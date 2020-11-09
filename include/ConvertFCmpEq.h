#ifndef LLVM_TUTOR_CONVERT_FCMP_EQ_H
#define LLVM_TUTOR_CONVERT_FCMP_EQ_H

#include "llvm/IR/PassManager.h"
#include "llvm/Pass.h"

#include "FindFCmpEq.h"

// Forward declarations
namespace llvm {

class Function;

} // namespace llvm

class ConvertFCmpEq : public llvm::PassInfoMixin<ConvertFCmpEq> {
public:
  llvm::PreservedAnalyses run(llvm::Function &Func,
                              llvm::FunctionAnalysisManager &FAM);
  bool run(llvm::Function &Func, FindFCmpEq::Result Comparisons);
};

struct ConvertFCmpEqWrapper : llvm::FunctionPass {
  static char ID;
  ConvertFCmpEqWrapper();

  bool runOnFunction(llvm::Function &Func) override;
  void getAnalysisUsage(llvm::AnalysisUsage &AU) const override;
};

#endif // !LLVM_TUTOR_CONVERT_FCMP_EQ_H
