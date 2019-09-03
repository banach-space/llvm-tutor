//==============================================================================
// FILE:
//    MBASub.cpp
//
// DESCRIPTION:
//    Obfuscation for integer sub instructions through Mixed Boolean Arithmetic
//    (MBA). This pass performs an instruction substitution based on this equality:
//      a - b == (a + ~b) + 1
//    See formula 2.2 (j) in [1].
//
// USAGE:
//    This pass can be run through opt:
//      $ opt -load <BUILD_DIR>/lib/libMBASub.so -legacy-mba-sub <bitcode-file>
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
//  [1] "Hacker's Delight" by Henry S. Warren, Jr.
//
// License: MIT
//==============================================================================
#include "MBASub.h"

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

using namespace llvm;
using lt::MBASub;
using lt::LegacyMBASub;

#define DEBUG_TYPE "mba-sub"

STATISTIC(SubstCount, "The # of substituted instructions");

namespace lt {
/// Convenience typedef for a pass manager over functions.
llvm::PassPluginLibraryInfo getMBASubPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "mba-sub", LLVM_VERSION_STRING,
          [](PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](StringRef Name, FunctionPassManager &FPM,
                   ArrayRef<PassBuilder::PipelineElement>) {
                  if (Name == "mba-sub") {
                    FPM.addPass(MBASub());
                    return true;
                  }
                  return false;
                });
          }};
}
} // namespace lt

extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
  return lt::getMBASubPluginInfo();
}

PreservedAnalyses MBASub::run(llvm::Function &F, llvm::FunctionAnalysisManager &) {
  bool Changed = false;

  for (auto &BB : F) {
    Changed = runOnBasicBlock(BB);
  }
  return (Changed ? llvm::PreservedAnalyses::none()
                  : llvm::PreservedAnalyses::all());
}

bool MBASub::runOnBasicBlock(BasicBlock &BB) {
  bool Changed = false;

  // Loop over all instructions in the block. Replacing instructions requires
  // iterators, hence a for-range loop wouldn't be suitable.
  for (auto IIT = BB.begin(), IE = BB.end(); IIT != IE; ++IIT) {
    Instruction &Inst = *IIT;

    // Skip non-binary (e.g. unary or compare) instruction.
    auto *BinOp = dyn_cast<BinaryOperator>(&Inst);
    if (!BinOp)
      continue;

    /// Skip instructions other than integer sub.
    unsigned Opcode = BinOp->getOpcode();
    if (Opcode != Instruction::Sub || !BinOp->getType()->isIntegerTy())
      continue;

    // A uniform API for creating instructions and inserting
    // them into basic blocks.
    IRBuilder<> Builder(BinOp);

    // Create an instruction representing (a + ~b) + 1
    Instruction *NewValue = BinaryOperator::CreateAdd(
        Builder.CreateAdd(BinOp->getOperand(0),
                          Builder.CreateNot(BinOp->getOperand(1))),
        ConstantInt::get(BinOp->getType(), 1));

    // The following is visible only if you pass -debug on the command line
    // *and* you have an assert build.
    LLVM_DEBUG(dbgs() << *BinOp << " -> " << *NewValue << "\n");

    // Replace `(a - b)` (original instructions) with `(a + ~b) + 1`
    // (the new instruction)
    ReplaceInstWithInst(BB.getInstList(), IIT, NewValue);
    Changed = true;

    // Update the statistics
    ++SubstCount;
  }
  return Changed;
}

namespace lt {
char LegacyMBASub::ID = 0;

// Register the pass - required for (among others) opt
static RegisterPass<LegacyMBASub>
    X("legacy-mba-sub",
      "Mixed Boolean Arithmetic Substitution",
      true, // doesn't modify the CFG => true
      false // not a pure analysis pass => false
    );

bool LegacyMBASub::runOnFunction(llvm::Function &F) {
  bool Changed = false;

  for (auto &BB : F) {
    Changed = Impl.runOnBasicBlock(BB);
  }
  return Changed;
}
} // namespace lt
