//========================================================================
// FILE:
//    DynamicMain.cpp
//
// DESCRIPTION:
//    A command-line tool that counts all the dynamic/run-time function calls
//    in the input LLVM file. It builds on top of the DynamicCallCounter pass.
//
// USAGE:
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
#include "config.h"

#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/Bitcode/BitcodeWriter.h"
#include "llvm/CodeGen/CommandFlags.inc"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Verifier.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/Path.h"
#include "llvm/Support/PrettyStackTrace.h"
#include "llvm/Support/Program.h"
#include "llvm/Support/Signals.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/ToolOutputFile.h"

// Not used here per-se, but required by Debug.h
#define DEBUG_TYPE "cc"

using namespace llvm;
using llvm::legacy::PassManager;
using llvm::sys::ExecuteAndWait;
using llvm::sys::findProgramByName;

//===----------------------------------------------------------------------===//
// Command line options
//===----------------------------------------------------------------------===//
static cl::OptionCategory CallCounterCategory{"call counter options"};

// This enables the LLVM_DEBUG macro
// TODO The name of the option should be different - this will crash with opt
// built in debug mode
// static cl::opt<bool, true> Debug("debug", cl::desc("Enable debug output"),
//                                  cl::Hidden, cl::location(DebugFlag));

static cl::opt<bool>
    SaveInstrumented("save-instrumented",
                     cl::desc("Saves the instrumented module to disk"),
                     cl::Optional, cl::cat{CallCounterCategory});

static cl::opt<std::string> InputModule{cl::Positional,
                                        cl::desc{"<Module to analyze>"},
                                        cl::value_desc{"bitcode filename"},
                                        cl::init(""),
                                        cl::Required,
                                        cl::cat{CallCounterCategory}};

static cl::opt<std::string> OutputModule{
    "o", cl::desc{"Filename of the instrumented program"},
    cl::value_desc{"filename"}, cl::Required, cl::cat{CallCounterCategory}};

enum OptLevelTy { O0 = 0, O1, O2, O3 };

cl::opt<OptLevelTy>
    OptLevelCl(cl::desc("Choose optimization level (default = 'O2'):"),
               cl::values(clEnumVal(O1, "Enable trivial optimizations"),
                          clEnumVal(O2, "Enable default optimizations"),
                          clEnumVal(O3, "Enable expensive optimizations")),
               cl::init(O2), cl::cat{CallCounterCategory});

//===----------------------------------------------------------------------===//
// The DynamicCountPrinter pass - wrappers
//===----------------------------------------------------------------------===//
static void compile(Module &M, StringRef OutputPath) {
  // 1. Get the target specific information (architecture, OS, etc)
  std::string ErrMsg;
  Triple triple = Triple(M.getTargetTriple());
  Target const *TheTarget = TargetRegistry::lookupTarget(MArch, triple, ErrMsg);

  if (nullptr == TheTarget) {
    report_fatal_error("Unable to find target:\n " + ErrMsg);
  }

  // 2. Generate the optimisation level
  CodeGenOpt::Level OptLevel = CodeGenOpt::Default;
  switch (OptLevelCl) {
  default:
    report_fatal_error("Invalid optimization level\n");
  case O0:
    OptLevel = CodeGenOpt::None;
    break;
  case O1:
    OptLevel = CodeGenOpt::Less;
    break;
  case O2:
    OptLevel = CodeGenOpt::Default;
    break;
  case O3:
    OptLevel = CodeGenOpt::Aggressive;
    break;
  }

  // 3. Generate the output file
  std::error_code ErrCode;
  auto OutputFile =
      std::make_unique<ToolOutputFile>(OutputPath, ErrCode, sys::fs::F_None);
  if (nullptr == OutputFile) {
    report_fatal_error("Unable to create the output file:\n " +
                       ErrCode.message());
  }

  // 4. Get TargetMachine (interface to the complete target machine description)
  std::string FeaturesStr;
  TargetOptions TO = InitTargetOptionsFromCodeGenFlags();
  std::unique_ptr<TargetMachine> TheMachine(TheTarget->createTargetMachine(
      triple.getTriple(), MCPU, FeaturesStr, TO, getRelocModel(),
      /* CodeModel */ None, OptLevel));
  assert((nullptr != TheMachine) && "Could not allocate target machine!");

  // 5. Set layout properties related to datatype size/offset/alignment
  M.setDataLayout(TheMachine->createDataLayout());

  // 6. Create the output stream for the output file
  raw_pwrite_stream *OutS(&OutputFile->os());
  std::unique_ptr<buffer_ostream> BOS;
  if (!OutputFile->os().supportsSeeking()) {
    BOS = std::make_unique<buffer_ostream>(*OutS);
    OutS = BOS.get();
  }

  // 7. Ask the target to add backend passes as necessary Build up all the
  // passes for the module object file generation
  legacy::PassManager PM;

  TargetLibraryInfoImpl TLI(triple);
  PM.add(new TargetLibraryInfoWrapperPass(TLI));
  FileType = TargetMachine::CGFT_ObjectFile;
  if (TheMachine->addPassesToEmitFile(PM, *OutS, /*DWO file */ nullptr,
                                      FileType,
                                      /*DisableVerify */ true)) {
    report_fatal_error("target does not support generation "
                       "of this file type!\n");
  }

  // 8. Finally, run the passes
  PM.run(M);

  // 9. Keep the output binary if we've been successful to this point.
  OutputFile->keep();
}

