; RUN:  opt --load-pass-plugin=%shlibdir/libFindFCmpEq%shlibext \
; RUN:   --load-pass-plugin=%shlibdir/libConvertFCmpEq%shlibext \
; RUN:   --passes="convert-fcmp-eq" -S %S/Inputs/FCmpEqInput.ll  | FileCheck %s


; CHECK-LABEL:  @sqrt_impl
; CHECK:        %3 = fsub double %0, %2
; CHECK-NEXT:   %4 = bitcast double %3 to i64
; CHECK-NEXT:   %5 = and i64 %4, 9223372036854775807
; CHECK-NEXT:   %6 = bitcast i64 %5 to double
; CHECK-NEXT:   %7 = fcmp olt double %6, 0x3CB0000000000000
; CHECK-NEXT:   br i1 %7, label %root_found, label %approximate

; CHECK-LABEL:  @compare_fp_values
; CHECK:        %0 = tail call double @sqrt(double 5.000000e+00)
; CHECK-NEXT:   %1 = fdiv double 1.000000e+00, %0
; CHECK-NEXT:   %2 = tail call double @sqrt(double 5.000000e+00)
; CHECK-NEXT:   %b = fdiv double %1, %2
; CHECK-NEXT:   %3 = fsub double %a, %b
; CHECK-NEXT:   %4 = bitcast double %3 to i64
; CHECK-NEXT:   %5 = and i64 %4, 9223372036854775807
; CHECK-NEXT:   %6 = bitcast i64 %5 to double
; CHECK-NEXT:   %cmp = fcmp olt double %6, 0x3CB0000000000000
; CHECK-NEXT:   ret i1 %cmp
