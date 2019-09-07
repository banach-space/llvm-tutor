//========================================================================
// FILE:
//    DynamicCallCounter.cpp
//
// DESCRIPTION:
//    Counts the number of dynamic (i.e. run-time) direct function calls.
//    Leverages the runtime library, implemented in DynamicCallCounterRT.cpp.
//    It instruments the underlying module by:
//      * defining the variables required by the run-time library
//      * declaring the run-time library function which are installed inside the
//        module (more precisely, calls to these functions are installed).
//        These functions are defined in the runtime library.
//    There are two cases:
//      * for each function defined in this module, this pass inserts a call to
//        lt_RUNTIME_called before any other instruction inside the function
//        (i.e. it modifies the function)
//      * for each call to an externally defined function, this pass inserts a
//        call to lt_RUNTIME_called just before the instruction calling the
//        function (we can't instrument externally defined functions so we
//        modify the call site inside this module).
//    The function and data types inserted here *must* match those in
//    DynamicCallCounterRT.cpp.
//
// USAGE:
//    This pass can only be run through 'dynamic':
//
//    # First, generate an LLVM file
//    clang -emit-llvm <input-file> -o <output-llvm-file>
//    # Run this tool to generate an instrumented binary
//    <BUILD/DIR>/bin/dynamic <llvm-file-to-analyze> -o <output-bin-file>
//    # Run the instrumented binary
//    ./<output-bin-file>
//
// License: MIT
//========================================================================
#include "DynamicCallCounter.h"

#include "llvm/IR/IRBuilder.h"
#include "llvm/Transforms/Utils/ModuleUtils.h"

using namespace llvm;
using lt::DynamicCallCounter;

namespace lt {

char DynamicCallCounter::ID = 0;

} // namespace lt

// Returns a map (Function* -> uint64_t).
static DenseMap<Function *, uint64_t>
computeFunctionIDs(llvm::ArrayRef<Function *> Functions) {
  DenseMap<Function *, uint64_t> IdMap;

  size_t NextID = 0;
  for (auto F : Functions) {
    IdMap[F] = NextID;
    ++NextID;
  }

  return IdMap;
}

// Returns a set of all internal (i.e. defined in this module) functions.
static DenseSet<Function *>
computeInternal(llvm::ArrayRef<Function *> Functions) {
  DenseSet<Function *> Internal;

  for (auto F : Functions) {
    if (!F->isDeclaration()) {
      Internal.insert(F);
    }
  }

  return Internal;
}

static llvm::Constant *createConstantString(llvm::Module &M,
                                            llvm::StringRef InStr) {
  auto &CTX = M.getContext();

  auto *Name = llvm::ConstantDataArray::getString(CTX, InStr, true);
  auto *Int8Ty = llvm::Type::getInt8Ty(CTX);
  auto *ArrayTy = llvm::ArrayType::get(Int8Ty, InStr.size() + 1);
  auto *AsStr = new llvm::GlobalVariable(
      M, ArrayTy, true, llvm::GlobalValue::PrivateLinkage, Name);

  auto *Zero = llvm::ConstantInt::get(llvm::Type::getInt32Ty(CTX), 0);
  llvm::Value *indices[] = {Zero, Zero};
  return llvm::ConstantExpr::getInBoundsGetElementPtr(ArrayTy, AsStr, indices);
}

// Creates the global lt_RUNTIME_functionInfo table used by the runtime
// library.
static void createGlobalFunctionTable(Module &M, uint64_t NumFunctions) {
  auto &CTX = M.getContext();

  // 1. Create the component types of the table
  auto *Int64Ty = Type::getInt64Ty(CTX);
  auto *StringTy = Type::getInt8PtrTy(CTX);
  Type *FieldTys[] = {StringTy, Int64Ty};
  auto *structTy = StructType::get(CTX, FieldTys, false);

  // 2. Create and initialize a table of function information
  std::vector<Constant *> FunctionInfo;
  auto *Zero = ConstantInt::get(Int64Ty, 0, false);
  std::transform(
      M.begin(), M.end(), std::back_inserter(FunctionInfo),
      [&M, Zero, structTy](auto &f) {
        Constant *StructFields[] = {createConstantString(M, f.getName()), Zero};
        return ConstantStruct::get(structTy, StructFields);
      });

  // 3. Inject the table into the module as a global var
  auto *TableTy = ArrayType::get(structTy, NumFunctions);
  auto *FunctionTable = ConstantArray::get(TableTy, FunctionInfo);
  new GlobalVariable(M, TableTy, false, GlobalValue::ExternalLinkage,
                     FunctionTable, "lt_RUNTIME_functionInfo");
}

