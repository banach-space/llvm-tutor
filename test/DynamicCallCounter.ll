; RUN: %clang -c -emit-llvm %S/../inputs/input_for_cc.c -o - \
; RUN:   | opt -load ../lib/libDynamicCallCounter%shlibext -legacy-dynamic-cc -verify -o instrumented.bin
; RUN: lli ./instrumented.bin | FileCheck %s
; RUN: %clang -c -emit-llvm %S/../inputs/input_for_cc.c -o - \
; RUN:   | opt -load-pass-plugin ../lib/libDynamicCallCounter%shlibext -passes=dynamic-cc -verify -o instrumented.bin
; RUN: lli ./instrumented.bin | FileCheck %s

; Test DynamicCallCounter when run through opt. This is the expected output
; after running dynamic analysis.
; CHECK: foo                  13
; CHECK-NEXT: bar                  2
; CHECK-NEXT: fez                  1
; CHECK-NEXT: main                 1
