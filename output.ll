; ModuleID = '../test-licm.ll'
source_filename = "../test-licm.ll"

define i32 @foo(i32 %a, i32 %b) {
entry:
  %x = add i32 %a, 5
  %y = add i32 %x, %b
  br label %loop

loop:                                             ; preds = %loop, %entry
  %i = phi i32 [ 0, %entry ], [ %inc, %loop ]
  %inc = add i32 %i, 1
  %cond = icmp slt i32 %inc, 10
  br i1 %cond, label %loop, label %exit

exit:                                             ; preds = %loop
  %y.lcssa = phi i32 [ %y, %loop ]
  ret i32 %y.lcssa
}