// For an analysis pass, runOnModule should perform the actual analysis and
// compute the results. The actual output, however, is produced separately.
bool DynamicCallCounter::runOnModule(Module &M) {
  auto &CTX = M.getContext();

  // 1. Identify the functions we wish to track
  std::vector<Function *> ToCount;
  for (auto &F : M) {
    ToCount.push_back(&F);
  }

  IDs = computeFunctionIDs(ToCount);
  Internal = computeInternal(ToCount);
  auto const NumFunctions = ToCount.size();

  // 2. Store the number of functions into an externally visible variable.
  auto *Int64Ty = Type::getInt64Ty(CTX);
  auto *NumFunctionsGlobal = ConstantInt::get(Int64Ty, NumFunctions, false);
  new GlobalVariable(M, Int64Ty, true, GlobalValue::ExternalLinkage,
                     NumFunctionsGlobal, "lt_RUNTIME_numFunctions");

  // 3. Create a global table of function infos
  createGlobalFunctionTable(M, NumFunctions);

  // 4. Declare the counter function
  auto *VoidTy = Type::getVoidTy(CTX);
  auto *HelperTy = FunctionType::get(VoidTy, Int64Ty, false);
  auto *CounterFunc = M.getOrInsertFunction("lt_RUNTIME_called", HelperTy);

  // 5. Declare and install the result printing function so that it prints out
  // the counts after the entire program is finished executing.
  auto *Printer = M.getOrInsertFunction("lt_RUNTIME_print", VoidTy);
  appendToGlobalDtors(M, llvm::cast<Function>(Printer), 0);

  for (auto F : ToCount) {
    // We only want to instrument internally defined functions.
    if (F->isDeclaration()) {
      continue;
    }

    // Count each internal function - install a call to
    // lt_RUNTIME_called at the beginning of each internal function.
    installCCFunction(*F, CounterFunc);

    // Count each external function - loop over all instructions in
    // this function and for each function call insert a call to
    // lt_RUNTIME_called just before the call-site instruction.
    for (auto &BB : *F) {
      for (auto &I : BB) {
        installCCInstruction(CallSite(&I), CounterFunc);
      }
    }
  }

  return true;
}

void DynamicCallCounter::installCCFunction(Function &F, Value *Counter) {
  IRBuilder<> builder(&*F.getEntryBlock().getFirstInsertionPt());
  builder.CreateCall(Counter, builder.getInt64(IDs[&F]));
}

void DynamicCallCounter::installCCInstruction(CallSite CS, Value *Counter) {
  // Check whether the instruction is actually a call
  if (nullptr == CS.getInstruction()) {
    return;
  }

  // Check whether the called function is directly invoked
  auto Called = dyn_cast<Function>(CS.getCalledValue()->stripPointerCasts());
  if (nullptr == Called) {
    return;
  }

  // Check if the function is internal or blacklisted.
  // (A function is blacklisted if it wasn't present in the module at the point
  // of creating the ids map, i.e. the functions from the run-time library).
  if ((Internal.count(Called) > 0) || (0 == IDs.count(Called))) {
    // Internal functions are counted upon the entry of each function body.
    // Blacklisted functions are not counted. Neither should proceed.
    return;
  }

  // External functions are counted at their invocation sites.
  IRBuilder<> Builder(CS.getInstruction());
  Builder.CreateCall(Counter, Builder.getInt64(IDs[Called]));
}
