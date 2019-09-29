; RUN: %clang -c -emit-llvm %S/../inputs/input_for_cc.c -o %t
; RUN: ../bin/static %t 2>&1 | FileCheck %s

; Test StaticCallCounter when run via static. This is the expected output after
; running static analysis. Note that calls via function pointers are not
; counted.
; CHECK: a                    2
; CHECK: b                    2
; CHECK: foo                  1
; CHECK: printf               7
; CHECK-NOT: g                1
; CHECK-NOT: h                1
