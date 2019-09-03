; RUN: clang -S -emit-llvm %S/../inputs/input_for_mba_sub.c -o - \
; RUN:   | opt -load ../lib/libMBASub%shlibext -legacy-mba-sub -S -o %instrumented_sub.ll
; RUN: clang %instrumented_sub.ll -o %out1.bin

; The program implemented in input_for_mba_sub.c takes for inputs and subs them up,
; and returns the result. So if they sub up to 0, then ; the binary returns `0`
; (aka success). Verify that the obfuscation didn't violate this invariant.
; RUN: ./%out1.bin 0 0 0 0
; RUN: ./%out1.bin 1 10 -10 -1
; RUN: ./%out1.bin 13 13 13 13
; RUN: ./%out1.bin 11100 100 1000 -10000

; If the input values don't subtract to 0, then the result shouldn't sub to `0`.
; Use `not` to negate the result so that we still test for `success`.
; RUN: not ./%out1.bin 0 0 0 1
; RUN: not ./%out1.bin 1 2 3 -7
; RUN: not ./%out1.bin 13 13 -13 13
; RUN: not ./%out1.bin -11101 100 1000 10000
