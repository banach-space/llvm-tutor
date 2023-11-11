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

#include "llvm/IR/PassManager.h"
#include "llvm/Pass.h"
#include <vector>

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
  // This is one of the standard run() member functions expected by
  // PassInfoMixin. When the pass is executed by the new PM, this is the
  // function that will be called.
  Result run(llvm::Function &Func, llvm::FunctionAnalysisManager &FAM);
  // This is a helper run() member function overload which can be called by the
  // legacy pass (or any other code) without having to supply a
  // FunctionAnalysisManager argument.
  Result run(llvm::Function &Func);

private:
  friend struct llvm::AnalysisInfoMixin<FindFCmpEq>;
  static llvm::AnalysisKey Key;
};

//------------------------------------------------------------------------------
// New PM interface for the printer pass
//------------------------------------------------------------------------------
class FindFCmpEqPrinter : public llvm::PassInfoMixin<FindFCmpEqPrinter> {
public:
  explicit FindFCmpEqPrinter(llvm::raw_ostream &OutStream) : OS(OutStream){};

  llvm::PreservedAnalyses run(llvm::Function &Func,
                              llvm::FunctionAnalysisManager &FAM);

private:
  llvm::raw_ostream &OS;
};

#endif // !LLVM_TUTOR_FIND_FCMP_EQ_H
