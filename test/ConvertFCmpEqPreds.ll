; RUN: opt -load %shlibdir/libFindFCmpEq%shlibext  -load %shlibdir/libConvertFCmpEq%shlibext -convert-fcmp-eq  -S %s \
; RUN:  | FileCheck %s
; RUN: opt -load-pass-plugin=%shlibdir/libFindFCmpEq%shlibext  -load-pass-plugin=%shlibdir/libConvertFCmpEq%shlibext --passes=convert-fcmp-eq  -S %s \
; RUN:  | FileCheck %s

define i32 @fp_cmp(double %a, double %b) {
; CHECK-LABEL: @fp_cmp
; CHECK-DAG: %1 = fsub double %a, %b
; CHECK-NEXT: %2 = bitcast double %1 to i64
; CHECK-NEXT: %3 = and i64 %2, 9223372036854775807
; CHECK-NEXT: %4 = bitcast i64 %3 to double
; CHECK-NEXT: %cmp = fcmp olt double %4, 0x3CB0000000000000
; CHECK-NEXT: %conv = zext i1 %cmp to i32
; CHECK-NOT: fcmp oeq
; CHECK-DAG: ret i32 %conv

  %cmp = fcmp oeq double %a, %b
  %conv = zext i1 %cmp to i32
  ret i32 %conv
}

define i32 @fp_cmp1(double %a, double %b) {
; CHECK-LABEL: @fp_cmp1
; CHECK-DAG: %1 = fsub double %a, %b
; CHECK-NEXT: %2 = bitcast double %1 to i64
; CHECK-NEXT: %3 = and i64 %2, 9223372036854775807
; CHECK-NEXT: %4 = bitcast i64 %3 to double
; CHECK-NEXT: %cmp = fcmp uge double %4, 0x3CB0000000000000
; CHECK-NEXT: %conv = zext i1 %cmp to i32
; CHECK-NOT: fcmp une
; CHECK-DAG: ret i32 %conv

  %cmp = fcmp une double %a, %b
  %conv = zext i1 %cmp to i32
  ret i32 %conv
}

define i32 @fp_cmp2(double %a, double %b) {
; CHECK-DAG: @fp_cmp2
; CHECK-NEXT: %fneg = fneg double %a
; CHECK-NEXT: %1 = fsub double %fneg, %b
; CHECK-NEXT: %2 = bitcast double %1 to i64
; CHECK-NEXT: %3 = and i64 %2, 9223372036854775807
; CHECK-NEXT: %4 = bitcast i64 %3 to double
; CHECK-NEXT: %cmp = fcmp olt double %4, 0x3CB0000000000000
; CHECK-NEXT: %conv = zext i1 %cmp to i32
; CHECK-NOT: fcmp oeq
; CHECK-DAG: ret i32 %conv

  %fneg = fneg double %a
  %cmp = fcmp oeq double %fneg, %b
  %conv = zext i1 %cmp to i32
  ret i32 %conv
}

define i32 @fp_cmp3(double %a, double %b) {
; CHECK-LABEL: @fp_cmp3
; CHECK-DAG %fneg = fneg double %a
; CHECK-NEXT %1 = fsub double %fneg, %b
; CHECK-NEXT %2 = bitcast double %1 to i64
; CHECK-NEXT %3 = and i64 %2, 9223372036854775807
; CHECK-NEXT %4 = bitcast i64 %3 to double
; CHECK-NEXT %cmp = fcmp uge double %4, 0x3CB0000000000000
; CHECK-NEXT %conv = zext i1 %cmp to i32
; CHECK-NOT: fcmp une
; CHECK-DAG ret i32 %conv

  %fneg = fneg double %a
  %cmp = fcmp une double %fneg, %b
  %conv = zext i1 %cmp to i32
  ret i32 %conv
}
