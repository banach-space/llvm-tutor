; RUN: clang -c -emit-llvm %S/../test_examples/example_1.c -o %t
; RUN: opt -load ../lib/libCallCounter%shlibext --static-cc -analyze %t | FileCheck %s

; This is the expected output after running static analysis. Note calls via
function pointers are not counted.
; CHECK: a                    2
; CHECK: b                    2
; CHECK: foo                  1
; CHECK: printf               7
; CHECK-NOT: g                1
; CHECK-NOT: h                1
