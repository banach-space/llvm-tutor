//========================================================================
// FILE:
//    RIV.h
//
// DESCRIPTION:
//    Declares the RIV passes:
//      * new pass manager interface
//      * legacy pass manager interface
//      * printer pass for the new pass manager
//
// License: MIT
//========================================================================
#ifndef LLVM_TUTOR_RIV_H
#define LLVM_TUTOR_RIV_H

#include "llvm/ADT/MapVector.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/ValueMap.h"
#include "llvm/Pass.h"

//------------------------------------------------------------------------------
// New PM interface
//------------------------------------------------------------------------------
struct RIV : public llvm::AnalysisInfoMixin<RIV> {
  // A map that for every basic block holds a set of pointers to reachable
  // integer values for that block.
  using Result = llvm::MapVector<llvm::BasicBlock const *,
                                 llvm::SmallPtrSet<llvm::Value *, 8>>;
  Result run(llvm::Function &F, llvm::FunctionAnalysisManager &);
  Result buildRIV(llvm::Function &F,
                  llvm::DomTreeNodeBase<llvm::BasicBlock> *CFGRoot);

private:
  // A special type used by analysis passes to provide an address that
  // identifies that particular analysis pass type.
  static llvm::AnalysisKey Key;
  friend struct llvm::AnalysisInfoMixin<RIV>;
};

//------------------------------------------------------------------------------
// New PM interface for the printer pass
//------------------------------------------------------------------------------
class RIVPrinter : public llvm::PassInfoMixin<RIVPrinter> {
public:
  explicit RIVPrinter(llvm::raw_ostream &OutS) : OS(OutS) {}
  llvm::PreservedAnalyses run(llvm::Function &F,
                              llvm::FunctionAnalysisManager &FAM);

private:
  llvm::raw_ostream &OS;
};

//------------------------------------------------------------------------------
// Legacy PM interface
//------------------------------------------------------------------------------
struct LegacyRIV : public llvm::FunctionPass {
  static char ID;
  LegacyRIV() : FunctionPass(ID) {}
  bool runOnFunction(llvm::Function &) override;
  void print(llvm::raw_ostream &O, llvm::Module const *) const override;
  void getAnalysisUsage(llvm::AnalysisUsage &Info) const override;

  RIV::Result const &getRIVMap() const { return RIVMap; }

  // The actual mapping computed by this pass. Note that for every invocation
  // of the compiler, only one instance of this pass is created. This means
  // that one instance of RIVMap is re-used every-time this pass is run.
  RIV::Result RIVMap;

  RIV Impl;
};

#endif // LLVM_TUTOR_RIV_H
