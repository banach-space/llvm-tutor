; RUN:  opt -load-pass-plugin %shlibdir/libRIV%shlibext -passes="print<riv>" -disable-output %s 2>&1 | FileCheck %s

; Verifies that the result from the RIV pass for the following module is
; correct. Note that most values are floats. These values should be ignored by
; the pass.

define float @foo(float %a, float %b, float %c) {
entry:
  %add = fadd float %a, 1.230000e+02
  %cmp = fcmp ogt float %a, 0.000000e+00
  br i1 %cmp, label %if.then, label %if.end8

if.then:                                          ; preds = %entry
  %mul = fmul float %a, %b
  %div = fdiv float %b, %c
  %cmp1 = fcmp oeq float %mul, %div
  br i1 %cmp1, label %if.then2, label %if.else

if.then2:                                         ; preds = %if.then
  %mul3 = fmul float %mul, %div
  %mul4 = fmul float %mul3, 2.000000e+00
  %sub = fsub float %add, %mul4
  br label %if.end8

if.else:                                          ; preds = %if.then
  %mul5 = fmul float %c, 9.870000e+02
  %mul6 = fmul float %mul5, %div
  br label %if.end8

if.end8:                                          ; preds = %entry, %if.then2, %if.else
  %result.1 = phi float [ %sub, %if.then2 ], [ %mul6, %if.else ], [ 3.210000e+02, %entry ]
  ret float %result.1
}

; CHECK-LABEL: BB %entry
; CHECK-LABEL: BB %if.then
; CHECK-NEXT:          %cmp = fcmp ogt float %a, 0.000000e+00
; CHECK-LABEL: BB %if.end8
; CHECK-NEXT:          %cmp = fcmp ogt float %a, 0.000000e+00
; CHECK-LABEL: BB %if.then2
; CHECK-NEXT:          %cmp1 = fcmp oeq float %mul, %div
; CHECK-NEXT:          %cmp = fcmp ogt float %a, 0.000000e+00
; CHECK-LABEL: BB %if.else
; CHECK-NEXT:          %cmp1 = fcmp oeq float %mul, %div
; CHECK-NEXT:          %cmp = fcmp ogt float %a, 0.000000e+00
