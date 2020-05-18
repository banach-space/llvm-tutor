//==============================================================================
// FILE:
//    MBAAdd.cpp
//
// DESCRIPTION:
//    This pass performs a substitution for 8-bit integer add
//    instruction based on this Mixed Boolean-Airthmetic expression:
//      a + b == (((a ^ b) + 2 * (a & b)) * 39 + 23) * 151 + 111
//    See formula (3) in [1].
//
// USAGE:
//    1. Legacy pass manager:
//      $ opt -load <BUILD_DIR>/lib/libMBAAdd.so `\`
//        --legacy-mba-add [-mba-ratio=<ratio>] <bitcode-file>
//      with the optional ratio in the range [0, 1.0].
//    2. New pass maanger:
//      $ opt -load-pass-plugin <BUILD_DIR>/lib/libMBAAdd.so `\`
//        -passes=-"mba-add" <bitcode-file>
//      The command line option is not available for the new PM
//
//  
// [1] "Defeating MBA-based Obfuscation" Ninon Eyrolles, Louis Goubin, Marion
//     Videau
//
// License: MIT
//==============================================================================
#include "MBAAdd.h"

#include "llvm/ADT/Statistic.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"

#include "Ratio.h"

#include <random>

using namespace llvm;

#define DEBUG_TYPE "mba-add"

STATISTIC(SubstCount, "The # of substituted instructions");

// Pass Option declaration
static cl::opt<Ratio, false, llvm::cl::parser<Ratio>> MBARatio{
    "mba-ratio",
    cl::desc("Only apply the mba pass on <ratio> of the candidates"),
    cl::value_desc("ratio"), cl::init(1.), cl::Optional};

//-----------------------------------------------------------------------------
// MBAAdd Implementation
//-----------------------------------------------------------------------------
bool MBAAdd::runOnBasicBlock(BasicBlock &BB) {
  bool Changed = false;

  // Get a (rather naive) random number generator that will be used to decide
  // whether to replace the current instruction or not.
  // FIXME We should be using 'Module::createRNG' here instead. However, that
  // method requires a pointer to 'Pass' on input and passes
  // for the new pass manager _do_not_ inherit from llvm::Pass. In other words,
  // 'createRNG' cannot be used here and there's no other way of obtaining
  // llvm::RandomNumberGenerator. Last checked for LLVM 8.
  std::mt19937_64 RNG;
  RNG.seed(1234);
  std::uniform_real_distribution<double> Dist(0., 1.);

  // Loop over all instructions in the block. Replacing instructions requires
  // iterators, hence a for-range loop wouldn't be suitable
  for (auto Inst = BB.begin(), IE = BB.end(); Inst != IE; ++Inst) {
    // Skip non-binary (e.g. unary or compare) instructions
    auto *BinOp = dyn_cast<BinaryOperator>(Inst);
    if (!BinOp)
      continue;

    // Skip instructions other than add
    if (BinOp->getOpcode() != Instruction::Add)
      continue;

    // Skip if the result is not 8-bit wide (this implies that the operands are
    // also 8-bit wide)
    if (!BinOp->getType()->isIntegerTy() ||
        !(BinOp->getType()->getIntegerBitWidth() == 8))
      continue;

    // Use Ratio and RNG to decide whether to substitute this particular 'add'
    if (Dist(RNG) > MBARatio.getRatio())
      continue;

    // A uniform API for creating instructions and inserting
    // them into basic blocks
    IRBuilder<> Builder(BinOp);

    // Constants used in building the instruction for substitution
    auto Val39 = ConstantInt::get(BinOp->getType(), 39);
    auto Val151 = ConstantInt::get(BinOp->getType(), 151);
    auto Val23 = ConstantInt::get(BinOp->getType(), 23);
    auto Val2 = ConstantInt::get(BinOp->getType(), 2);
    auto Val111 = ConstantInt::get(BinOp->getType(), 111);

    // Build an instruction representing `(((a ^ b) + 2 * (a & b)) * 39 + 23) *
    // 151 + 111`
    Instruction *NewInst =
        // E = e5 + 111
        BinaryOperator::CreateAdd(
            Val111,
            // e5 = e4 * 151
            Builder.CreateMul(
                Val151,
                // e4 = e2 + 23
                Builder.CreateAdd(
                    Val23,
                    // e3 = e2 * 39
                    Builder.CreateMul(
                        Val39,
                        // e2 = e0 + e1
                        Builder.CreateAdd(
                            // e0 = a ^ b
                            Builder.CreateXor(BinOp->getOperand(0),
                                              BinOp->getOperand(1)),
                            // e1 = 2 * (a & b)
                            Builder.CreateMul(
                                Val2, Builder.CreateAnd(BinOp->getOperand(0),
                                                        BinOp->getOperand(1))))
                    ) // e3 = e2 * 39
                ) // e4 = e2 + 23
            ) // e5 = e4 * 151
        ); // E = e5 + 111

    // The following is visible only if you pass -debug on the command line
    // *and* you have an assert build.
    LLVM_DEBUG(dbgs() << *BinOp << " -> " << *NewInst << "\n");

    // Replace `(a + b)` (original instructions) with `(((a ^ b) + 2 * (a & b))
    // * 39 + 23) * 151 + 111` (the new instruction)
    ReplaceInstWithInst(BB.getInstList(), Inst, NewInst);
    Changed = true;

    // Update the statistics
    ++SubstCount;
  }
  return Changed;
}

PreservedAnalyses MBAAdd::run(llvm::Function &F,
                              llvm::FunctionAnalysisManager &) {
  bool Changed = false;

  for (auto &BB : F) {
    Changed |= runOnBasicBlock(BB);
  }
  return (Changed ? llvm::PreservedAnalyses::none()
                  : llvm::PreservedAnalyses::all());
}

bool LegacyMBAAdd::runOnFunction(llvm::Function &F) {
  bool Changed = false;

  for (auto &BB : F) {
    Changed |= Impl.runOnBasicBlock(BB);
  }
  return Changed;
}

//-----------------------------------------------------------------------------
// New PM Registration
//-----------------------------------------------------------------------------
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

extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
  return getMBAAddPluginInfo();
}

//-----------------------------------------------------------------------------
// Legacy PM Registration
//-----------------------------------------------------------------------------
char LegacyMBAAdd::ID = 0;

static RegisterPass<LegacyMBAAdd> X(/*PassArg=*/"legacy-mba-add",
                                    /*Name=*/"MBAAdd",
                                    /*CFGOnly=*/true,
                                    /*is_analysis=*/false);
