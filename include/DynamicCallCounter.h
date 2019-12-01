//==============================================================================
// FILE:
//    DynamicCallCounter.h
//
// DESCRIPTION:
//    Declares the DynamicCallCounter pass for the new and the legacy pass
//    managers.
//
// License: MIT
//==============================================================================
#ifndef LLVM_TUTOR_INSTRUMENT_BASIC_H
#define LLVM_TUTOR_INSTRUMENT_BASIC_H

#include "llvm/IR/PassManager.h"
#include "llvm/Pass.h"

//------------------------------------------------------------------------------
// New PM interface
//------------------------------------------------------------------------------
struct DynamicCallCounter : public llvm::PassInfoMixin<DynamicCallCounter> {
  llvm::PreservedAnalyses run(llvm::Module &M,
                              llvm::ModuleAnalysisManager &);
  bool runOnModule(llvm::Module &M);
};

//------------------------------------------------------------------------------
// Legacy PM interface
//------------------------------------------------------------------------------
struct LegacyDynamicCallCounter : public llvm::ModulePass {
  static char ID;
  LegacyDynamicCallCounter() : ModulePass(ID) {}
  bool runOnModule(llvm::Module &M) override;

  DynamicCallCounter Impl;
};

#endif
