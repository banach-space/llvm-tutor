; RUN: lli %S/Inputs/FCmpEqInput.ll
; RUN: opt --load-pass-plugin=%shlibdir/libFindFCmpEq%shlibext \
; RUN:   --load-pass-plugin=%shlibdir/libConvertFCmpEq%shlibext \
; RUN:   --passes="convert-fcmp-eq" -S %S/Inputs/FCmpEqInput.ll -o %t.bin
; RUN: not lli %t.bin
