; RUN:  opt --load-pass-plugin=%shlibdir/libFindFCmpEq%shlibext --passes="print<find-fcmp-eq>" -disable-output %s | FileCheck -allow-empty %s

define i32 @fcmp_oeq(double %a, double %b) {
; CHECK-NOT: Floating-point equality comparisons in fcmp_oeq
  ; a == b
  %cmp = fcmp oeq double %a, %b
  %conv = zext i1 %cmp to i32
  ret i32 %conv
}

define i32 @fcmp_une(double %a, double %b) {
; CHECK-NOT: Floating-point equality comparisons in fcmp_une
  ; a != b
  %cmp = fcmp une double %a, %b
  %conv = zext i1 %cmp to i32
  ret i32 %conv
}

define i32 @fcmp_neg_oeq(double %a, double %b) {
; CHECK-NOT: Floating-point equality comparisons in fcmp_neg_oeq
  ; -a == b
  %fneg = fneg double %a
  %cmp = fcmp oeq double %fneg, %b
  %conv = zext i1 %cmp to i32
  ret i32 %conv
}

define i32 @fcmp_neg_une(double %a, double %b) {
; CHECK-NOT: Floating-point equality comparisons in fcmp_neg_une
  ; -a != b
  %fneg = fneg double %a
  %cmp = fcmp une double %fneg, %b
  %conv = zext i1 %cmp to i32
  ret i32 %conv
}
