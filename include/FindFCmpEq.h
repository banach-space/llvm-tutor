//==============================================================================
// FILE:
//    FindFCmpEq.h
//
// DESCRIPTION:
//    Declares the FindFCmpEq pass for the new and the legacy pass managers as
//    well as a printing pass for the new pass manager.
//
// License: MIT
//==============================================================================

#ifndef LLVM_TUTOR_FIND_FCMP_EQ_H
#define LLVM_TUTOR_FIND_FCMP_EQ_H

#include <vector>

#include "llvm/IR/PassManager.h"
#include "llvm/Pass.h"

// Forward declarations
namespace llvm {

class FCmpInst;
class Function;
class Module;
class raw_ostream;

} // namespace llvm

//------------------------------------------------------------------------------
// New PM interface
//------------------------------------------------------------------------------
class FindFCmpEq : public llvm::AnalysisInfoMixin<FindFCmpEq> {
public:
  using Result = std::vector<llvm::FCmpInst *>;
  Result run(llvm::Function &Func, llvm::FunctionAnalysisManager &FAM);
  Result run(llvm::Function &Func);

private:
  friend class llvm::AnalysisInfoMixin<FindFCmpEq>;
  static llvm::AnalysisKey Key;
};

//------------------------------------------------------------------------------
// New PM interface for the printer pass
//------------------------------------------------------------------------------
class FindFCmpEqPrinter : public llvm::PassInfoMixin<FindFCmpEqPrinter> {
public:
  explicit FindFCmpEqPrinter(llvm::raw_ostream &OutStream);

  llvm::PreservedAnalyses run(llvm::Function &Func,
                              llvm::FunctionAnalysisManager &FAM);

private:
  llvm::raw_ostream &OS;
};

//------------------------------------------------------------------------------
// Legacy PM interface
//------------------------------------------------------------------------------
class FindFCmpEqWrapper : public llvm::FunctionPass {
public:
  static char ID;

  FindFCmpEqWrapper();

  const FindFCmpEq::Result &getComparisons() const noexcept;

  bool runOnFunction(llvm::Function &F) override;
  void getAnalysisUsage(llvm::AnalysisUsage &AU) const override;
  void print(llvm::raw_ostream &OS,
             const llvm::Module *M = nullptr) const override;

private:
  FindFCmpEq::Result Results;
};

#endif // !LLVM_TUTOR_FIND_FCMP_EQ_H
