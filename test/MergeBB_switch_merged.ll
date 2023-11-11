; RUN: opt -load-pass-plugin %shlibdir/libMergeBB%shlibext -passes=merge-bb -S %s | FileCheck  %s

define i32 @main(i32) {
  switch i32 %0, label %4 [
    i32 1, label %2
    i32 2, label %3
  ]

; <label>:2:
  br label %5

; <label>:3:
  br label %5

; <label>:4:
  br label %5

; <label>:5:
  %6 = phi i32 [ 10, %4 ], [ 20, %3 ], [ 10, %2 ]
  ret i32 %6
}


; CHECK-LABEL: main
; CHECK-NEXT:  switch i32 %0, label %3 [
; CHECK-NEXT:    i32 1, label %3
; CHECK-NEXT:    i32 2, label %2
; CHECK-NEXT:  ]

; CHECK-LABEL: 2:
; CHECK-NEXT:  br label %4

; CHECK-LABEL: 3:
; CHECK-NEXT:  br label %4

; CHECK-LABEL: 4:
; CHECK-NEXT:  %5 = phi i32 [ 10, %3 ], [ 20, %2 ]
; CHECK-NEXT:  ret i32 %5

