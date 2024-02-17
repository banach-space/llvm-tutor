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
#include <memory>

namespace llvm {
class RandomNumberGenerator;
} // namespace llvm

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

  // Without isRequired returning true, this pass will be skipped for functions
  // decorated with the optnone LLVM attribute. Note that clang -O0 decorates
  // all functions with optnone.
  static bool isRequired() { return true; }

  std::unique_ptr<llvm::RandomNumberGenerator> pRNG;
};

#endif
