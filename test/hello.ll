; RUN: clang -S -emit-llvm %S/../inputs/input_for_hello.c -o - \
; RUN:   | opt -load ../lib/libHelloWorld%shlibext --legacy-hello-world -disable-output 2>&1\
; RUN:   | FileCheck %s
; RUN: clang -S -emit-llvm %S/../inputs/input_for_hello.c -o - \
; RUN:   | opt -load-pass-plugin ../lib/libHelloWorld%shlibext -passes=hello-world -disable-output 2>&1\
; RUN:   | FileCheck %s

; Test 'hello-world' when run through opt (both new and legacy PMs, as well
; within the -O{0|1|2|3} pipeline)
; CHECK: Visiting: foo (takes 1 args)
; CHECK: Visiting: bar (takes 2 args)
; CHECK: Visiting: fez (takes 3 args)
; CHECK: Visiting: main (takes 2 args)
