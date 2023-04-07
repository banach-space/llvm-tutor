; RUN: opt -passes=memcpyopt -S < %s | FileCheck %s

; memcpyopt will only work if the specified value can be set by repeating the
; same byte in memory. This is always true for i8 values, but not necesarilly for i16, i32, etc.

declare void @llvm.memcpy.p0i8.p0i8.i64(i8* nocapture, i8* nocapture, i64, i1) nounwind

;------------------------------------------------------------------------------
; CASE 1: The input array contains values that _do_ qualify for optimisation
;------------------------------------------------------------------------------
; 160 = OxA0
@src_array_i8 = internal constant [3 x i8] [i8 160, i8 160, i8 160], align 4
define void @input_i8() nounwind {
  %dest_array = alloca [3 x i8], align 4
  %dest_ptr = bitcast [3 x i8]* %dest_array to i8*
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 4 %dest_ptr, i8* align 4 bitcast ([3 x i8]* @src_array_i8 to i8*), i64 12, i1 false)
  ret void
; CHECK-LABEL: @input_i8(
; CHECK:       call void @llvm.memset
; CHECK-NOT:   call void @llvm.memcpy
; CHECK:       ret void
}

; 41120 = OxA0A0
@src_array_i16 = internal constant [3 x i16] [i16 41120, i16 41120, i16 41120], align 4
define void @input_i16() nounwind {
  %dest_array = alloca [3 x i16], align 4
  %dest_ptr = bitcast [3 x i16]* %dest_array to i8*
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 4 %dest_ptr, i8* align 4 bitcast ([3 x i16]* @src_array_i16 to i8*), i64 12, i1 false)
  ret void
; CHECK-LABEL: @input_i16(
; CHECK:       call void @llvm.memset
; CHECK-NOT:   call void @llvm.memcpy
; CHECK:       ret void
}

; 2694881440 = OxA0A0A0A0
@src_array_i32 = internal constant [3 x i32] [i32 2694881440, i32 2694881440, i32 2694881440], align 4
define void @input_i32() nounwind {
  %dest_array = alloca [3 x i32], align 4
  %dest_ptr = bitcast [3 x i32]* %dest_array to i8*
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 4 %dest_ptr, i8* align 4 bitcast ([3 x i32]* @src_array_i32 to i8*), i64 12, i1 false)
  ret void
; CHECK-LABEL: @input_i32(
; CHECK:       call void @llvm.memset
; CHECK-NOT:   call void @llvm.memcpy
; CHECK:       ret void
}

;------------------------------------------------------------------------------
; CASE 2: The input array contains values that _do not_ qualify for optimisation
;------------------------------------------------------------------------------
@src_array_i16_v2 = internal constant [3 x i16] [i16 123, i16 123, i16 123], align 4
define void @input_i16_v2() nounwind {
  %dest_array = alloca [3 x i16], align 4
  %dest_ptr = bitcast [3 x i16]* %dest_array to i8*
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 4 %dest_ptr, i8* align 4 bitcast ([3 x i16]* @src_array_i16_v2 to i8*), i64 12, i1 false)
  ret void
; CHECK-LABEL: @input_i16_v2(
; CHECK-NOT:   call void @llvm.memset
; CHECK:       call void @llvm.memcpy
; CHECK:       ret void
}

@src_array_i32_v2 = internal constant [3 x i32] [i32 123, i32 123, i32 123], align 4
define void @input_i32_v2() nounwind {
  %dest_array = alloca [3 x i32], align 4
  %dest_ptr = bitcast [3 x i32]* %dest_array to i8*
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 4 %dest_ptr, i8* align 4 bitcast ([3 x i32]* @src_array_i32_v2 to i8*), i64 12, i1 false)
  ret void
; CHECK-LABEL: @input_i32_v2(
; CHECK-NOT:   call void @llvm.memset
; CHECK:       call void @llvm.memcpy
; CHECK:       ret void
}

;------------------------------------------------------------------------------
; CASE 3: The input array contains an undef value - optimised
;------------------------------------------------------------------------------
@src_array_undef = internal constant [3 x i32] [i32 -1, i32 undef, i32 -1], align 4

define void @input_undef() nounwind {
  %dest_array = alloca [3 x i32], align 4
  %dest_ptr = bitcast [3 x i32]* %dest_array to i8*
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 4 %dest_ptr, i8* align 4 bitcast ([3 x i32]* @src_array_undef to i8*), i64 12, i1 false)
  ret void
; CHECK-LABEL: @input_undef(
; CHECK:       call void @llvm.memset
; CHECK-NOT:   call void @llvm.memcpy
; CHECK:       ret void
}
