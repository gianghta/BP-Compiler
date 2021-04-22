; ModuleID = 'runtime.c'

%struct.__sFILE = type { i8*, i32, i32, i16, i16, %struct.__sbuf, i32, i8*, i32 (i8*)*, i32 (i8*, i8*, i32)*, i64 (i8*, i64, i32)*, i32 (i8*, i8*, i32)*, %struct.__sbuf, %struct.__sFILEX*, i32, [3 x i8], [1 x i8], %struct.__sbuf, i32, i64 }
%struct.__sFILEX = type opaque
%struct.__sbuf = type { i8*, i32 }

@.str = private unnamed_addr constant [3 x i8] c"%i\00", align 1
@.str.1 = private unnamed_addr constant [3 x i8] c"%f\00", align 1
@__stdinp = external global %struct.__sFILE*, align 8
@.str.2 = private unnamed_addr constant [4 x i8] c"%i\0A\00", align 1
@.str.3 = private unnamed_addr constant [4 x i8] c"%f\0A\00", align 1
@.str.4 = private unnamed_addr constant [4 x i8] c"%s\0A\00", align 1
@.str.5 = private unnamed_addr constant [28 x i8] c"Error: Index out of bounds\0A\00", align 1

; Function Attrs: noinline nounwind optnone ssp uwtable
define zeroext i1 @getbool() #0 {
  %1 = alloca i32, align 4
  %2 = call i32 (i8*, ...) @scanf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @.str, i64 0, i64 0), i32* %1)
  %3 = call i32 @getchar()
  %4 = load i32, i32* %1, align 4
  %5 = icmp ne i32 %4, 0
  ret i1 %5
}

declare i32 @scanf(i8*, ...) #1

declare i32 @getchar() #1

; Function Attrs: noinline nounwind optnone ssp uwtable
define i32 @getinteger() #0 {
  %1 = alloca i32, align 4
  %2 = call i32 (i8*, ...) @scanf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @.str, i64 0, i64 0), i32* %1)
  %3 = call i32 @getchar()
  %4 = load i32, i32* %1, align 4
  ret i32 %4
}

; Function Attrs: noinline nounwind optnone ssp uwtable
define float @getfloat() #0 {
  %1 = alloca float, align 4
  %2 = call i32 (i8*, ...) @scanf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @.str.1, i64 0, i64 0), float* %1)
  %3 = call i32 @getchar()
  %4 = load float, float* %1, align 4
  ret float %4
}

; Function Attrs: noinline nounwind optnone ssp uwtable
define i8* @getstring() #0 {
  %1 = alloca i8*, align 8
  %2 = call i8* @malloc(i64 256) #5
  store i8* %2, i8** %1, align 8
  %3 = load i8*, i8** %1, align 8
  %4 = load %struct.__sFILE*, %struct.__sFILE** @__stdinp, align 8
  %5 = call i8* @fgets(i8* %3, i32 256, %struct.__sFILE* %4)
  %6 = load i8*, i8** %1, align 8
  %7 = call i64 @strlen(i8* %6)
  %8 = icmp ugt i64 %7, 0
  br i1 %8, label %9, label %24

9:                                                ; preds = %0
  %10 = load i8*, i8** %1, align 8
  %11 = load i8*, i8** %1, align 8
  %12 = call i64 @strlen(i8* %11)
  %13 = sub i64 %12, 1
  %14 = getelementptr inbounds i8, i8* %10, i64 %13
  %15 = load i8, i8* %14, align 1
  %16 = sext i8 %15 to i32
  %17 = icmp eq i32 %16, 10
  br i1 %17, label %18, label %24

18:                                               ; preds = %9
  %19 = load i8*, i8** %1, align 8
  %20 = load i8*, i8** %1, align 8
  %21 = call i64 @strlen(i8* %20)
  %22 = sub i64 %21, 1
  %23 = getelementptr inbounds i8, i8* %19, i64 %22
  store i8 0, i8* %23, align 1
  br label %24

