; RUN: opt -load ../lib/libDynamicCallCounter%shlibext -legacy-dynamic-cc -verify %S/Inputs/CallCounterInput.ll -o instrumented.bin
; RUN: lli ./instrumented.bin | FileCheck %s
; RUN: opt -load-pass-plugin ../lib/libDynamicCallCounter%shlibext -passes=dynamic-cc -verify %S/Inputs/CallCounterInput.ll -o instrumented.bin
; RUN: lli ./instrumented.bin | FileCheck %s

; Instrument this file with DynamicCallCounter, run it and verify that it
; generates the expected output.

; CHECK: foo                  13
; CHECK-NEXT: bar                  2
; CHECK-NEXT: fez                  1
; CHECK-NEXT: main                 1
