// RUN: clang -S -emit-llvm %S/../test_examples/MBA.c -o - \
// RUN:   | opt -load ../lib/libMBA%shlibext -mba -S \
// RUN:   | FileCheck %s

// The input file has 3 additions. Verify that these are correctly replaced as
// follows:
//    a + b == (a ^ b) + 2 * (a & b)
  
// CHECK:       [[REG_1:%[0-9]+]] = load i32, i32* {{%[0-9]+}}, align 4
// CHECK-NEXT:  [[REG_2:%[0-9]+]] = load i32, i32* {{%[0-9]+}}, align 4
// CHECK-NEXT:  [[REG_3:%[0-9]+]] = xor i32 [[REG_1]], [[REG_2]]
// CHECK-NEXT:  [[REG_4:%[0-9]+]] = and i32 [[REG_1]], [[REG_2]]
// CHECK-NEXT:  [[REG_5:%[0-9]+]] = mul i32 2, [[REG_4]]
// CHECK-NEXT:  [[REG_6:%[0-9]+]] = add i32 [[REG_3]], [[REG_5]]
//
// CHECK:       [[REG_7:%[0-9]+]] = load i32, i32* {{%[0-9]+}}, align 4
// CHECK-NEXT:  [[REG_8:%[0-9]+]] = load i32, i32* {{%[0-9]+}}, align 4
// CHECK-NEXT:  [[REG_9:%[0-9]+]] = xor i32 [[REG_7]], [[REG_8]]
// CHECK-NEXT:  [[REG_10:%[0-9]+]] = and i32 [[REG_7]], [[REG_8]]
// CHECK-NEXT:  [[REG_11:%[0-9]+]] = mul i32 2, [[REG_10]]
// CHECK-NEXT:  [[REG_12:%[0-9]+]] = add i32 [[REG_9]], [[REG_11]]
//
// CHECK:       [[REG_13:%[0-9]+]] = load i32, i32* {{%[0-9]+}}, align 4
// CHECK-NEXT:  [[REG_14:%[0-9]+]] = load i32, i32* {{%[0-9]+}}, align 4
// CHECK-NEXT:  [[REG_15:%[0-9]+]] = xor i32 [[REG_13]], [[REG_14]]
// CHECK-NEXT:  [[REG_16:%[0-9]+]] = and i32 [[REG_13]], [[REG_14]]
// CHECK-NEXT:  [[REG_17:%[0-9]+]] = mul i32 2, [[REG_16]]
// CHECK-NEXT:  [[REG_18:%[0-9]+]] = add i32 [[REG_15]], [[REG_17]]
// CHECK-NOT:   xor 
// CHECK-NOT:   and
// CHECK-NOT:   mul
// CHECK-NOT:   add