24:                                               ; preds = %18, %9, %0
  %25 = load i8*, i8** %1, align 8
  ret i8* %25
}

; Function Attrs: allocsize(0)
declare i8* @malloc(i64) #2

declare i8* @fgets(i8*, i32, %struct.__sFILE*) #1

declare i64 @strlen(i8*) #1

; Function Attrs: noinline nounwind optnone ssp uwtable
define zeroext i1 @putbool(i1 zeroext %0) #0 {
  %2 = alloca i8, align 1
  %3 = zext i1 %0 to i8
  store i8 %3, i8* %2, align 1
  %4 = load i8, i8* %2, align 1
  %5 = trunc i8 %4 to i1
  %6 = zext i1 %5 to i32
  %7 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([4 x i8], [4 x i8]* @.str.2, i64 0, i64 0), i32 %6)
  ret i1 true
}

declare i32 @printf(i8*, ...) #1

; Function Attrs: noinline nounwind optnone ssp uwtable
define zeroext i1 @putinteger(i32 %0) #0 {
  %2 = alloca i32, align 4
  store i32 %0, i32* %2, align 4
  %3 = load i32, i32* %2, align 4
  %4 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([4 x i8], [4 x i8]* @.str.2, i64 0, i64 0), i32 %3)
  ret i1 true
}

; Function Attrs: noinline nounwind optnone ssp uwtable
define zeroext i1 @putfloat(float %0) #0 {
  %2 = alloca float, align 4
  store float %0, float* %2, align 4
  %3 = load float, float* %2, align 4
  %4 = fpext float %3 to double
  %5 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([4 x i8], [4 x i8]* @.str.3, i64 0, i64 0), double %4)
  ret i1 true
}

; Function Attrs: noinline nounwind optnone ssp uwtable
define zeroext i1 @putstring(i8* %0) #0 {
  %2 = alloca i8*, align 8
  store i8* %0, i8** %2, align 8
  %3 = load i8*, i8** %2, align 8
  %4 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([4 x i8], [4 x i8]* @.str.4, i64 0, i64 0), i8* %3)
  ret i1 true
}

; Function Attrs: noinline nounwind optnone ssp uwtable
define float @_sqrt(i32 %0) #0 {
  %2 = alloca i32, align 4
  store i32 %0, i32* %2, align 4
  %3 = load i32, i32* %2, align 4
  %4 = sitofp i32 %3 to float
  %5 = call float @llvm.sqrt.f32(float %4)
  ret float %5
}

; Function Attrs: nofree nosync nounwind readnone speculatable willreturn
declare float @llvm.sqrt.f32(float) #3

; Function Attrs: noinline nounwind optnone ssp uwtable
define void @outOfBoundsError() #0 {
  %1 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([28 x i8], [28 x i8]* @.str.5, i64 0, i64 0))
  call void @exit(i32 1) #6
  unreachable
}

; Function Attrs: noreturn
declare void @exit(i32) #4

attributes #0 = { noinline nounwind optnone ssp uwtable "frame-pointer"="all" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+cx8,+fxsr,+mmx,+sahf,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "tune-cpu"="generic" }
attributes #1 = { "frame-pointer"="all" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+cx8,+fxsr,+mmx,+sahf,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "tune-cpu"="generic" }
attributes #2 = { allocsize(0) "frame-pointer"="all" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+cx8,+fxsr,+mmx,+sahf,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "tune-cpu"="generic" }
attributes #3 = { nofree nosync nounwind readnone speculatable willreturn }
attributes #4 = { noreturn "frame-pointer"="all" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+cx8,+fxsr,+mmx,+sahf,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "tune-cpu"="generic" }
attributes #5 = { allocsize(0) }
attributes #6 = { noreturn }

!llvm.module.flags = !{!0, !1}
!llvm.ident = !{!2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"PIC Level", i32 2}
!2 = !{!"clang version 13.0.0 (https://github.com/llvm/llvm-project.git be0ffbb21f6631a73679d371da5315db3eb3537c)"}
