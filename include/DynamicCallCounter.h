//========================================================================
// FILE:
//    DynamicCallCounter.h
//
// AUTHOR:
//    banach-space@github
//
// DESCRIPTION:
//    This pass cannnot be used through opt as it calculates the function calls
//    at runtime.
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
#include "llvm/Support/raw_ostream.h"

namespace lt {

struct DynamicCallCounter : public llvm::ModulePass {
  static char ID;

  llvm::DenseMap<llvm::Function *, uint64_t> ids;
  llvm::DenseSet<llvm::Function *> internal;

  DynamicCallCounter() : llvm::ModulePass(ID) {}

  bool runOnModule(llvm::Module &m) override;

  // Installs the call-counter (CC) at the the beginning of the given function
  void installCCFunction(llvm::Function &f, llvm::Value *counter);
  // Installs the call-counter (CC) before the given call-site instruction
  void installCCInstruction(llvm::CallSite cs, llvm::Value *counter);
};

}  // namespace lt

#endif
