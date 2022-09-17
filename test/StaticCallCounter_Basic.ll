; RUN:  opt -load-pass-plugin %shlibdir/libStaticCallCounter%shlibext -passes="print<static-cc>" -disable-output \
; RUN:   %S/Inputs/CallCounterInput.ll 2>&1 | FileCheck %s

; Test StaticCallCounter when run through opt using - basic function calls

; CHECK: foo                  3
; CHECK: bar                  2
; CHECK: fez                  1
