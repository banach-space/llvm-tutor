//==============================================================================
// FILE:
//    MBA.cpp
//
// DESCRIPTION:
//    Obfuscation through Mixed Boolean Arithmetic (MBA)
//
//    This pass performs an instruction substitution based on this equality: 
//      a + b == (a ^ b) + 2 * (a & b)
//    See formula 2.2 in https://tel.archives-ouvertes.fr/tel-01623849/document
//
//    For LLVM_DEBUG support see:
//      http://llvm.org/docs/ProgrammersManual.html#the-llvm-debug-macro-and-debug-option
//    
//    For STATISTIC support see:
//      http://llvm.org/docs/WritingAnLLVMPass.html#pass-statistics
//      
//    For CommandLine support see:
//      http://llvm.org/docs/CommandLine.html#quick-start-guide
//
//    This pass can be run through opt:
//      $ opt -load <BUILD_DIR>/lib/liblt-libmba-shared.so --mba \
//        [-mba-ratio=<ratio>] <bitcode-file>
//    with the optional ratio in the range [0, 1.0].
//
// License: MIT
//==============================================================================
#include "MBA.h"

#include "llvm/ADT/Statistic.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/Module.h"
#include "llvm/Pass.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"

using namespace llvm;
using lt::MBA;

#define DEBUG_TYPE "mba"

STATISTIC(MBACount, "The # of substituted instructions");

// Pass Option declaration
static cl::opt<Ratio, false, llvm::cl::parser<Ratio>> MBARatio{
    "mba-ratio",
    cl::desc("Only apply the mba pass on <ratio> of the candidates"),
    cl::value_desc("ratio"), cl::init(1.), cl::Optional};

namespace lt {
char MBA::ID = 0;

// Register the pass - required for (among others) opt
static RegisterPass<MBA>
    X("mba",
      "Mixed Boolean Arithmetic Substitution",
      true, // doesn't modify the CFG => true
      false // not a pure analysis pass => false
    );
} // namespace lt

// Called once for each module, before the calls on basic blocks.
bool MBA::doInitialization(Module &M) {
  RNG = M.createRNG(this);
  return false;
}

bool MBA::runOnBasicBlock(BasicBlock &BB) {
  bool modified = false;
  std::uniform_real_distribution<double> Dist(0., 1.);

  // Can't use a for-range loop because we want to delete the instruction from
  // the list we're iterating when replacing it.
  for (auto IIT = BB.begin(), IE = BB.end(); IIT != IE; ++IIT) {
    Instruction &Inst = *IIT;

    auto *BinOp = dyn_cast<BinaryOperator>(&Inst);
    if (!BinOp)
      // The instruction is not a binary operator, skip.
      continue;

    if (Dist(*RNG) > MBARatio.getRatio())
      // Probabilistic replacement, skip if we are not in the threshold.
      continue;

    unsigned Opcode = BinOp->getOpcode();
    if (Opcode != Instruction::Add || !BinOp->getType()->isIntegerTy())
      continue;

    // 
    IRBuilder<> Builder(BinOp);

    // This creates an instruction representing `(a ^ b) + 2 * (a & b)`.
    // See: http://llvm.org/docs/ProgrammersManual.html#creating-and-inserting-new-instructions
    Instruction *NewValue2 = BinaryOperator::CreateAdd(
        Builder.CreateXor(BinOp->getOperand(0), BinOp->getOperand(1)),
        Builder.CreateMul(
            ConstantInt::get(BinOp->getType(), 2),
            Builder.CreateAnd(BinOp->getOperand(0), BinOp->getOperand(1))));

    // The following is visible only if you pass -debug on the command line
    // *and* you have an assert build.
    LLVM_DEBUG(dbgs() << *BinOp << " -> " << *NewValue2 << "\n");

    // Replace the instruction for `(a + b)` with the one created above.
    // See: http://llvm.org/docs/ProgrammersManual.html#replacing-an-instruction-with-another-value
    ReplaceInstWithInst(BB.getInstList(), IIT, NewValue2);
    modified = true;

    // Update the statistics
    ++MBACount;
  }
  return modified;
}
