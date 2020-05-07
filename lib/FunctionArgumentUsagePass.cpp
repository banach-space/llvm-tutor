//========================================================================
// FILE:
//    FunctionArgumentUsagePass.cpp
//
// DESCRIPTION:
// This is an analysis pass that diagnoses type mismatches around function
// calls. The idea is borrowed from the task "Writing your own Analysis Pass"
// (http://www.isi.edu/~pedro/Teaching/CSCI565-Spring15/Projects/Project1-LLVM/Project1-LLVM.pdf)
// course CSCI565 Compilers Design.
//
// USAGE:
//    0. clang -O0 -g -S -emit-llvm inputs/input_for_fnargusage.c -o input_for_fnargusage.ll
//    1. Legacy pass manager:
//      $ opt -load <BUILD_DIR>/lib/libFunctionArgumentUsage.so \
//        --legacy-fnargusage -analyze input_for_fnargusage.ll
//
// EXAMPLE OUTPUT:
// Printing analysis 'Function Argument Usage Pass' for function 'demo':
// Function 'main' call on line '16': argument type mismatch. Argument #1
//   Expected 'i32*' but argument is of type 'i32**'
// Function 'callee' call on line '6': argument type mismatch. Argument #1
//   Expected 'i32*' but argument is of type 'i8*'
//
// License: MIT
//========================================================================

#include "FunctionArgumentUsagePass.h"

#include "llvm/ADT/Statistic.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Operator.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"

#define DEBUG_TYPE "ArgUsage"

STATISTIC(NumOfMismatches, "Number of type mismatches are found");

using namespace llvm;

#ifndef NDEBUG
static void dumpFunctionArgs(const Function &F) {
  dbgs() << "function '";
  dbgs().write_escaped(F.getName());
  dbgs() << "' takes " << F.arg_size() << " parameters:\n";
  for (auto a = F.arg_begin(), e = F.arg_end(); a != e; ++a) {
    if (a->hasName()) {
      dbgs() << '\t' << a->getName();
    } else {
      dbgs() << "\tanonymous";
    }
    dbgs() << ": " << *a->getType() << '\n';
  }
}
#endif

#ifndef NDEBUG
static void dumpFunctionUsedFrom(const Value *From, bool hasLine,
                                 unsigned line) {
  dbgs() << "and is used in the '";
  dbgs().write_escaped(From->getName());
  dbgs() << "' function";
  if (hasLine) {
    dbgs() << " (on line: " << line << ')';
  }
  dbgs() << ":\n";
}
#endif

static llvm::Type* uncastOriginalType(llvm::Value *value) {
  if (auto *inst = dyn_cast<CastInst>(value)) {
    return uncastOriginalType(inst->getOperand(0));
  }
  return value->getType();
}

void FunctionArgumentUsagePass::analyzeFunctionUsages(Function &F,
                                                      CallBase *call) {
  bool hasLine = false;
  unsigned line = 0;
  if (auto &debugLoc = call->getDebugLoc()) {
    line = call->getDebugLoc().getLine();
    hasLine = true;
  }

  LLVM_DEBUG(dumpFunctionUsedFrom(call->getFunction(), hasLine, line));

  // check on argument type mismatch
  //   fa - a function's formal argument (an argument from
  //        the signature of the function).
  //   pha - a physical argument, an argument the function
  //         is exactly executed with.
  auto fa = F.arg_begin(), fe = F.arg_end();
  for (auto pha = call->arg_begin(), phe = call->arg_end();
       (fa != fe && pha != phe); ++pha, ++fa) {
    const Type *ftypeptr = fa->getType();
    const Type *phtypeptr = uncastOriginalType(pha->get());

    LLVM_DEBUG({
      dbgs() << "\targ #" << fa->getArgNo();
      if (pha->get()->hasName()) {
        dbgs() << " (" << pha->get()->getName() << ')';
      }
      dbgs() << ": " << *phtypeptr << '\n';
    });
    if (ftypeptr != phtypeptr) {
      // type mismatch is here
      // ... register it:
      NumOfMismatches++;
      typeMismatches.emplace_back(call->getFunction()->getName(), line, hasLine,
                                  fa->getArgNo(), ftypeptr, phtypeptr);
      LLVM_DEBUG({
        // ... and debug:
        dbgs() << "\ttype mismatch: expected '";
        dbgs() << *ftypeptr << "' but argument is of type '";
        dbgs() << *phtypeptr << "'\n";
      });
    }
  }
}

FunctionArgumentUsagePass::Result
FunctionArgumentUsagePass::run(Function &F, FunctionAnalysisManager &) {
  return runOnFunction(F);
}

FunctionArgumentUsagePass::Result
FunctionArgumentUsagePass::runOnFunction(Function &F) {
  releaseMemory(); // new pass manager has to clear memory on every run
  LLVM_DEBUG(dumpFunctionArgs(F));

  for (auto use = F.use_begin(), e = F.use_end(); use != e; ++use) {
    if (CallBase *call = dyn_cast<CallBase>(use->getUser())) {
      analyzeFunctionUsages(F, call);
    } else if (Operator *oper = dyn_cast<Operator>(use->getUser())) {
      for (auto it = oper->use_begin(), ite = oper->use_end(); it != ite;
           ++it) {
        Value *parent = it->getUser();
        if (CallBase *call = dyn_cast<CallBase>(parent)) {
          analyzeFunctionUsages(F, call);
        }
      }
    }
  }
  return Result(typeMismatches.begin(), typeMismatches.end());
}

// We can just put typeMismatches into the runOnFunction body and do not
// invalidate it there but the goal is to demonstrate how to use
// the releaseMemory() legacy pass method.
void FunctionArgumentUsagePass::releaseMemory() {
  typeMismatches.clear();
}

bool LegacyFunctionArgumentUsagePass::runOnFunction(Function &F) {
  Result = Impl.runOnFunction(F);
  return false;
}

void LegacyFunctionArgumentUsagePass::print(llvm::raw_ostream &O,
                                            const Module *M) const {
  printTypeMismatches(O, Result);
}

void LegacyFunctionArgumentUsagePass::releaseMemory() {
  LLVM_DEBUG(dbgs() << "Release memory" << '\n');
  Impl.releaseMemory();
}

//-----------------------------------------------------------------------------
// For New PM Registration
//-----------------------------------------------------------------------------
AnalysisKey FunctionArgumentUsagePass::Key;

//-----------------------------------------------------------------------------
// Legacy PM Registration
//-----------------------------------------------------------------------------

char LegacyFunctionArgumentUsagePass::ID = 0;

static RegisterPass<LegacyFunctionArgumentUsagePass> X(
    /*PassArg=*/"legacy-fnargusage",
    /*Name=*/"Function Argument Usage Pass",
    /*CFGOnly=*/false,
    /*is_analysis=*/false);

//-----------------------------------------------------------------------------
// Helper functions
//-----------------------------------------------------------------------------
void printTypeMismatches(llvm::raw_ostream &OS,
                         const TypeMismatchRange &TypeMismatches) {
  for (auto &mismatch : TypeMismatches) {
    OS << "Function '";
    OS.write_escaped(mismatch.callerName);
    OS << "'";
    if (mismatch.hasLine) {
      OS << " call on line '" << mismatch.line << '\'';
    }
    OS << ": argument type mismatch. ";
    OS << "Argument #" << mismatch.argNo << ' ';
    OS << "Expected '" << *mismatch.expectedType << "' ";
    OS << "but argument is of type '" << *mismatch.actualType << "'\n";
  }
}
