; Generated from <llvm-tutor>/inputs/input_for_fcmp_eq.c

@value.test = private constant double 2.000000e-01, align 8

define double @sqrt_impl(double %x, double %hi, double %lo) {
entry:
  br label %newton_raphson

newton_raphson:                                   ; preds = %approximate, %entry
  %0 = phi double [ %hi, %entry ], [ %6, %approximate ]
  %1 = phi double [ %lo, %entry ], [ %2, %approximate ]
  br label %compare_limits

compare_limits:                                   ; preds = %newton_raphson, %converge
  %2 = phi double [ %9, %converge ], [ %1, %newton_raphson ]
  %3 = fcmp oeq double %0, %2
  br i1 %3, label %root_found, label %approximate

approximate:                                      ; preds = %compare_limits
  ; midpoint = (%lo + %hi + 1) / 2
  %4 = fadd double %0, %2
  %5 = fadd double %4, 1.000000e+00
  %6 = fmul double %5, 5.000000e-01
  ; candidate = %x / midpoint
  %7 = fdiv double %x, %6
  %8 = fcmp olt double %7, %6
  br i1 %8, label %converge, label %newton_raphson
  
converge:                                         ; preds = %approximate
  %9 = fadd double %6, -1.000000e+00
  br label %compare_limits

root_found:                                       ; preds = %compare_limits
  ret double %0
}

define double @sqrt(double %x) {
entry:
  %half = fmul double %x, 5.000000e-01
  %hi = fadd double %half, 1.000000e+00
  %root = tail call double @sqrt_impl(double %x, double 0.000000e+00, double %hi)
  ret double %root
}

define i32 @main() {
entry:
  %a = load double, double* @value.test, align 8
  ; %b = 1.0 / sqrt(5.0) / sqrt(5.0)
  %0 = tail call double @sqrt(double 5.000000e+00)
  %1 = fdiv double 1.000000e+00, %0
  %2 = tail call double @sqrt(double 5.000000e+00)
  %b = fdiv double %1, %2

  %cmp = fcmp oeq double %a, %b
  %result = zext i1 %cmp to i32
  ret i32 %result
}
