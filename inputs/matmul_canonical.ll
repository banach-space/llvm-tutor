; ModuleID = 'matmul_canonical.c'
source_filename = "matmul_canonical.c"
target datalayout = "e-m:o-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-apple-macosx15.0.0"

; Function Attrs: nofree norecurse nosync nounwind ssp memory(argmem: readwrite) uwtable
define void @matmul(ptr noundef readonly captures(none) %0, ptr noundef readonly captures(none) %1, ptr noundef writeonly captures(none) %2) local_unnamed_addr #0 {
  br label %4

4:                                                ; preds = %3, %9
  %5 = phi i64 [ 0, %3 ], [ %10, %9 ]
  br label %7

6:                                                ; preds = %9
  ret void

7:                                                ; preds = %4, %12
  %8 = phi i64 [ 0, %4 ], [ %14, %12 ]
  br label %16

9:                                                ; preds = %12
  %10 = add nuw nsw i64 %5, 1
  %11 = icmp eq i64 %10, 800
  br i1 %11, label %6, label %4, !llvm.loop !6

12:                                               ; preds = %16
  %13 = getelementptr inbounds nuw [800 x double], ptr %2, i64 %5, i64 %8
  store double %23, ptr %13, align 8, !tbaa !9
  %14 = add nuw nsw i64 %8, 1
  %15 = icmp eq i64 %14, 800
  br i1 %15, label %9, label %7, !llvm.loop !13

16:                                               ; preds = %7, %16
  %17 = phi i64 [ 0, %7 ], [ %24, %16 ]
  %18 = phi double [ 0.000000e+00, %7 ], [ %23, %16 ]
  %19 = getelementptr inbounds nuw [800 x double], ptr %0, i64 %5, i64 %17
  %20 = load double, ptr %19, align 8, !tbaa !9
  %21 = getelementptr inbounds nuw [800 x double], ptr %1, i64 %17, i64 %8
  %22 = load double, ptr %21, align 8, !tbaa !9
  %23 = tail call double @llvm.fmuladd.f64(double %20, double %22, double %18)
  %24 = add nuw nsw i64 %17, 1
  %25 = icmp eq i64 %24, 800
  br i1 %25, label %12, label %16, !llvm.loop !14
}

; Function Attrs: mustprogress nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare double @llvm.fmuladd.f64(double, double, double) #1

attributes #0 = { nofree norecurse nosync nounwind ssp memory(argmem: readwrite) uwtable "frame-pointer"="all" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cmov,+cx16,+cx8,+fxsr,+mmx,+sahf,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "tune-cpu"="generic" }
attributes #1 = { mustprogress nocallback nofree nosync nounwind speculatable willreturn memory(none) }

!llvm.module.flags = !{!0, !1, !2, !3, !4}
!llvm.ident = !{!5}

!0 = !{i32 2, !"SDK Version", [2 x i32] [i32 15, i32 5]}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{i32 8, !"PIC Level", i32 2}
!3 = !{i32 7, !"uwtable", i32 2}
!4 = !{i32 7, !"frame-pointer", i32 2}
!5 = !{!"Homebrew clang version 21.1.2"}
!6 = distinct !{!6, !7, !8}
!7 = !{!"llvm.loop.mustprogress"}
!8 = !{!"llvm.loop.unroll.disable"}
!9 = !{!10, !10, i64 0}
!10 = !{!"double", !11, i64 0}
!11 = !{!"omnipotent char", !12, i64 0}
!12 = !{!"Simple C/C++ TBAA"}
!13 = distinct !{!13, !7, !8}
!14 = distinct !{!14, !7, !8}
