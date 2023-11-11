; RUN: %clang -S -emit-llvm %S/../inputs/input_for_mba_sub.c -o - \
; RUN:   | opt  -load-pass-plugin=%shlibdir/libMBASub%shlibext -passes="mba-sub" -S -o %t.ll
; RUN: %clang %t.ll -o %t.bin

; The program implemented in input_for_mba_sub.c takes for inputs and subs them up,
; and returns the result. So if they sub up to 0, then ; the binary returns `0`
; (aka success). Verify that the obfuscation didn't violate this invariant.
; RUN: %t.bin 0 0 0 0
; RUN: %t.bin 1 10 -10 -1
; RUN: %t.bin 13 13 13 13
; RUN: %t.bin 11100 100 1000 -10000

; If the input values don't subtract to 0, then the result shouldn't sub to `0`.
; Use `not` to negate the result so that we still test for `success`.
; RUN: not %t.bin 0 0 0 1
; RUN: not %t.bin 1 2 3 -7
; RUN: not %t.bin 13 13 -13 13
; RUN: not %t.bin -11101 100 1000 10000
