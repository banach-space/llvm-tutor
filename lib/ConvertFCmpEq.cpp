//=============================================================================
// FILE:
//    ConvertFCmpEq.cpp
//
// DESCRIPTION:
//    Transformation pass which uses the results of the FindFCmpEq analysis pass
//    to convert all equality-based floating point comparison instructions in a
//    function to indirect, difference-based comparisons.
//
//    This example demonstrates how to couple an analysis pass with a
//    transformation pass, the use of statistics (the STATISTIC macro), and LLVM
//    debugging operations (the LLVM_DEBUG macro and the llvm::dbgs() output
//    stream). It also demonstrates how instructions can be modified without
//    having to completely replace them.
//
//    Originally developed for [1].
//
//    [1] "Writing an LLVM Optimization" by Jonathan Smith
//
// USAGE:
//      opt --load-pass-plugin libConvertFCmpEq.dylib [--stats] `\`
//        --passes='convert-fcmp-eq' --disable-output <input-llvm-file>
//
// License: MIT
//=============================================================================
#include "ConvertFCmpEq.h"
#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/APInt.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/IR/Attributes.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Value.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/ErrorHandling.h"
#include <cassert>

using namespace llvm;

// Unnamed namespace for private functions
static FCmpInst *convertFCmpEqInstruction(FCmpInst *FCmp) noexcept {
  assert(FCmp && "The given fcmp instruction is null");

  if (!FCmp->isEquality()) {
    // We're only interested in equality-based comparisons, so return null if
    // this comparison isn't equality-based.
    return nullptr;
  }

  Value *LHS = FCmp->getOperand(0);
  Value *RHS = FCmp->getOperand(1);
  // Determine the new floating-point comparison predicate based on the current
  // one.
  CmpInst::Predicate CmpPred = [FCmp] {
    switch (FCmp->getPredicate()) {
    case CmpInst::Predicate::FCMP_OEQ:
      return CmpInst::Predicate::FCMP_OLT;
    case CmpInst::Predicate::FCMP_UEQ:
      return CmpInst::Predicate::FCMP_ULT;
    case CmpInst::Predicate::FCMP_ONE:
      return CmpInst::Predicate::FCMP_OGE;
    case CmpInst::Predicate::FCMP_UNE:
      return CmpInst::Predicate::FCMP_UGE;
    default:
      llvm_unreachable("Unsupported fcmp predicate");
    }
  }();

  // Create the objects and values needed to perform the equality comparison
  // conversion.
  Module *M = FCmp->getModule();
  assert(M && "The given fcmp instruction does not belong to a module");
  LLVMContext &Ctx = M->getContext();
  IntegerType *I64Ty = IntegerType::get(Ctx, 64);
  Type *DoubleTy = Type::getDoubleTy(Ctx);

  // Define the sign-mask and double-precision machine epsilon constants.
  ConstantInt *SignMask = ConstantInt::get(I64Ty, ~(1L << 63));
  // The machine epsilon value for IEEE 754 double-precision values is 2 ^ -52
  // or (b / 2) * b ^ -(p - 1) where b (base) = 2 and p (precision) = 53.
  APInt EpsilonBits(64, 0x3CB0000000000000);
  Constant *EpsilonValue =
      ConstantFP::get(DoubleTy, EpsilonBits.bitsToDouble());

  // Create an IRBuilder with an insertion point set to the given fcmp
  // instruction.
  IRBuilder<> Builder(FCmp);
  // Create the subtraction, casting, absolute value, and new comparison
  // instructions one at a time.
  // %0 = fsub double %a, %b
  auto *FSubInst = Builder.CreateFSub(LHS, RHS);
  // %1 = bitcast double %0 to i64
  auto *CastToI64 = Builder.CreateBitCast(FSubInst, I64Ty);
  // %2 = and i64 %1, 0x7fffffffffffffff
  auto *AbsValue = Builder.CreateAnd(CastToI64, SignMask);
  // %3 = bitcast i64 %2 to double
  auto *CastToDouble = Builder.CreateBitCast(AbsValue, DoubleTy);
  // %4 = fcmp <olt/ult/oge/uge> double %3, 0x3cb0000000000000
  // Rather than creating a new instruction, we'll just change the predicate and
  // operands of the existing fcmp instruction to match what we want.
  FCmp->setPredicate(CmpPred);
  FCmp->setOperand(0, CastToDouble);
  FCmp->setOperand(1, EpsilonValue);
  return FCmp;
}

static constexpr char PassArg[] = "convert-fcmp-eq";
static constexpr char PluginName[] = "ConvertFCmpEq";

#define DEBUG_TYPE ::PassArg
STATISTIC(FCmpEqConversionCount,
          "Number of direct floating-point equality comparisons converted");

//------------------------------------------------------------------------------
// ConvertFCmpEq implementation
//------------------------------------------------------------------------------
PreservedAnalyses ConvertFCmpEq::run(Function &Func,
                                     FunctionAnalysisManager &FAM) {
  auto &Comparisons = FAM.getResult<FindFCmpEq>(Func);
  bool Modified = run(Func, Comparisons);
  return Modified ? PreservedAnalyses::none() : PreservedAnalyses::all();
}

bool ConvertFCmpEq::run(Function &Func,
                        const FindFCmpEq::Result &Comparisons) {
  bool Modified = false;
  // Functions marked explicitly 'optnone' should be ignored since we shouldn't
  // be changing anything in them anyway.
  if (Func.hasFnAttribute(Attribute::OptimizeNone)) {
    LLVM_DEBUG(dbgs() << "Ignoring optnone-marked function \"" << Func.getName()
                      << "\"\n");
    Modified = false;
  } else {
    for (FCmpInst *FCmp : Comparisons) {
      if (convertFCmpEqInstruction(FCmp)) {
        ++FCmpEqConversionCount;
        Modified = true;
      }
    }
  }

  return Modified;
}

//-----------------------------------------------------------------------------
// New PM Registration
//-----------------------------------------------------------------------------
PassPluginLibraryInfo getConvertFCmpEqPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, PluginName, LLVM_VERSION_STRING,
          [](PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [&](StringRef Name, FunctionPassManager &FPM,
                    ArrayRef<PassBuilder::PipelineElement>) {
                  if (!Name.compare(PassArg)) {
                    FPM.addPass(ConvertFCmpEq());
                    return true;
                  }

                  return false;
                });
          }};
}

extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
  return getConvertFCmpEqPluginInfo();
}
