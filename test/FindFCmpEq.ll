; RUN:  opt --load-pass-plugin=%shlibdir/libFindFCmpEq%shlibext \
; RUN:   --passes="print<find-fcmp-eq>" %S/Inputs/FCmpEqInput.ll  | FileCheck %s



; CHECK: Floating-point equality comparisons in "sqrt_impl":
; CHECK-NEXT:   %3 = fcmp oeq double %0, %2
; CHECK-NEXT: Floating-point equality comparisons in "compare_fp_values":
; CHECK-NEXT:   %cmp = fcmp oeq double %a, %b
