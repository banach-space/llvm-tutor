//========================================================================
// FILE:
//    LVN.cpp
//
// DESCRIPTION:
//    This pass implements a simple Local Value Numbering (LVN) optimization
//    pass for LLVM IR. The optimization is performed independently within
//    each basic block.
//
//    The pass performs the following optimizations:
//
//      1. Redundant Load Elimination
//         Repeated loads from the same memory location are eliminated when
//         no intervening store modifies the value.
//
//      2. Constant Propagation and Constant Folding
//         Constant values are tracked locally within a basic block. Binary
//         operations whose operands are known constants are folded into a
//         single constant value at compile time.
//
//      3. Common Subexpression Elimination (CSE)
//         Previously computed expressions are detected using value numbering.
//         Repeated computations are replaced with the already computed value.
//
//      4. Dead Alloca Elimination
//         Unused stack allocations introduced during earlier transformations
//         are removed from the function.
//
//    The pass uses a hash-based value numbering scheme where expressions are
//    represented as tuples consisting of:
//
//        (Opcode, Left Operand, Right Operand)
//
//    Commutative operations such as addition and multiplication are normalized
//    so that equivalent expressions produce identical value numbers.
//
// USAGE:
//      $ opt -load-pass-plugin <BUILD_DIR>/lib/libLVN.so \
//        -passes="lvn" <bitcode-file>
//
// License: MIT
//========================================================================
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"
#include <iterator>
#include <llvm/ADT/DenseMap.h>
#include <llvm/ADT/StringMapEntry.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/InstrTypes.h>
#include <llvm/IR/Instruction.h>
#include <llvm/IR/Instructions.h>
#include <llvm/Support/Casting.h>
#include <optional>
#include <string>
#include <unordered_map>
#define DEBUG_TYPE "lvn"
using namespace llvm;
namespace {
struct LVN : PassInfoMixin<LVN> {
  std::optional<int>
  getValue(Value *operand,
           std::unordered_map<std::basic_string<char>, int> constantmap) {
    // this is a simple method , given a value , it checks if its a constant
    //  ie if the operands are constants or either if they were folded earlier
    //  and now have a constant entry
    if (auto val = dyn_cast<ConstantInt>(operand)) {
      int n = val->getSExtValue();
      return n;
    }
    auto val = constantmap.find(operand->getNameOrAsOperand());
    if (val != constantmap.end()) {
      return val->second;
    }
    return std::nullopt; // this is if the operand is found to be not a constant
  }
  void eliminateRedLoads(Function *F) {
    // we are looking to eliminate redundant loads
    // and example would be
    //%9 = load i32, ptr %2, align 4
    //%10 = load i32, ptr %2, align 4
    // here since there is no store happening in between
    // the two instructions , the second value %10 can be replaced
    // with %9, this will help us identify commonn sub expressions in the future
    //  and also eliminate unwanted loads
    errs() << "................................................................"
              "..................\n";
    errs() << "Running Redundant Load Elimination on : " << F->getName()
           << "\n";
    for (auto BB = F->begin(), eBB = F->end(); BB != eBB; BB++) {
      std::unordered_map<Value *, Value *> map;
      for (auto inst = BB->begin(), einst = BB->end(); inst != einst;) {
        bool del = false;
        if (auto op = dyn_cast<LoadInst>(inst)) {
          auto item = map.find(op->getPointerOperand());
          if (item != map.end()) {
            del = true;
            errs() << "Replaced : " << *inst << "| with : " << *item->second
                   << "\n";
            op->replaceAllUsesWith(item->second);
          } else {
            map[op->getPointerOperand()] = op;
          }
        } else if (auto op = dyn_cast<StoreInst>(inst)) {
          auto item = map.find(op->getPointerOperand());
          if (item != map.end()) {
            map.erase(item);
          }
        }
        if (del) {
          auto next = std::next(inst);
          inst->eraseFromParent();
          inst = next;
        } else {
          inst++;
        }
      }
    }
  }
  void eliminateDeadAllocs(Function *F) {
    // a simple metho to check if the alloca instruction is used anywhere in the
    // function
    // if it is unused, we delete it , this is required after Common Sub
    // Expression Elimination
    // and constant propogation since there maybe dead alloca , test it without
    // this function to play with it
    errs() << "................................................................"
              "..................\n";
    errs() << "Running Dead Alloc Elimination on Function : " << F->getName()
           << "\n";
    std::unordered_map<std::string, Instruction *> map;
    for (auto BB = F->begin(), eBB = F->end(); BB != eBB; BB++) {
      for (auto inst = BB->begin(), einst = BB->end(); inst != einst; inst++) {
        if (auto op = dyn_cast<AllocaInst>(inst)) {
          // errs() << "  " << *inst << "  " << op->getName() << "\n";
          std::string a = op->getNameOrAsOperand();
          map.insert({a, &*inst});
        }
        if (auto op = dyn_cast<StoreInst>(inst)) {
          map.erase(op->getOperand(1)->getNameOrAsOperand());
        }
      }
    }
    for (auto deadstore = map.begin(), e = map.end(); deadstore != e;) {
      errs() << "Erased Dead Alloc : " << *deadstore->second << "\n";
      auto next = std::next(deadstore);
      deadstore->second->eraseFromParent();
      deadstore = next;
    }
  }

