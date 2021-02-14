; RUN: opt --basic-aa -slp-vectorizer -dce -mtriple=x86_64-unknown-linuxgnu -S %s

; The input function can be expressed in C as:
; void foo(int a1, int a2, int b1, int b2, int *A) {
;   A[0] = a1*(a1 + b1);
;   A[1] = a2*(a2 + b2);
;   A[2] = a1*(a1 + b1);
;   A[3] = a2*(a2 + b2);
; }
define void @foo(i32, i32, i32, i32, i32* nocapture) local_unnamed_addr #0 {
; CHECK-LABEL: foo
; CHECK-NEXT:  %6 = insertelement <2 x i32> undef, i32 %2, i32 0
; CHECK-NEXT:  %7 = insertelement <2 x i32> %6, i32 %3, i32 1
; CHECK-NEXT:  %8 = insertelement <2 x i32> undef, i32 %0, i32 0
; CHECK-NEXT:  %9 = insertelement <2 x i32> %8, i32 %1, i32 1
  ; %10 = [a1, a2] + [b1, b2] = [a1 + b1, a2 + b2]
; CHECK-NEXT:  %10 = add nsw <2 x i32> %7, %9
  ; %11 = [a1 * (a1 + b1), a2 * (a2 + b2)]
; CHECK-NEXT:  %11 = mul nsw <2 x i32> %10, %9
; CHECK-NEXT:  %shuffle = shufflevector <2 x i32> %11, <2 x i32> undef, <4 x i32> <i32 0, i32 1, i32 0, i32 1>
  ; Not used
; CHECK-NEXT: %15 = bitcast i32* %4 to <4 x i32>*
  ; A = [shuffle[0], shuffle[1], shuffle[2], shuffle[3]]
; CHECK-NEXT:  store <4 x i32> %shuffle, <4 x i32>* %15, align 4
; CHECK-NEXT:  ret void

  %6 = add nsw i32 %2, %0
  %7 = mul nsw i32 %6, %0
  store i32 %7, i32* %4, align 4
  %8 = add nsw i32 %3, %1
  %9 = mul nsw i32 %8, %1
  %10 = getelementptr inbounds i32, i32* %4, i64 1
  store i32 %9, i32* %10, align 4
  %11 = getelementptr inbounds i32, i32* %4, i64 2
  store i32 %7, i32* %11, align 4
  %12 = getelementptr inbounds i32, i32* %4, i64 3
  store i32 %9, i32* %12, align 4
  ret void
}

attributes #0 = {  "target-cpu"="penryn" }
