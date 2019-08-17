; RUN: opt -load ../lib/libMBA%shlibext -mba -mba-ratio=1 -S %s  | FileCheck -check-prefix=MBA %s
; RUN: opt -load ../lib/libMBA%shlibext -mba -mba-ratio=0 -S %s  | FileCheck -check-prefix=NO_MBA %s

; Verify that -mba-ratio has the expected effect.

; 1st '+'
; MBA:        %1 = xor i32 %in_0, %in_1
; MBA-NEXT:   %2 = and i32 %in_0, %in_1
; MBA-NEXT:   %3 = mul i32 2, %2
; MBA-NEXT:   %4 = add i32 %1, %3
; 2nd '+'
; MBA-NEXT:   %5 = xor i32 %in_2, %4
; MBA-NEXT:   %6 = and i32 %in_2, %4
; MBA-NEXT:   %7 = mul i32 2, %6
; MBA-NEXT:   %8 = add i32 %5, %7
; 3rd '+'
; MBA-NEXT:   %9 = xor i32 %in_3, %8
; MBA-NEXT:   %10 = and i32 %in_3, %8
; MBA-NEXT:   %11 = mul i32 2, %10
; MBA-NEXT:   %12 = add i32 %9, %11
; 4th '+'
; MBA-NEXT:   %13 = xor i32 %in_4, %12
; MBA-NEXT:   %14 = and i32 %in_4, %12
; MBA-NEXT:   %15 = mul i32 2, %14
; MBA-NEXT:   %16 = add i32 %13, %15
; MBA-NEXT:   ret i32 %16

; 1st '+'
; NO_MBA:  %1 = add nsw i32 %in_0, %in_1
; 2nd '+'
; NO_MBA-NEXT:  %2 = add nsw i32 %in_2, %1
; 3rd '+'
; NO_MBA-NEXT:  %3 = add nsw i32 %in_3, %2
; 4th '+'
; NO_MBA-NEXT:  %4 = add nsw i32 %in_4, %3
; NO_MBA-NO:  xor 
; NO_MBA-NO:  and
; NO_MBA-NO:  mul
; NO_MBA-NEXT:  ret i32 %4

define i32 @main(i32 %in_0, i32 %in_1, i32 %in_2, i32 %in_3, i32 %in_4)  {
  %1 = add nsw i32 %in_0, %in_1
  %2 = add nsw i32 %in_2, %1
  %3 = add nsw i32 %in_3, %2
  %4 = add nsw i32 %in_4, %3
  ret i32 %4
}
