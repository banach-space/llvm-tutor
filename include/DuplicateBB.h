//========================================================================
// FILE:
//    DuplicateBB.h
//
// DESCRIPTION:
//    Declares the DuplicateBB pass.
//
// License: MIT
//========================================================================
#ifndef LLVM_TUTOR_DUPLICATE_BB_H
#define LLVM_TUTOR_DUPLICATE_BB_H

#include "llvm/IR/ValueMap.h"
#include "llvm/Pass.h"
#include "llvm/Support/RandomNumberGenerator.h"

#include "Ratio.h"

#include <map>

namespace lt {

struct DuplicateBB : public llvm::FunctionPass {
  // The address of this static is used to uniquely identify this pass in the
  // pass registry. The PassManager relies on this address to find instance of
  // analyses passes and build dependencies on demand.
  // The value does not matter.
  static char ID;
  DuplicateBB() : llvm::FunctionPass(ID), RNG(nullptr) {}
  bool runOnFunction(llvm::Function &F) override;
  bool doInitialization(llvm::Module &M) override;
  void getAnalysisUsage(llvm::AnalysisUsage &Info) const override;

  std::unique_ptr<llvm::RandomNumberGenerator> RNG;

private:
  // This method does most of the actual heavy-lifting.
  void duplicate(llvm::BasicBlock &BB, llvm::Value *ContextValue,
                 std::map<llvm::Value *, llvm::Value *> &ReMapper);
};
} // namespace lt

#endif
