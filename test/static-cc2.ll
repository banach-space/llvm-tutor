; RUN: %clang -c -emit-llvm %S/../inputs/input_for_cc.c -o %t
; RUN: ../bin/static %t 2>&1 | FileCheck %s

; Test StaticCallCounter when run via static. This is the expected output after
; running static analysis. Note that calls via function pointers are not
; counted.
; CHECK: bar                  2
; CHECK: fez                  1
; CHECK: foo                  3
