; RUN: %clang -c -emit-llvm %S/../inputs/input_for_cc.c -o - \
; RUN:   | opt -load ../lib/libStaticCallCounter%shlibext --legacy-static-cc -analyze \
; RUN:   | FileCheck %s

; Test StaticCallCounter when run through opt. This is the expected output after
; running static analysis. Note that calls via function pointers are not
; counted.
; CHECK: bar                  2
; CHECK: fez                  1
; CHECK: foo                  3
