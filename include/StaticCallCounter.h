//========================================================================
// FILE:
//    StaticCallCounter.h
//
// DESCRIPTION:
//    Declares the StaticCallCounter Passes
//      * new pass manager interface
//      * legacy pass manager interface
//      * printer pass for the new pass manager
//
// License: MIT
//========================================================================
#ifndef LLVM_TUTOR_STATICCALLCOUNTER_H
#define LLVM_TUTOR_STATICCALLCOUNTER_H

#include "llvm/ADT/MapVector.h"
#include "llvm/IR/AbstractCallSite.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"

//------------------------------------------------------------------------------
// New PM interface
//------------------------------------------------------------------------------
using ResultStaticCC = llvm::MapVector<const llvm::Function *, unsigned>;

struct StaticCallCounter : public llvm::AnalysisInfoMixin<StaticCallCounter> {
  using Result = ResultStaticCC;
  Result run(llvm::Module &M, llvm::ModuleAnalysisManager &);
  Result runOnModule(llvm::Module &M);
  // Part of the official API:
  //  https://llvm.org/docs/WritingAnLLVMNewPMPass.html#required-passes
  static bool isRequired() { return true; }

private:
  // A special type used by analysis passes to provide an address that
  // identifies that particular analysis pass type.
  static llvm::AnalysisKey Key;
  friend struct llvm::AnalysisInfoMixin<StaticCallCounter>;
};

//------------------------------------------------------------------------------
// New PM interface for the printer pass
//------------------------------------------------------------------------------
class StaticCallCounterPrinter
    : public llvm::PassInfoMixin<StaticCallCounterPrinter> {
public:
  explicit StaticCallCounterPrinter(llvm::raw_ostream &OutS) : OS(OutS) {}
  llvm::PreservedAnalyses run(llvm::Module &M,
                              llvm::ModuleAnalysisManager &MAM);
  // Part of the official API:
  //  https://llvm.org/docs/WritingAnLLVMNewPMPass.html#required-passes
  static bool isRequired() { return true; }

private:
  llvm::raw_ostream &OS;
};

//------------------------------------------------------------------------------
// Legacy PM interface
//------------------------------------------------------------------------------
struct LegacyStaticCallCounter : public llvm::ModulePass {
  static char ID;
  LegacyStaticCallCounter() : llvm::ModulePass(ID) {}
  bool runOnModule(llvm::Module &M) override;
  // The print method must be implemented by Legacy analysis passes in order to
  // print a human readable version of the analysis results:
  //    http://llvm.org/docs/WritingAnLLVMPass.html#the-print-method
  void print(llvm::raw_ostream &OutS, llvm::Module const *M) const override;

  ResultStaticCC DirectCalls;
  StaticCallCounter Impl;
};

#endif // LLVM_TUTOR_STATICCALLCOUNTER_H
