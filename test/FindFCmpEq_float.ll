; RUN:  opt --load-pass-plugin=%shlibdir/libFindFCmpEq%shlibext --passes="print<find-fcmp-eq>" %S/Inputs/FCmpEqInput.ll  | FileCheck %s

; Verify that FindFCmpEq correctly finds floating point comparison operations
; in a slightly more complex example (e.g. with function calls)

; CHECK-LABEL: Floating-point equality comparisons in "sqrt_impl":
; CHECK-NEXT:   %3 = fcmp oeq double %0, %2
; CHECK-LABEL: Floating-point equality comparisons in "main":
; CHECK-NEXT:   %cmp = fcmp oeq double %a, %b
