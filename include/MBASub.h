//========================================================================
// FILE:
//    MBASub.h
//
// DESCRIPTION:
//    Declares the MBASub pass for the new and the legacy pass managers.
//
// License: MIT
//========================================================================
#ifndef LLVM_TUTOR_MBA_SUB_H
#define LLVM_TUTOR_MBA_SUB_H

#include "llvm/IR/PassManager.h"
#include "llvm/Pass.h"

// PassInfoMixIn is a CRTP mix-in to automatically provide informational APIs
// needed for passes. Currently it provides only the 'name' method.
struct MBASub : public llvm::PassInfoMixin<MBASub> {
  llvm::PreservedAnalyses run(llvm::Function &F,
                              llvm::FunctionAnalysisManager &);
  bool runOnBasicBlock(llvm::BasicBlock &B);

  // Without isRequired returning true, this pass will be skipped for functions
  // decorated with the optnone LLVM attribute. Note that clang -O0 decorates
  // all functions with optnone.
  static bool isRequired() { return true; }
};

struct LegacyMBASub : public llvm::FunctionPass {
  // The address of this static is used to uniquely identify this pass in the
  // pass registry. The PassManager relies on this address to find instance of
  // analyses passes and build dependencies on demand.
  // The value does not matter.
  static char ID;
  LegacyMBASub() : FunctionPass(ID) {}
  bool runOnFunction(llvm::Function &F) override;

  MBASub Impl;
};
#endif
