//==============================================================================
// FILE:
//    MBAAdd.cpp
//
// DESCRIPTION:
//    Obfuscation for integer add instructions through Mixed Boolean Arithmetic
//    (MBA)
//
//    This pass performs an instruction substitution based on this equality:
//      a + b == (a ^ b) + 2 * (a & b)
//    See formula 2.2 in https://tel.archives-ouvertes.fr/tel-01623849/document
//
// USAGE:
//    This pass can be run through opt:
//      $ opt -load <BUILD_DIR>/lib/liblt-libmba-shared.so --mba \
//        [-mba-ratio=<ratio>] <bitcode-file>
//    with the optional ratio in the range [0, 1.0].
//
// USEFUL LINKS:
//    Creating a new instruction:
//      http://llvm.org/docs/ProgrammersManual.html#creating-and-inserting-new-instructions
//    Replacing an instruction
//      http://llvm.org/docs/ProgrammersManual.html#replacing-an-instruction-with-another-value
//    LLVM_DEBUG support:
//      http://llvm.org/docs/ProgrammersManual.html#the-llvm-debug-macro-and-debug-option
//    STATISTIC support:
//      http://llvm.org/docs/WritingAnLLVMPass.html#pass-statistics
//    CommandLine support:
//      http://llvm.org/docs/CommandLine.html#quick-start-guide
//
//
// License: MIT
//==============================================================================
#include "MBAAdd.h"

#include "llvm/ADT/Statistic.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/IR/Module.h"
#include "llvm/Pass.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"

#include <random>

#include "Ratio.h"

using namespace llvm;
using lt::MBAAdd;
using lt::LegacyMBAAdd;

#define DEBUG_TYPE "mba-add"

STATISTIC(SubstCount, "The # of substituted instructions");

// Pass Option declaration
static cl::opt<Ratio, false, llvm::cl::parser<Ratio>> MBARatio{
    "mba-ratio",
    cl::desc("Only apply the mba pass on <ratio> of the candidates"),
    cl::value_desc("ratio"), cl::init(1.), cl::Optional};

namespace lt {
/// Convenience typedef for a pass manager over functions.
llvm::PassPluginLibraryInfo getMBAAddPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "mba-add", LLVM_VERSION_STRING,
          [](PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](StringRef Name, FunctionPassManager &FPM,
                   ArrayRef<PassBuilder::PipelineElement>) {
                  if (Name == "mba-add") {
                    FPM.addPass(MBAAdd());
                    return true;
                  }
                  return false;
                });
          }};
}
} // namespace lt

extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
  return lt::getMBAAddPluginInfo();
}

PreservedAnalyses MBAAdd::run(llvm::Function &F, llvm::FunctionAnalysisManager &) {
  bool Changed = false;

  for (auto &BB : F) {
    Changed = runOnBasicBlock(BB);
  }
  return (Changed ? llvm::PreservedAnalyses::none()
                  : llvm::PreservedAnalyses::all());
}

bool MBAAdd::runOnBasicBlock(BasicBlock &BB) {
  bool Changed = false;

  // Get a (rather naive) random number generator that will be used to decide
  // whether to replace the current instruction or not.
  // FIXME We should be using 'Module::createRNG' here instead. However, that
  // method requires a pointer to 'Pass' on input and passes
  // for the new pass manager _do_not_ inherit from llvm::Pass. In other words,
  // 'createRNG' cannot be used here and there's not other way of obtaining
  // llvm::RandomNumberGenerator. Based on LLVM-8.
  std::mt19937_64 RNG;
  RNG.seed(1234);
  std::uniform_real_distribution<double> Dist(0., 1.);

  // Loop over all instructions in the block. Replacing instructions requires
  // iterators, hence a for-range loop wouldn't be suitable.
  for (auto IIT = BB.begin(), IE = BB.end(); IIT != IE; ++IIT) {
    Instruction &Inst = *IIT;

    // Skip non-binary (e.g. unary or compare) instruction.
    auto *BinOp = dyn_cast<BinaryOperator>(&Inst);
    if (!BinOp)
      continue;

    /// Skip instructions other than integer add.
    unsigned Opcode = BinOp->getOpcode();
    if (Opcode != Instruction::Add || !BinOp->getType()->isIntegerTy())
      continue;

    // Use Ratio and RNG to decide whether to substitute this particular 'add'.
    if (Dist(RNG) > MBARatio.getRatio())
      continue;

    // A uniform API for creating instructions and inserting
    // them into basic blocks.
    IRBuilder<> Builder(BinOp);

    // Create an instruction representing `(a ^ b) + 2 * (a & b)`.
    Instruction *NewValue = BinaryOperator::CreateAdd(
        Builder.CreateXor(BinOp->getOperand(0), BinOp->getOperand(1)),
        Builder.CreateMul(
            ConstantInt::get(BinOp->getType(), 2),
            Builder.CreateAnd(BinOp->getOperand(0), BinOp->getOperand(1))));

    // The following is visible only if you pass -debug on the command line
    // *and* you have an assert build.
    LLVM_DEBUG(dbgs() << *BinOp << " -> " << *NewValue << "\n");

    // Replace `(a + b)` (original instructions) with `(a ^ b) + 2 * (a & b)`
    // (the new instruction)
    ReplaceInstWithInst(BB.getInstList(), IIT, NewValue);
    Changed = true;

    // Update the statistics
    ++SubstCount;
  }
  return Changed;
}

namespace lt {
char LegacyMBAAdd::ID = 0;

// Register the pass - required for (among others) opt
static RegisterPass<LegacyMBAAdd>
    X("legacy-mba-add",
      "Mixed Boolean Arithmetic Substitution",
      true, // doesn't modify the CFG => true
      false // not a pure analysis pass => false
    );

bool LegacyMBAAdd::runOnFunction(llvm::Function &F) {
  bool Changed = false;

  for (auto &BB : F) {
    Changed = Impl.runOnBasicBlock(BB);
  }
  return Changed;
}
} // namespace lt
