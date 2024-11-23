//========================================================================
// FILE:
//    MergeBB.h
//
// DESCRIPTION:
//    Declares the MergeBB Pass
//
// License: MIT
//========================================================================
#ifndef LLVM_TUTOR_MERGEBBS_H
#define LLVM_TUTOR_MERGEBBS_H

#include "llvm/IR/Instruction.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Pass.h"

using ResultMergeBB = llvm::StringMap<unsigned>;

//------------------------------------------------------------------------------
// New PM interface
//------------------------------------------------------------------------------
struct MergeBB : public llvm::PassInfoMixin<MergeBB> {
  using Result = ResultMergeBB;
  llvm::PreservedAnalyses run(llvm::Function &F,
                              llvm::FunctionAnalysisManager &);

  // Checks whether the input instruction Inst (that has exactly one use) can be
  // removed. This is the case when its only user is either:
  //  1) a PHI (it can be easily updated if Inst is removed), or
  //  2) located in the same block as Inst (if that block is removed then the
  //     user will also be removed)
  bool canRemoveInst(const llvm::Instruction *Inst);

  // Instructions in Insts belong to different blocks that unconditionally
  // branch to a common successor. Analyze them and return true if it would be
  // possible to merge them, i.e. replace Inst1 with Inst2 (or vice-versa).
  bool canMergeInstructions(llvm::ArrayRef<llvm::Instruction *> Insts);

  // Replace the destination of incoming edges of BBToErase by BBToRetain
  unsigned updateBranchTargets(llvm::BasicBlock *BBToErase,
                               llvm::BasicBlock *BBToRetain);

  // If BB is duplicated, then merges BB with its duplicate and adds BB to
  // DeleteList. DeleteList contains the list of blocks to be deleted.
  bool
  mergeDuplicatedBlock(llvm::BasicBlock *BB,
                       llvm::SmallPtrSet<llvm::BasicBlock *, 8> &DeleteList);

  // Without isRequired returning true, this pass will be skipped for functions
  // decorated with the optnone LLVM attribute. Note that clang -O0 decorates
  // all functions with optnone.
  static bool isRequired() { return true; }
};

//------------------------------------------------------------------------------
// Helper data structures
//------------------------------------------------------------------------------
// Iterates through instructions in BB1 and BB2 in reverse order from the first
// non-debug instruction. For example (assume all blocks have size n):
//   LockstepReverseIterator I(BB1, BB2);
//   *I-- = [BB1[n], BB2[n]];
//   *I-- = [BB1[n-1], BB2[n-1]];
//   *I-- = [BB1[n-2], BB2[n-2]];
//   ...
class LockstepReverseIterator {
  llvm::BasicBlock *BB1;
  llvm::BasicBlock *BB2;

  llvm::SmallVector<llvm::Instruction *, 2> Insts;
  bool Fail;

public:
  LockstepReverseIterator(llvm::BasicBlock *BB1In, llvm::BasicBlock *BB2In);

  llvm::Instruction *getLastNonDbgInst(llvm::BasicBlock *BB);
  bool isValid() const { return !Fail; }

  void operator--();

  llvm::ArrayRef<llvm::Instruction *> operator*() const { return Insts; }
};
#endif
