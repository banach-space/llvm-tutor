; RUN:  opt --load-pass-plugin=%shlibdir/libFindFCmpEq%shlibext --passes="print<find-fcmp-eq>" -disable-output %s | FileCheck -allow-empty %s

; Verify that integer comparisons are ignored by FindFCmpEq

; CHECK-NOT: Floating-point equality comparisons in {{.*}}

define i32 @icmp_eq(i32 %a, i32 %b) {
  ; a = b
  %cmp = icmp eq i32 %a, %b
  %conv = zext i1 %cmp to i32
  ret i32 %conv
}

define i32 @icmp_ne(i32 %a, i32 %b) {
  ; a != b
  %cmp = icmp ne i32 %a, %b
  %conv = zext i1 %cmp to i32
  ret i32 %conv
}

define i32 @icmp_ugt(i32 %a, i32 %b) {
  ; a > b
  %cmp = icmp ugt i32 %a, %b
  %conv = zext i1 %cmp to i32
  ret i32 %conv
}

define i32 @icmp_uge(i32 %a, i32 %b) {
  ; a >= b
  %cmp = icmp uge i32 %a, %b
  %conv = zext i1 %cmp to i32
  ret i32 %conv
}

define i32 @icmp_ult(i32 %a, i32 %b) {
  ; a < b
  %cmp = icmp ult i32 %a, %b
  %conv = zext i1 %cmp to i32
  ret i32 %conv
}

define i32 @icmp_ule(i32 %a, i32 %b) {
  ; a <= b
  %cmp = icmp ule i32 %a, %b
  %conv = zext i1 %cmp to i32
  ret i32 %conv
}

define i32 @icmp_sgt(i32 %a, i32 %b) {
  ; a > b
  %cmp = icmp sgt i32 %a, %b
  %conv = zext i1 %cmp to i32
  ret i32 %conv
}

define i32 @icmp_sge(i32 %a, i32 %b) {
  ; a >= b
  %cmp = icmp sge i32 %a, %b
  %conv = zext i1 %cmp to i32
  ret i32 %conv
}

define i32 @icmp_slt(i32 %a, i32 %b) {
  ; a < b
  %cmp = icmp slt i32 %a, %b
  %conv = zext i1 %cmp to i32
  ret i32 %conv
}

define i32 @icmp_sle(i32 %a, i32 %b) {
  ; a <= b
  %cmp = icmp sle i32 %a, %b
  %conv = zext i1 %cmp to i32
  ret i32 %conv
}
