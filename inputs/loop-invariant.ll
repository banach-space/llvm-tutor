; Simple test loop for SimpleLICM
define i32 @test(i32 %a, i32 %b) {
entry:
  br label %loop

loop:
  %i = phi i32 [0, %entry], [%inc, %loop]
  %mul = mul i32 %a, %b
  %inc = add i32 %i, 1
  %cmp = icmp slt i32 %i, 10
  br i1 %cmp, label %loop, label %exit

exit:
  ret i32 %mul
}

