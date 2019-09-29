//========================================================================
// FILE:
//      DynamicCallCounterRT.cpp
//
// DESCRIPTION:
//     Runtime functions for the DynamicCallCounter pass. These functions
//     are installed inside the instrumented module.
//
//     The names in this file are prefixed with lt_RUNTIME to avoid clashing
//     with any other names in the final binary.
//
// License: MIT
//========================================================================
#include <cstdint>
#include <cstdio>

extern "C" {

// The count of the number of functions is stored in a global variable inside
// the instrumented module.
extern uint64_t lt_RUNTIME_numFunctions;

// An array of information for each function ID is stored within the
// instrumented module.
extern struct {
  char *name;
  uint64_t count;
} lt_RUNTIME_functionInfo[];

void lt_RUNTIME_incrCC(uint64_t id) { ++lt_RUNTIME_functionInfo[id].count; }

void lt_RUNTIME_print() {
  printf("=================================================\n"
         "LLVM-TUTOR: dynamic analysis results\n"
         "=================================================\n");
  printf("%-20s %-10s\n", "NAME", "#N DIRECT CALLS");
  printf("-------------------------------------------------\n");
  for (size_t id = 0; id < lt_RUNTIME_numFunctions; ++id) {
    auto &info = lt_RUNTIME_functionInfo[id];
    printf("%-20s %-10lu\n", info.name, info.count);
  }
  printf("-------------------------------------------------\n");
}
}
