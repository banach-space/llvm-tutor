//========================================================================
// FILE:
//    MBA.h
//
// DESCRIPTION:
//    Declares the MBA pass.
//
// License: MIT
//========================================================================
#ifndef LLVM_TUTOR_MBA_H
#define LLVM_TUTOR_MBA_H

#include "llvm/Pass.h"
#include "llvm/Support/RandomNumberGenerator.h"

#include "Ratio.h"

namespace lt {

struct MBA : public llvm::BasicBlockPass {
  // The address of this static is used to uniquely identify this pass in the
  // pass registry. The PassManager relies on this address to find instance of
  // analyses passes and build dependencies on demand.
  // The value does not matter.
  static char ID;
  MBA() : BasicBlockPass(ID), RNG(nullptr) {}
  bool runOnBasicBlock(llvm::BasicBlock &BB) override;
  bool doInitialization(llvm::Module &M) override;

  std::unique_ptr<llvm::RandomNumberGenerator> RNG;
};
} // namespace lt

#endif
