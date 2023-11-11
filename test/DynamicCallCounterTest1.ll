; RUN: opt -load-pass-plugin %shlibdir/libDynamicCallCounter%shlibext -passes="dynamic-cc,verify" %S/Inputs/CallCounterInput.ll -o %t.bin
; RUN: lli %t.bin | FileCheck %s

; RUN: %clang -S -emit-llvm %S/../inputs/input_for_cc.c -o - \
; RUN:   | opt -load-pass-plugin %shlibdir/libDynamicCallCounter%shlibext -passes="dynamic-cc,verify" -o %t.bin
; RUN: lli %t.bin | FileCheck %s

; Instrument this file with DynamicCallCounter, run it and verify that it
; generates the expected output.

; CHECK: bar                  2
; CHECK-NEXT: main                 1
; CHECK-NEXT: foo                  13
; CHECK-NEXT: fez                  1
