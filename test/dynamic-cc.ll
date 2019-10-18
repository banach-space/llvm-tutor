; RUN: %clang -c -emit-llvm %S/../inputs/input_for_cc.c -o %t
; RUN: ../bin/dynamic %t -o instrumented.bin
; RUN: ./instrumented.bin | FileCheck %s

; Test the dynamic call counter pass. This is the expected output after
; running the dynamic analysis.
; CHECK: foo                  13
; CHECK: bar                  2
; CHECK: fez                  1
; CHECK: main                 1
