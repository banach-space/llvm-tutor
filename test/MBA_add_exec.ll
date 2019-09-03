; RUN: clang -S -emit-llvm %S/../inputs/input_for_mba.c -o - \
; RUN:   | opt -load ../lib/libMBAAdd%shlibext -legacy-mba-add -S -o %instrumented_add.ll
; RUN: clang %instrumented_add.ll -o %out.bin

; The program implemented in input_for_mba.c takes for inputs and adds them up,
; and returns the result. So if they add up to 0, then ; the binary returns `0`
; (aka success). Verify that the obfuscation didn't violate this invariant.
; RUN: ./%out.bin 0 0 0 0
; RUN: ./%out.bin 1 2 3 -6
; RUN: ./%out.bin -13 13 -13 13
; RUN: ./%out.bin -11100 100 1000 10000

; If the input values don't add up to 0, then the result shouldn't add to `0`.
; Use `not` to negate the result so that we still test for `success`.
; RUN: not ./%out.bin 0 0 0 1
; RUN: not ./%out.bin 1 2 3 -7
; RUN: not ./%out.bin 13 13 -13 13
; RUN: not ./%out.bin -11101 100 1000 10000
