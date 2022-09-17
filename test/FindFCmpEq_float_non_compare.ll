; RUN:  opt --load-pass-plugin=%shlibdir/libFindFCmpEq%shlibext --passes="print<find-fcmp-eq>" -disable-output %s | FileCheck -allow-empty %s

; Verify that float comparisons that are not "equal" comparisons are ignored
; by FindFCmpEq

; CHECK-NOT: Floating-point equality comparisons in {{.*}}

define i32 @fcmp_false(double %a, double %b) {
  ; No compare, always return false
  %cmp = fcmp false double %a, %b
  %conv = zext i1 %cmp to i32
  ret i32 %conv
}

define i32 @fcmp_true(double %a, double %b) {
  ; No compare, always return true
  %cmp = fcmp true double %a, %b
  %conv = zext i1 %cmp to i32
  ret i32 %conv
}

define i32 @fcmp_ogt(double %a, double %b) {
  ; a > b
  %cmp = fcmp ogt double %a, %b
  %conv = zext i1 %cmp to i32
  ret i32 %conv
}

define i32 @fcmp_oge(double %a, double %b) {
  ; a >= b
  %cmp = fcmp oge double %a, %b
  %conv = zext i1 %cmp to i32
  ret i32 %conv
}

define i32 @fcmp_olt(double %a, double %b) {
  ; a < b
  %cmp = fcmp olt double %a, %b
  %conv = zext i1 %cmp to i32
  ret i32 %conv
}

define i32 @fcmp_ole(double %a, double %b) {
  ; a <= b
  %cmp = fcmp ole double %a, %b
  %conv = zext i1 %cmp to i32
  ret i32 %conv
}

define i32 @fcmp_ord(double %a, double %b) {
  ; a != NaN && b != NaN
  %cmp = fcmp ord double %a, %b
  %conv = zext i1 %cmp to i32
  ret i32 %conv
}

define i32 @fcmp_ugt(double %a, double %b) {
  ; a > b
  %cmp = fcmp ogt double %a, %b
  %conv = zext i1 %cmp to i32
  ret i32 %conv
}

define i32 @fcmp_uge(double %a, double %b) {
  ; a >= b
  %cmp = fcmp oge double %a, %b
  %conv = zext i1 %cmp to i32
  ret i32 %conv
}

define i32 @fcmp_ult(double %a, double %b) {
  ; a < b
  %cmp = fcmp olt double %a, %b
  %conv = zext i1 %cmp to i32
  ret i32 %conv
}

define i32 @fcmp_ule(double %a, double %b) {
  ; a <= b
  %cmp = fcmp ole double %a, %b
  %conv = zext i1 %cmp to i32
  ret i32 %conv
}
