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

#include "llvm/IR/CallSite.h"
#include "llvm/IR/Module.h"
#include "llvm/Pass.h"

namespace lt {

struct StaticCallCounter : public llvm::ModulePass {
  static char ID;

  llvm::DenseMap<const llvm::Function *, unsigned> DirectCalls;

  StaticCallCounter() : llvm::ModulePass(ID) {}

  bool runOnModule(llvm::Module &M) override;

  void print(llvm::raw_ostream &OutS, llvm::Module const *M) const override;

private:
  // Checks whether CS is indeed a CallSite and then for CallSites incrementes
  // the counter for the corresponding function.
  void handleInstruction(llvm::ImmutableCallSite CS);
};

} // namespace lt

#endif
