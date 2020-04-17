; RUN:  opt -load %shlibdir/libStaticCallCounter%shlibext --legacy-static-cc -analyze %S/Inputs/CallCounterInput.ll \
; RUN:   | FileCheck %s

; Test StaticCallCounter when run through opt (possible with the Legacy PM
; only)

; CHECK: foo                  3
; CHECK: bar                  2
; CHECK: fez                  1
