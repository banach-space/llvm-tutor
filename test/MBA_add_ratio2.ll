; RUN: not opt --enable-new-pm=0 -load %shlibdir/libMBAAdd%shlibext -legacy-mba-add -mba-ratio=100 -S %s 2>&1 | FileCheck %s

;  Verify that wrong MBA ratio triggers an adequate error

; CHECK: opt: for the --mba-ratio option: '100' is not in [0., 1.] 

define i32 @main(i32 %in_0, i32 %in_1, i32 %in_2, i32 %in_3, i32 %in_4) {
  %1 = add nsw i32 %in_0, %in_1
  %2 = add nsw i32 %in_2, %1
  %3 = add nsw i32 %in_3, %2
  %4 = add nsw i32 %in_4, %3
  ret i32 %4
}
