//========================================================================
// FILE:
//    MBAAdd.h
//
// DESCRIPTION:
//    Declares the MBAAdd pass for the new and the legacy pass managers.
//
// License: MIT
//========================================================================
#ifndef LLVM_TUTOR_MBA_ADD_H
#define LLVM_TUTOR_MBA_ADD_H

#include "llvm/IR/PassManager.h"
#include "llvm/Pass.h"

namespace lt {
// PassInfoMixIn is a CRTP mix-in to automatically provide informational APIs
// needed for passes. Currently it provides only the 'name' method.
struct MBAAdd : public llvm::PassInfoMixin<MBAAdd> {
  llvm::PreservedAnalyses run(llvm::Function &F,
                              llvm::FunctionAnalysisManager &);
  bool runOnBasicBlock(llvm::BasicBlock &B);
};

struct LegacyMBAAdd : public llvm::FunctionPass {
  // The address of this static is used to uniquely identify this pass in the
  // pass registry. The PassManager relies on this address to find instance of
  // analyses passes and build dependencies on demand.
  // The value does not matter.
  static char ID;
  LegacyMBAAdd() : FunctionPass(ID) {}
  bool runOnFunction(llvm::Function &F) override;

  MBAAdd Impl;
};
} // namespace lt

#endif
