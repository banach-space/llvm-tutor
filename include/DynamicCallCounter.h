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

//------------------------------------------------------------------------------
// Legacy PM interface
//------------------------------------------------------------------------------
struct DynamicCallCounter : public llvm::ModulePass {
  static char ID;
  bool runOnModule(llvm::Module &M);
  DynamicCallCounter() : llvm::ModulePass(ID) {}

  // A mapping between all function defined and declared in this module and
  // unique IDs.
  llvm::DenseMap<llvm::Function *, uint64_t> IDs;
  // A set of Functions in the current module, M, with internal linkage, i.e.
  // defined in M (i.e. doesn't contain functions that are only declared in M).
  llvm::DenseSet<llvm::Function *> InternalFuncs;

  // Installs a call to Counter at the beginning of Func. The input parameter
  // is set to IDs[&Func]. This is used for functions defined in M.
  // TODO: The 2nd argument should be factored out.
  void installIncrCC(llvm::Function &Func, llvm::Value *Counter);
  // Installs a call to Counter before the given call-site instruction. The
  // input parameter is set to IDs[&Func]. This is used for functions defined
  // in a separate module
  // TODO: The 2nd argument should be factored out.
  void installCCInstruction(llvm::CallSite CS, llvm::Value *Counter);
};
#endif
