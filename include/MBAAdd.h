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

#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Pass.h"

//------------------------------------------------------------------------------
// New PM interface
//------------------------------------------------------------------------------
struct MBAAdd : public llvm::PassInfoMixin<MBAAdd> {
  llvm::PreservedAnalyses run(llvm::Function &F,
                              llvm::FunctionAnalysisManager &);
  bool runOnBasicBlock(llvm::BasicBlock &B);

  // Without isRequired returning true, this pass will be skipped for functions
  // decorated with the optnone LLVM attribute. Note that clang -O0 decorates
  // all functions with optnone.
  static bool isRequired() { return true; }
};

#endif
