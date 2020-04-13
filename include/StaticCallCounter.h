//========================================================================
// FILE:
//    StaticCallCounter.h
//
// DESCRIPTION:
//    Declares the StaticCallCounter Pass
//
// License: MIT
//========================================================================
#ifndef LLVM_TUTOR_STATICCALLCOUNTER_H
#define LLVM_TUTOR_STATICCALLCOUNTER_H

#include "llvm/ADT/MapVector.h"
#include "llvm/IR/CallSite.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"

using ResultStaticCC = llvm::MapVector<const llvm::Function *, unsigned>;

//------------------------------------------------------------------------------
// New PM interface
//------------------------------------------------------------------------------
struct StaticCallCounter : public llvm::AnalysisInfoMixin<StaticCallCounter> {
  using Result = ResultStaticCC;
  Result run(llvm::Module &M, llvm::ModuleAnalysisManager &);
  Result runOnModule(llvm::Module &M);

  // A special type used by analysis passes to provide an address that
  // identifies that particular analysis pass type.
  static llvm::AnalysisKey Key;
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

//------------------------------------------------------------------------------
// Helper functions
//------------------------------------------------------------------------------
// Pretty-prints the result of this analysis
void printStaticCCResult(llvm::raw_ostream &OutS,
                         const ResultStaticCC &DirectCalls);

#endif
