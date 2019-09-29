; RUN: %clang -c -emit-llvm %S/../inputs/input_for_cc.c -o %t
; RUN: ../bin/dynamic %t -o %instrumented.bin
; RUN: ./%instrumented.bin | FileCheck %s

; Test the dynamic call counter pass. This is the expected output after
; running the dynamic analysis.
; CHECK: a                    21
; CHECK: printf               28
; CHECK: b                    2
; CHECK: c                    0
; CHECK: foo                  3
; CHECK: bar                  0
; CHECK: main                 1
