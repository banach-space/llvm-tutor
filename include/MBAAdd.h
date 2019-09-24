//==============================================================================
// FILE:
//    MBAAdd.h
//
// DESCRIPTION:
//    Declares the MBAAdd pass for the new and the legacy pass managers.
//
// License: MIT
//==============================================================================
#ifndef LLVM_TUTOR_MBA_ADD_H
#define LLVM_TUTOR_MBA_ADD_H

#include "llvm/IR/PassManager.h"
#include "llvm/Pass.h"

//------------------------------------------------------------------------------
// New PM interface
//------------------------------------------------------------------------------
struct MBAAdd : public llvm::PassInfoMixin<MBAAdd> {
  llvm::PreservedAnalyses run(llvm::Function &F,
                              llvm::FunctionAnalysisManager &);
  bool runOnBasicBlock(llvm::BasicBlock &B);
};

//------------------------------------------------------------------------------
// Legacy PM interface
//------------------------------------------------------------------------------
struct LegacyMBAAdd : public llvm::FunctionPass {
  static char ID;
  LegacyMBAAdd() : FunctionPass(ID) {}
  bool runOnFunction(llvm::Function &F) override;

  MBAAdd Impl;
};

#endif
