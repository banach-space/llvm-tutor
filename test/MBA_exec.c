// RUN: clang -S -emit-llvm %S/../test_examples/MBA.c -o - \
// RUN:   | opt -load ../lib/liblt-mba-shared%shlibext -mba -S -o %instrumented.ll
// RUN: clang %instrumented.ll -o %instrumented.bin
// RUN: ./%instrumented.bin -13 13 -13 13
// RUN: not ./%instrumented.bin 13 13 13 13
