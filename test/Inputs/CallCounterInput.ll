define void @foo() {
  ret void
}

define void @bar() {
  call void @foo()
  ret void
}

define void @fez() {
  call void @bar()
  ret void
}

define i32 @main() {
  %1 = alloca i32, align 4
  %2 = alloca i32, align 4
  store i32 0, i32* %1, align 4
  call void @foo()
  call void @bar()
  call void @fez()
  store i32 0, i32* %2, align 4
  store i32 0, i32* %2, align 4
  br label %3

  %4 = load i32, i32* %2, align 4
  %5 = icmp slt i32 %4, 10
  br i1 %5, label %6, label %10

  call void @foo()
  br label %7

  %8 = load i32, i32* %2, align 4
  %9 = add nsw i32 %8, 1
  store i32 %9, i32* %2, align 4
  br label %3

  ret i32 0
}