  void constantPropogation(Function *F) {
    // here constant map stores if the value is constant
    // it stores the ssa value example %3 and the integer value for it
    errs() << "................................................................"
              "..................\n";

    errs() << "Constant propogation / folding on Function : " << F->getName()
           << "\n";
    for (auto BB = F->begin(), eBB = F->end(); BB != eBB; BB++) {
      std::unordered_map<std::basic_string<char>, int> constantmap;
      for (auto inst = BB->begin(), eInst = BB->end(); inst != eInst;) {
        bool del = false;
        if (auto op = dyn_cast<BinaryOperator>(inst)) {
          auto l = getValue(op->getOperand(0),
                            constantmap); // we see if its a constant or its
                                          // inside the constant map
          auto r = getValue(op->getOperand(1), constantmap);
          if (l && r) {
            int result = 0;
            if (op->getOpcode() == Instruction::Add) {
              result = *l + *r;
            } else if (op->getOpcode() == Instruction::Sub) {
              result = *l - *r;
            } else if (op->getOpcode() == Instruction::Mul) {
              result = *l * *r;
            } else if (op->getOpcode() == Instruction::SDiv) {
              result = *l / *r;
            } else if (op->getOpcode() == Instruction::SRem) {
              result = *l % *r;
            }
            // if both are constants then they can be folded
            auto value = ConstantInt::get(op->getType(), result);
            errs() << "constants folded : left :" << l << "  right :" << r
                   << "\n";
            op->replaceAllUsesWith(value);
            del = true;
          }
        } else if (auto op = dyn_cast<StoreInst>(inst)) {
          if (auto val = dyn_cast<ConstantInt>(op->getOperand(0))) {
            constantmap.insert(
                {op->getOperand(1)->getNameOrAsOperand(), val->getSExtValue()});
            // errs() << "Erased Dead Store : " << *inst
            //      << " Replaced with Constant : " << val->getSExtValue()
            //    << "\n";
            // op->replaceAllUsesWith(ConstantInt::get(
            // op->getType(),
            // val->getSExtValue())); // we eliminate redundant stores by
            //  replacing all the values in the
            //  upcoming blocks
            // del = true;
          }
        } else if (auto op = dyn_cast<LoadInst>(inst)) {
          auto item = constantmap.find(op->getOperand(0)->getNameOrAsOperand());
          if (item != constantmap.end()) {
            errs() << "Replaced Dead Load : " << *inst
                   << "| with constant : " << item->second << "\n";
            op->replaceAllUsesWith(
                ConstantInt::get(op->getType(), item->second));
            // constant loads are eliminated and replaced with the values
            // instead
            del = true;
          }
        }
        if (del) {
          auto next = std::next(inst);
          inst->eraseFromParent();
          inst = next;
        } else {
          inst++;
        }
      }
    }
  }
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &) {
    eliminateRedLoads(&F);
    constantPropogation(&F);
    errs() << "................................................................"
              "..................\n";
    errs() << "Common SubExpression Elimination : " << F.getName() << "\n";
    for (auto BB = F.begin(), eBB = F.end(); BB != eBB; BB++) {
      // eliminateRedLoads();   // we eliminate redundant loads since its a
      //  pre-requisite for CSE
      // constantPropogation(&F); // we eliminate constants from each basic
      //  blocks
      llvm::DenseMap<std::tuple<unsigned, Value *, Value *>, Value *> map;
      // this is out LVN map where we store the expressions as tuples
      bool del = false;
      for (auto inst = BB->begin(), eInst = BB->end(); inst != eInst;) {
        bool del = false;
        if (auto op = dyn_cast<BinaryOperator>(inst)) {
          auto hashl = op->getOperand(0);
          auto hashr = op->getOperand(1);
          if (hashl > hashr && (op->getOpcode() == Instruction::Add ||
                                op->getOpcode() == Instruction::Mul)) {
            std::swap(hashl,
                      hashr); // this is to maintain commutativity , so a+b and
                              // b+a are always stored in the same order
          }
          auto expr = map.find(std::make_tuple(op->getOpcode(), hashl, hashr));
          if (expr != map.end()) {
            errs() << "Common SubExpr Found : " << *inst
                   << " replaced with : " << *expr->second << "\n";
            op->replaceAllUsesWith(
                expr->second); // if its an old expression then replace it
            del = true;
          } else {
            map[std::make_tuple(op->getOpcode(), hashl, hashr)] =
                &*inst; // if its a new tuple store it
          }
        }
        if (del) {
          auto next = std::next(inst);
          inst->eraseFromParent();
          inst = next;
        } else {
          inst++;
        }
      }
    }
    eliminateDeadAllocs(&F);
    return PreservedAnalyses::all();
  }
};
} // namespace
// namespace
PassPluginLibraryInfo getLVNPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "LVN", LLVM_VERSION_STRING,
          [](PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](StringRef Name, FunctionPassManager &FPM,
                   ArrayRef<PassBuilder::PipelineElement>) {
                  if (Name == "lvn") {
                    FPM.addPass(LVN());
                    return true;
                  }
                  return false;
                });
            PB.registerPipelineStartEPCallback([](ModulePassManager &MPM,
                                                  OptimizationLevel Level) {
              FunctionPassManager FPM;
              FPM.addPass(LVN());
              MPM.addPass(createModuleToFunctionPassAdaptor(std::move(FPM)));
            });
          }};
}
extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
  return getLVNPluginInfo();
}
