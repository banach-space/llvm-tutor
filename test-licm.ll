; test-licm.ll
; A simple loop with invariant computation

define i32 @foo(i32 %a, i32 %b) {
entry:
  br label %loop

loop:
  %i = phi i32 [0, %entry], [%inc, %loop]
  %x = add i32 %a, 5          ; loop-invariant
  %y = add i32 %x, %b         ; loop-invariant
  %inc = add i32 %i, 1
  %cond = icmp slt i32 %inc, 10
  br i1 %cond, label %loop, label %exit

exit:
  ret i32 %y
}

