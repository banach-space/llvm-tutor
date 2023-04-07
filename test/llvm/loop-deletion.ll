; RUN: opt -passes=loop-deletion -S < %s | FileCheck %s

;------------------------------------------------------------------------------
; CASE 1: A loop that's never executed
;------------------------------------------------------------------------------
define void @bar(i64 %n, i64 %m) nounwind {
; CHECK-LABEL: bar
; CHECK-LABEL: entry:
; CHECK-NEXT: br i1 true, label %return, label %header_block.preheader
; CHECK-NOT: header_block:
; CHECK-NOT: latch_block:
entry:
  br i1 true, label %return, label %header_block

header_block:
  %x.0 = phi i64 [ 0, %entry ], [ %t0, %latch_block ]
  %t0 = add i64 %x.0, 1
  %t1 = icmp slt i64 %x.0, %n
  %t3 = icmp sgt i64 %x.0, %m
  %t4 = and i1 %t1, %t3
  br label %latch_block

latch_block:
  br i1 true, label %header_block, label %return

return:
  ret void
}

;------------------------------------------------------------------------------
; CASE 2: A finite loop that does not affect the return value
;------------------------------------------------------------------------------
define void @foo(i64 %n, i64 %m) nounwind {
; CHECK-LABEL: foo
; CHECK-LABEL:  entry:
; CHECK:  br label %return
; CHECK-LABEL: return:
; CHECK: ret void
entry:
  br label %header_block

header_block:
  %x.0 = phi i64 [ 0, %entry ], [ %t0, %latch_block ]
  %t0 = add i64 %x.0, 1
  %t1 = icmp slt i64 %x.0, %n
  br i1 %t1, label %latch_block, label %return

latch_block:
  %t2 = icmp slt i64 %x.0, %m
  br i1 %t1, label %header_block, label %return

return:
  ret void
}
