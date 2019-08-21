; RUN: opt -load ../lib/libRIV%shlibext -riv -analyze -S %s  | FileCheck %s

; Verifies that the result from the RIV pass for the following module is correct

define i32 @fez() #0 {
  %1 = alloca i32, align 4
  %2 = alloca i32, align 4
  %3 = alloca i32, align 4
  store i32 1, i32* %1, align 4
  store i32 2, i32* %2, align 4
  %4 = load i32, i32* %1, align 4
  %5 = load i32, i32* %2, align 4
  %6 = add nsw i32 %4, %5
  store i32 %6, i32* %3, align 4
  %7 = load i32, i32* %1, align 4
  %8 = icmp ne i32 %7, 0
  br i1 %8, label %9, label %10

; <label>:9:                                      ; preds = %0
  store i32 3, i32* %2, align 4
  br label %10

; <label>:10:                                     ; preds = %9, %0
  %11 = load i32, i32* %3, align 4
  ret i32 %11
}

; Function Attrs: noinline nounwind optnone ssp uwtable
define i32 @foo(i32) #0 {
  %2 = alloca i32, align 4
  %3 = alloca i32, align 4
  %4 = alloca i32, align 4
  %5 = alloca i32, align 4
  store i32 %0, i32* %2, align 4
  store i32 1, i32* %3, align 4
  store i32 2, i32* %4, align 4
  %6 = load i32, i32* %3, align 4
  %7 = load i32, i32* %4, align 4
  %8 = add nsw i32 %6, %7
  store i32 %8, i32* %5, align 4
  %9 = load i32, i32* %5, align 4
  ret i32 %9
}

; Function Attrs: noinline nounwind optnone ssp uwtable
define i32 @bar() #0 {
  %1 = alloca i32, align 4
  %2 = alloca i32, align 4
  %3 = alloca i32, align 4
  %4 = call i32 @foo(i32 1)
  store i32 %4, i32* %1, align 4
  store i32 0, i32* %2, align 4
  store i32 0, i32* %2, align 4
  br label %5

; <label>:5:                                      ; preds = %13, %0
  %6 = load i32, i32* %2, align 4
  %7 = icmp slt i32 %6, 10
  br i1 %7, label %8, label %16

; <label>:8:                                      ; preds = %5
  %9 = load i32, i32* %2, align 4
  store i32 %9, i32* %3, align 4
  %10 = load i32, i32* %3, align 4
  %11 = load i32, i32* %1, align 4
  %12 = add nsw i32 %11, %10
  store i32 %12, i32* %1, align 4
  br label %13

; <label>:13:                                     ; preds = %8
  %14 = load i32, i32* %2, align 4
  %15 = add nsw i32 %14, 1
  store i32 %15, i32* %2, align 4
  br label %5

; <label>:16:                                     ; preds = %5
  %17 = load i32, i32* %1, align 4
  ret i32 %17
}

; Function Attrs: noinline nounwind optnone ssp uwtable
define i32 @main(i32, i8**) #0 {
  %3 = alloca i32, align 4
  %4 = alloca i32, align 4
  %5 = alloca i8**, align 8
  store i32 0, i32* %3, align 4
  store i32 %0, i32* %4, align 4
  store i8** %1, i8*** %5, align 8
  %6 = call i32 @bar()
  ret i32 %6
}

; CHECK: 'fez':
; CHECK: BB 0                                        
; CHECK: BB 9                                        
; CHECK:                   %4 = load i32, i32* %1, align 4
; CHECK:                   %5 = load i32, i32* %2, align 4
; CHECK:                   %6 = add nsw i32 %4, %5     
; CHECK:                   %7 = load i32, i32* %1, align 4
; CHECK:                   %8 = icmp ne i32 %7, 0      
; CHECK: BB 10                                       
; CHECK:                   %4 = load i32, i32* %1, align 4
; CHECK:                   %5 = load i32, i32* %2, align 4
; CHECK:                   %6 = add nsw i32 %4, %5     
; CHECK:                   %7 = load i32, i32* %1, align 4
; CHECK:                   %8 = icmp ne i32 %7, 0      

; CHECK: 'foo':
; CHECK: BB 1                                        
; CHECK:                 i32 %0                        

; CHECK: 'bar':
; CHECK: BB 0                                        
; CHECK: BB 5                                        
; CHECK:                   %4 = call i32 @foo(i32 1)   
; CHECK: BB 8                                        
; CHECK:                   %6 = load i32, i32* %2, align 4
; CHECK:                   %7 = icmp slt i32 %6, 10    
; CHECK:                   %4 = call i32 @foo(i32 1)   
; CHECK: BB 13                                       
; CHECK:                   %9 = load i32, i32* %2, align 4
; CHECK:                   %10 = load i32, i32* %3, align 4
; CHECK:                   %11 = load i32, i32* %1, align 4
; CHECK:                   %12 = add nsw i32 %11, %10  
; CHECK:                   %6 = load i32, i32* %2, align 4
; CHECK:                   %7 = icmp slt i32 %6, 10    
; CHECK:                   %4 = call i32 @foo(i32 1)   
; CHECK: BB 16                                       
; CHECK:                   %6 = load i32, i32* %2, align 4
; CHECK:                   %7 = icmp slt i32 %6, 10    
; CHECK:                   %4 = call i32 @foo(i32 1)   

; CHECK-LABEL: 'main':
; CHECK: BB 2                                        
; CHECK:                 i32 %0                        
; CHECK:                 i8** %1                       
