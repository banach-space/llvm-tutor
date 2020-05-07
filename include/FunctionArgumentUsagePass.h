//==============================================================================
// FILE:
//    FunctionArgumentUsagePass.h
//
// DESCRIPTION:
//    An example of an analysis pass: reports any implicit casts around
//    function calls.
//    Declares the FunctionArgumentUsagePass pass and interface data types.
//
// License: MIT
//==============================================================================

#ifndef LLVM_TUTOR_FUNCTIONARGUMENTUSAGEPASS_H
#define LLVM_TUTOR_FUNCTIONARGUMENTUSAGEPASS_H

#include "llvm/ADT/iterator_range.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Pass.h"

namespace llvm {

class AnalysisUsage;
class CallBase;
class Function;
class FunctionPass;
class StringRef;
class Type;

} // namespace llvm

struct TypeMismatchRecord {
  llvm::StringRef callerName;
  unsigned line;
  bool hasLine;
  unsigned argNo;
  const llvm::Type *expectedType;
  const llvm::Type *actualType;

  TypeMismatchRecord(llvm::StringRef callerName, unsigned line, bool hasLine,
                     unsigned argNo, const llvm::Type *expectedType,
                     const llvm::Type *actualType)
      : callerName(callerName), line(line), hasLine(hasLine), argNo(argNo),
        expectedType(expectedType), actualType(actualType) {}
};

//------------------------------------------------------------------------------
// New PM interface
//------------------------------------------------------------------------------
class FunctionArgumentUsagePass
    : public llvm::AnalysisInfoMixin<FunctionArgumentUsagePass> {

  using TypeMismatchVector = llvm::SmallVector<TypeMismatchRecord, 6>;

  TypeMismatchVector typeMismatches;

  void analyzeFunctionUsages(llvm::Function &F, llvm::CallBase *call);

public:
  using const_mismatch_iterator = TypeMismatchVector::const_iterator;

  /// Provide the result typedef for this analysis pass.
  using Result = llvm::iterator_range<const_mismatch_iterator>;

  Result run(llvm::Function &F, llvm::FunctionAnalysisManager &);

  Result runOnFunction(llvm::Function &F);

  void releaseMemory();

  // A special type used by analysis passes to provide an address that
  // identifies that particular analysis pass type.
  // or .../llvm/IR/PassManager.h:409:23: error: no member named 'Key'
  // in 'FunctionArgumentUsagePass
  static llvm::AnalysisKey Key;
};

class LegacyFunctionArgumentUsagePass : public llvm::FunctionPass {
  FunctionArgumentUsagePass Impl;
  FunctionArgumentUsagePass::Result Result;

public:
  using const_mismatch_iterator = FunctionArgumentUsagePass::const_mismatch_iterator;

  static char ID;

  LegacyFunctionArgumentUsagePass()
      : llvm::FunctionPass(ID), Result(nullptr, nullptr)
  {}

  void getAnalysisUsage(llvm::AnalysisUsage &AU) const override {
    AU.setPreservesAll();
  }

  const_mismatch_iterator begin() const {
    return Result.begin();
  }

  const_mismatch_iterator end() const {
    return Result.end();
  }

  bool runOnFunction(llvm::Function &F) override;

  void print(llvm::raw_ostream &O, const llvm::Module *M) const override;

  void releaseMemory() override;
};


using TypeMismatchRange
  = llvm::iterator_range<FunctionArgumentUsagePass::const_mismatch_iterator>;

/// Helper functions
/// Pretty-prints the result of this analysis
void printTypeMismatches(llvm::raw_ostream &OS,
                         const TypeMismatchRange &TypeMismatches);

#endif // LLVM_TUTOR_FUNCTIONARGUMENTUSAGEPASS_H
