; RUN: clang -c -emit-llvm %S/../inputs/input_for_cc.c -o %t
; RUN: ../bin/lt-cc -static %t | FileCheck %s

; Test 'static-cc' when run via lt-cc. This is the expected output after
; running static analysis. Note that calls via function pointers are not
; counted.
; CHECK: a                    2
; CHECK: b                    2
; CHECK: foo                  1
; CHECK: printf               7
; CHECK-NOT: g                1
; CHECK-NOT: h                1
