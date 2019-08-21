//========================================================================
// FILE:
//    RIV.h
//
// DESCRIPTION:
//    Declares the RIV pass
//
// License: MIT
//========================================================================
#ifndef LLVM_TUTOR_REACHABLE_INTEGER_VALUES_H
#define LLVM_TUTOR_REACHABLE_INTEGER_VALUES_H

#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/IR/ValueMap.h"
#include "llvm/Pass.h"

struct RIV : public llvm::FunctionPass {
  using RIVMapTy = llvm::ValueMap<llvm::BasicBlock const *,
                                  llvm::SmallPtrSet<llvm::Value *, 8>>;
  static char ID;

  RIV() : FunctionPass(ID) {}
  void getAnalysisUsage(llvm::AnalysisUsage &Info) const override;
  bool runOnFunction(llvm::Function &) override;
  RIVMapTy const &getRIVMap() const;
  void print(llvm::raw_ostream &O, llvm::Module const *) const override;

private:
  RIVMapTy RIVMap;
};
#endif
