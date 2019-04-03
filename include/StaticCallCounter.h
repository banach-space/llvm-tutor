//========================================================================
// FILE:
//    StaticCallCounter.h
//
// AUTHOR:
//    banach-space@github
//
// DESCRIPTION:
//    Declares the StaticCallCounter Pass
//
// License: MIT
//========================================================================
#ifndef LLVM_TUTOR_STATICCALLCOUNTER_H
#define LLVM_TUTOR_STATICCALLCOUNTER_H

#include "llvm/ADT/DenseMap.h"
#include "llvm/IR/CallSite.h"
#include "llvm/IR/Module.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"

namespace lt {

struct StaticCallCounter : public llvm::ModulePass {
  static char ID;

  llvm::DenseMap<const llvm::Function *, uint64_t> counts;

  StaticCallCounter() : llvm::ModulePass(ID) {}

  bool runOnModule(llvm::Module &m) override;

  void print(llvm::raw_ostream &out, llvm::Module const *m) const override;

 private:
  // Checks whether cs is indeed a CallSite and then for CallSites incrementes
  // the counter for the corresponding function.
  void handleInstruction(llvm::ImmutableCallSite cs);
};

} // namespace lt

#endif
