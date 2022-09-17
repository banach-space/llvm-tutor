; RUN:  opt -load-pass-plugin %shlibdir/libRIV%shlibext -passes="print<riv>" -disable-output %s 2>&1 | FileCheck %s

; Verifies that the result from the RIV pass for the following module is
; correct. Note that all values are integers and should be included in the
; results.

define i32 @foo(i32 %a, i32 %b, i32 %c) {
entry:
  %add = add nsw i32 %a, 123
  %cmp = icmp sgt i32 %a, 0
  br i1 %cmp, label %if.then, label %if.end8

if.then:                                          ; preds = %entry
  %mul = mul nsw i32 %b, %a
  %div = sdiv i32 %b, %c
  %cmp1 = icmp eq i32 %mul, %div
  br i1 %cmp1, label %if.then2, label %if.else

if.then2:                                         ; preds = %if.then
  %mul4 = mul i32 %mul, -2
  %mul4.neg = mul i32 %mul4, %div
  %sub = add i32 %add, %mul4.neg
  br label %if.end8

if.else:                                          ; preds = %if.then
  %mul5 = mul nsw i32 %c, 987
  %mul6 = mul nsw i32 %mul5, %div
  br label %if.end8

if.end8:                                          ; preds = %entry, %if.then2, %if.else
  %result.1 = phi i32 [ %sub, %if.then2 ], [ %mul6, %if.else ], [ 321, %entry ]
  ret i32 %result.1
}

; CHECK-LABEL: BB %entry
; CHECK-NEXT:        i32 %a
; CHECK-NEXT:        i32 %b
; CHECK-NEXT:        i32 %c

; CHECK-LABEL: BB %if.then
; CHECK-NEXT:          %add = add nsw i32 %a, 123
; CHECK-NEXT:          %cmp = icmp sgt i32 %a, 0
; CHECK-NEXT:        i32 %a
; CHECK-NEXT:        i32 %b
; CHECK-NEXT:        i32 %c

; CHECK-LABEL: BB %if.end8
; CHECK-NEXT:          %add = add nsw i32 %a, 123
; CHECK-NEXT:          %cmp = icmp sgt i32 %a, 0
; CHECK-NEXT:        i32 %a
; CHECK-NEXT:        i32 %b
; CHECK-NEXT:        i32 %c

; CHECK-LABEL: BB %if.then2
; CHECK-NEXT:          %mul = mul nsw i32 %b, %a
; CHECK-NEXT:          %div = sdiv i32 %b, %c
; CHECK-NEXT:          %cmp1 = icmp eq i32 %mul, %div
; CHECK-NEXT:          %add = add nsw i32 %a, 123
; CHECK-NEXT:          %cmp = icmp sgt i32 %a, 0
; CHECK-NEXT:        i32 %a
; CHECK-NEXT:        i32 %b
; CHECK-NEXT:        i32 %c

; CHECK-LABEL: BB %if.else
; CHECK-NEXT:          %mul = mul nsw i32 %b, %a
; CHECK-NEXT:          %div = sdiv i32 %b, %c
; CHECK-NEXT:          %cmp1 = icmp eq i32 %mul, %div
; CHECK-NEXT:          %add = add nsw i32 %a, 123
; CHECK-NEXT:          %cmp = icmp sgt i32 %a, 0
; CHECK-NEXT:        i32 %a
; CHECK-NEXT:        i32 %b
; CHECK-NEXT:        i32 %c
