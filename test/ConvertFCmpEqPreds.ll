; RUN: opt -load-pass-plugin=%shlibdir/libFindFCmpEq%shlibext  -load-pass-plugin=%shlibdir/libConvertFCmpEq%shlibext --passes=convert-fcmp-eq  -S %s \
; RUN:  | FileCheck %s

define i32 @fcmp_oeq(double %a, double %b) {
; CHECK-LABEL: @fcmp_oeq
; CHECK-DAG: %1 = fsub double %a, %b
; CHECK-NEXT: %2 = bitcast double %1 to i64
; CHECK-NEXT: %3 = and i64 %2, 9223372036854775807
; CHECK-NEXT: %4 = bitcast i64 %3 to double
; CHECK-NEXT: %cmp = fcmp olt double %4, 0x3CB0000000000000
; CHECK-NEXT: %conv = zext i1 %cmp to i32
; CHECK-NOT: fcmp oeq
; CHECK-DAG: ret i32 %conv

  ; a == b
  %cmp = fcmp oeq double %a, %b
  %conv = zext i1 %cmp to i32
  ret i32 %conv
}

define i32 @fcmp_une(double %a, double %b) {
; CHECK-LABEL: @fcmp_une
; CHECK-DAG: %1 = fsub double %a, %b
; CHECK-NEXT: %2 = bitcast double %1 to i64
; CHECK-NEXT: %3 = and i64 %2, 9223372036854775807
; CHECK-NEXT: %4 = bitcast i64 %3 to double
; CHECK-NEXT: %cmp = fcmp uge double %4, 0x3CB0000000000000
; CHECK-NEXT: %conv = zext i1 %cmp to i32
; CHECK-NOT: fcmp une
; CHECK-DAG: ret i32 %conv

  ; a != b
  %cmp = fcmp une double %a, %b
  %conv = zext i1 %cmp to i32
  ret i32 %conv
}

define i32 @fcmp_neg_oeq(double %a, double %b) {
; CHECK-DAG: @fcmp_neg_oeq
; CHECK-NEXT: %fneg = fneg double %a
; CHECK-NEXT: %1 = fsub double %fneg, %b
; CHECK-NEXT: %2 = bitcast double %1 to i64
; CHECK-NEXT: %3 = and i64 %2, 9223372036854775807
; CHECK-NEXT: %4 = bitcast i64 %3 to double
; CHECK-NEXT: %cmp = fcmp olt double %4, 0x3CB0000000000000
; CHECK-NEXT: %conv = zext i1 %cmp to i32
; CHECK-NOT: fcmp oeq
; CHECK-DAG: ret i32 %conv

  ; -a == b
  %fneg = fneg double %a
  %cmp = fcmp oeq double %fneg, %b
  %conv = zext i1 %cmp to i32
  ret i32 %conv
}

define i32 @fcmp_neg_une(double %a, double %b) {
; CHECK-LABEL: @fcmp_neg_une
; CHECK-DAG %fneg = fneg double %a
; CHECK-NEXT %1 = fsub double %fneg, %b
; CHECK-NEXT %2 = bitcast double %1 to i64
; CHECK-NEXT %3 = and i64 %2, 9223372036854775807
; CHECK-NEXT %4 = bitcast i64 %3 to double
; CHECK-NEXT %cmp = fcmp uge double %4, 0x3CB0000000000000
; CHECK-NEXT %conv = zext i1 %cmp to i32
; CHECK-NOT: fcmp une
; CHECK-DAG ret i32 %conv

  ; -a != b
  %fneg = fneg double %a
  %cmp = fcmp une double %fneg, %b
  %conv = zext i1 %cmp to i32
  ret i32 %conv
}
