//==============================================================================
// FILE:
//    DuplicateBB.h
//
// DESCRIPTION:
//    Declares the DuplicateBB pass
//
// License: MIT
//==============================================================================
#ifndef LLVM_TUTOR_DUPLICATE_BB_H
#define LLVM_TUTOR_DUPLICATE_BB_H

#include "RIV.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/PassManager.h"
#include "llvm/IR/ValueMap.h"
#include "llvm/Pass.h"

#include <map>

//------------------------------------------------------------------------------
// New PM interface
//------------------------------------------------------------------------------
struct DuplicateBB : public llvm::PassInfoMixin<DuplicateBB> {
  llvm::PreservedAnalyses run(llvm::Function &F,
                              llvm::FunctionAnalysisManager &);

  // Maps BB, a BasicBlock, to one integer value (defined in a different
  // BasicBlock) that's reachable in BB. The Value that BB is mapped to is used
  // in the `if-then-else` construct when cloning BB.
  using BBToSingleRIVMap =
      std::vector<std::tuple<llvm::BasicBlock *, llvm::Value *>>;
  // Maps a Value before duplication to a Phi node that merges the
  // corresponding values after duplication/cloning.
  using ValueToPhiMap = std::map<llvm::Value *, llvm::Value *>;

  // Creates a BBToSingleRIVMap of BasicBlocks that are suitable for cloning.
  BBToSingleRIVMap findBBsToDuplicate(llvm::Function &F,
                                      const RIV::Result &RIVResult);

  // Clones the input basic block:
  //  * injects an `if-then-else` construct using ContextValue
  //  * duplicates BB
  //  * adds PHI nodes as required
  void cloneBB(llvm::BasicBlock &BB, llvm::Value *ContextValue,
               ValueToPhiMap &ReMapper);

  unsigned DuplicateBBCount = 0;
};

//------------------------------------------------------------------------------
// Legacy PM interface
//------------------------------------------------------------------------------
struct LegacyDuplicateBB : public llvm::FunctionPass {
  static char ID;
  LegacyDuplicateBB() : llvm::FunctionPass(ID) {}
  bool runOnFunction(llvm::Function &F) override;
  void getAnalysisUsage(llvm::AnalysisUsage &Info) const override;

  DuplicateBB Impl;
};

#endif