static void link(StringRef InObjFile, StringRef OutObjFile) {
  // Arguments for the linker
  SmallVector<const char *, 128> LinkerArgs;

  // Get the clang++ executable path
  auto ClangExecutable = sys::findProgramByName("clang++");
  if (!ClangExecutable) {
    report_fatal_error("Unable to find clang++");
  }
  LinkerArgs.push_back(ClangExecutable->c_str());

  // Optimisation level
  std::string opt("-O");
  opt += std::to_string(OptLevelCl);
  LinkerArgs.push_back(opt.c_str());

  // Output file
  LinkerArgs.push_back("-o");
  LinkerArgs.push_back(OutObjFile.data());

  // Input File
  LinkerArgs.push_back(InObjFile.data());

  // Add lt-rt-cc
  LinkerArgs.push_back("-L");
  LinkerArgs.push_back(LT_RT_LIBRARY_PATH);
  LinkerArgs.push_back("-l");
  LinkerArgs.push_back(LT_RUNTIME_LIB);

  // This is needed for toStringRefArray to work
  LinkerArgs.push_back(nullptr);

  // ExecuteAndWait requires the arguments as a vector of StringRef
  auto LinkerArgsStrRef = llvm::toStringRefArray(LinkerArgs.data());

  LLVM_DEBUG(
      errs() << "LLVM-TUTOR - about to run:\n\t"; for (auto &arg
                                                       : LinkerArgsStrRef) {
        errs() << arg << " ";
      } errs() << "\n";);

  // Run the linker (via clang++)
  if (0 != ExecuteAndWait(ClangExecutable.get(), LinkerArgsStrRef)) {
    report_fatal_error("Unable to link output file");
  }
}

static void saveModule(const Module &M, StringRef OutputFile) {
  std::error_code ErrC;
  raw_fd_ostream OutS(OutputFile.data(), ErrC, sys::fs::F_None);

  if (ErrC) {
    report_fatal_error("error saving llvm module to '" + OutputFile + "': \n" +
                       ErrC.message());
  }
  WriteBitcodeToFile(M, OutS);
}

static void countDynamicCalls(Module &M) {
  // 1. Initialize the native target for emitting object code.
  InitializeNativeTarget();
  InitializeNativeTargetAsmPrinter();

  // 2. Build up and run all of the passes that we want for the input module.
  legacy::PassManager PM;
  PM.add(new DynamicCallCounter());
  PM.add(createVerifierPass());

  PM.run(M);

  // 3. Save the instrumented module and generate a binary from it
  if (SaveInstrumented)
    saveModule(M, OutputModule + ".dynamic.bc");

  std::string ObjectFile = OutputModule + ".o";
  compile(M, ObjectFile);
  link(ObjectFile, OutputModule);
}

//===----------------------------------------------------------------------===//
// Main driver code.
//===----------------------------------------------------------------------===//
int main(int Argc, char **Argv) {
  // Hide all options apart from the ones specific to this tool
  cl::HideUnrelatedOptions(CallCounterCategory);

  cl::ParseCommandLineOptions(Argc, Argv,
                              "Counts the number of dynamic function "
                              "calls in the input IR file\n");

  // Makes sure llvm_shutdown() is called (which cleans up LLVM objects)
  //  http://llvm.org/docs/ProgrammersManual.html#ending-execution-with-llvm-shutdown
  llvm_shutdown_obj SDO;

  // Parse the IR file passed on the command line.
  SMDiagnostic Err;
  LLVMContext Ctx;
  std::unique_ptr<Module> M = parseIRFile(InputModule.getValue(), Err, Ctx);

  if (!M) {
    errs() << "Error reading bitcode file: " << InputModule << "\n";
    Err.print(Argv[0], errs());
    return -1;
  }

  // Instrument the binary and write it to the output file specified on the
  // command line.
  countDynamicCalls(*M);

  return 0;
}
