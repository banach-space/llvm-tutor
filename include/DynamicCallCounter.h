//========================================================================
// FILE:
//    DynamicCallCounter.h
//
// DESCRIPTION:
//   Declares the DynamicCallCounter.pass
//
// License: MIT
//========================================================================
#ifndef LLVM_TUTOR_DYNAMICCALLCOUNTER_H
#define LLVM_TUTOR_DYNAMICCALLCOUNTER_H

#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/DenseSet.h"
#include "llvm/IR/CallSite.h"
#include "llvm/IR/Module.h"
#include "llvm/Pass.h"

namespace lt {

struct DynamicCallCounter : public llvm::ModulePass {
  static char ID;

  llvm::DenseMap<llvm::Function *, uint64_t> IDs;
  llvm::DenseSet<llvm::Function *> Internal;

  DynamicCallCounter() : llvm::ModulePass(ID) {}

  bool runOnModule(llvm::Module &M) override;

  // Installs the call-counter (CC) at the the beginning of the given function
  void installCCFunction(llvm::Function &Func, llvm::Value *Counter);
  // Installs the call-counter (CC) before the given call-site instruction
  void installCCInstruction(llvm::CallSite CS, llvm::Value *Counter);
};

} // namespace lt

#endif
