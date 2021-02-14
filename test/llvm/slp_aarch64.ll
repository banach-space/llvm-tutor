; RUN: opt --basic-aa -slp-vectorizer -dce -S -mtriple=aarch64-unknown-linuxgnu %s

; The input function can be expressed in C as:
; int hadd(int *a) {
;   return a[0] + a[1] + a[2] + a[3]
; }
define i32 @hadd(i32* %a) {
; CHECK-LABEL: hadd
; CHECK-NEXT:  %0 = bitcast i32* %a to <4 x i32>*
; CHECK-NEXT:  %1 = load <4 x i32>, <4 x i32>* %0, align 4
; CHECK-NEXT:  %2 = call i32 @llvm.experimental.vector.reduce.add.v4i32(<4 x i32> %1)
; CHECK-NEXT:  ret i32 %2

entry:
  %0 = load i32, i32* %a, align 4

  %arrayidx1 = getelementptr inbounds i32, i32* %a, i32 1
  %1 = load i32, i32* %arrayidx1, align 4
  %add = add nsw i32 %0, %1

  %arrayidx2 = getelementptr inbounds i32, i32* %a, i32 2
  %2 = load i32, i32* %arrayidx2, align 4
  %add3 = add nsw i32 %add, %2

  %arrayidx3 = getelementptr inbounds i32, i32* %a, i32 3
  %3 = load i32, i32* %arrayidx3, align 4
  %add5 = add nsw i32 %add3, %3

  ret i32 %add5
}
