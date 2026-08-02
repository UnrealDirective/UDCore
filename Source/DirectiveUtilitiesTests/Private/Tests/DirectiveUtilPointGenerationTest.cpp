// Copyright (c) 2026 Unreal Directive. Licensed under the MIT License.

#include "Libraries/DirectiveUtilMathFunctionLibrary.h"

#include "Misc/AutomationTest.h"

#include <limits>

namespace
{
	bool PointsEqual(const TArray<FVector>& A, const TArray<FVector>& B, const double Tolerance = 1.e-9)
	{
		if (A.Num() != B.Num())
		{
			return false;
		}
		for (int32 Index = 0; Index < A.Num(); ++Index)
		{
			if (!A[Index].Equals(B[Index], Tolerance))
			{
				return false;
			}
		}
		return true;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDirectiveUtilPointGenerationTest,
	"DirectiveUtilities.Math.PointGeneration",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::EngineFilter)

bool FDirectiveUtilPointGenerationTest::RunTest(const FString& Parameters)
{
	const TArray<FVector> Grid2D = UDirectiveUtilMathFunctionLibrary::GenerateGridPoints2D(
		FVector::ZeroVector, FRotator::ZeroRotator, FIntPoint(3, 2), FVector2D(10.0, 20.0), true);
	const TArray<FVector> ExpectedGrid2D = {
		FVector(-10.0, -10.0, 0.0), FVector(0.0, -10.0, 0.0), FVector(10.0, -10.0, 0.0),
		FVector(-10.0, 10.0, 0.0), FVector(0.0, 10.0, 0.0), FVector(10.0, 10.0, 0.0)
	};
	TestTrue(TEXT("2D grid is centered and ordered by X then Y"), PointsEqual(Grid2D, ExpectedGrid2D));

	const TArray<FVector> RotatedGrid2D = UDirectiveUtilMathFunctionLibrary::GenerateGridPoints2D(
		FVector(5.0, 7.0, 9.0), FRotator(0.0, 90.0, 0.0), FIntPoint(2, 1), FVector2D(3.0, 0.0), false);
	TestTrue(TEXT("2D grid rotation places its local X axis in world space"),
		RotatedGrid2D.Num() == 2
		&& RotatedGrid2D[0].Equals(FVector(5.0, 7.0, 9.0), 1.e-9)
		&& RotatedGrid2D[1].Equals(FVector(5.0, 10.0, 9.0), 1.e-9));

	const TArray<FVector> Grid3D = UDirectiveUtilMathFunctionLibrary::GenerateGridPoints3D(
		FVector::ZeroVector, FRotator::ZeroRotator, FIntVector(2, 2, 2), FVector(2.0, 4.0, 6.0), true);
	TestTrue(TEXT("3D grid is centered and ordered by X then Y then Z"),
		Grid3D.Num() == 8
		&& Grid3D[0].Equals(FVector(-1.0, -2.0, -3.0), 1.e-9)
		&& Grid3D[1].Equals(FVector(1.0, -2.0, -3.0), 1.e-9)
		&& Grid3D[2].Equals(FVector(-1.0, 2.0, -3.0), 1.e-9)
		&& Grid3D.Last().Equals(FVector(1.0, 2.0, 3.0), 1.e-9));
	TestTrue(TEXT("Grid generation rejects non-positive dimensions"),
		UDirectiveUtilMathFunctionLibrary::GenerateGridPoints2D(
			FVector::ZeroVector, FRotator::ZeroRotator, FIntPoint(2, 0), FVector2D(1.0), true).IsEmpty());
	TestTrue(TEXT("Grid generation rejects point-count overflow"),
		UDirectiveUtilMathFunctionLibrary::GenerateGridPoints3D(
			FVector::ZeroVector, FRotator::ZeroRotator, FIntVector(MAX_int32, 2, 2), FVector::OneVector, true).IsEmpty());

	const TArray<FVector> DirectionPoints = UDirectiveUtilMathFunctionLibrary::GeneratePointsAlongDirection(
		FVector::ZeroVector, FVector(10.0, 0.0, 0.0), 4, 2.0, true);
	const TArray<FVector> ExpectedDirectionPoints = {
		FVector(-3.0, 0.0, 0.0), FVector(-1.0, 0.0, 0.0),
		FVector(1.0, 0.0, 0.0), FVector(3.0, 0.0, 0.0)
	};
	TestTrue(TEXT("Direction points normalize once and center around the origin"),
		PointsEqual(DirectionPoints, ExpectedDirectionPoints));
	const TArray<FVector> ReversedDirectionPoints = UDirectiveUtilMathFunctionLibrary::GeneratePointsAlongDirection(
		FVector::ZeroVector, FVector::ForwardVector, 3, -2.0, false);
	TestTrue(TEXT("Direction points preserve signed spacing"),
		ReversedDirectionPoints.Num() == 3
		&& ReversedDirectionPoints[0].Equals(FVector::ZeroVector)
		&& ReversedDirectionPoints[2].Equals(FVector(-4.0, 0.0, 0.0)));
	TestTrue(TEXT("Direction points reject a zero direction"),
		UDirectiveUtilMathFunctionLibrary::GeneratePointsAlongDirection(
			FVector::ZeroVector, FVector::ZeroVector, 3, 1.0, false).IsEmpty());

	const TArray<FVector> SegmentPoints = UDirectiveUtilMathFunctionLibrary::GeneratePointsBetweenLocations(
		FVector::ZeroVector, FVector(10.0, 0.0, 0.0), 3, true);
	TestTrue(TEXT("Segment points include exact endpoints"),
		SegmentPoints.Num() == 3
		&& SegmentPoints[0] == FVector::ZeroVector
		&& SegmentPoints[1].Equals(FVector(5.0, 0.0, 0.0))
		&& SegmentPoints[2] == FVector(10.0, 0.0, 0.0));
	const TArray<FVector> InteriorSegmentPoints = UDirectiveUtilMathFunctionLibrary::GeneratePointsBetweenLocations(
		FVector::ZeroVector, FVector(9.0, 0.0, 0.0), 2, false);
	TestTrue(TEXT("Segment points can exclude both endpoints"),
		InteriorSegmentPoints.Num() == 2
		&& InteriorSegmentPoints[0].Equals(FVector(3.0, 0.0, 0.0))
		&& InteriorSegmentPoints[1].Equals(FVector(6.0, 0.0, 0.0)));
	const TArray<FVector> SingleSegmentPoint = UDirectiveUtilMathFunctionLibrary::GeneratePointsBetweenLocations(
		FVector(2.0, 4.0, 6.0), FVector(6.0, 8.0, 10.0), 1, true);
	TestTrue(TEXT("A single segment point is the midpoint"),
		SingleSegmentPoint.Num() == 1 && SingleSegmentPoint[0].Equals(FVector(4.0, 6.0, 8.0)));

	const TArray<FVector> CirclePoints = UDirectiveUtilMathFunctionLibrary::GeneratePointsOnCircle(
		FVector::ZeroVector, FRotator::ZeroRotator, 10.0, 4, 0.0);
	const TArray<FVector> ExpectedCirclePoints = {
		FVector(10.0, 0.0, 0.0), FVector(0.0, 10.0, 0.0),
		FVector(-10.0, 0.0, 0.0), FVector(0.0, -10.0, 0.0)
	};
	TestTrue(TEXT("Circle points are evenly spaced without repeating the first point"),
		PointsEqual(CirclePoints, ExpectedCirclePoints, 1.e-8));

	const FRotator PlaneRotation(17.0, 31.0, 43.0);
	const FVector PlaneNormal = PlaneRotation.Quaternion().GetAxisZ();
	const FVector PlaneCenter(11.0, 13.0, 17.0);
	const TArray<FVector> RotatedCircle = UDirectiveUtilMathFunctionLibrary::GeneratePointsOnCircle(
		PlaneCenter, PlaneRotation, -7.0, 64, 15.0);
	bool bCirclePlaneValid = RotatedCircle.Num() == 64;
	for (const FVector& Point : RotatedCircle)
	{
		const FVector Offset = Point - PlaneCenter;
		bCirclePlaneValid &= FMath::IsNearlyEqual(Offset.Size(), 7.0, 1.e-8);
		bCirclePlaneValid &= FMath::IsNearlyZero(FVector::DotProduct(Offset, PlaneNormal), 1.e-8);
	}
	TestTrue(TEXT("Circle points honor rotation and negative radius"), bCirclePlaneValid);

	const TArray<FVector> ArcPoints = UDirectiveUtilMathFunctionLibrary::GeneratePointsOnArc(
		FVector::ZeroVector, FRotator::ZeroRotator, 10.0, 3, 0.0, 90.0, true);
	TestTrue(TEXT("Arc points include the requested endpoint"),
		ArcPoints.Num() == 3
		&& ArcPoints[0].Equals(FVector(10.0, 0.0, 0.0), 1.e-8)
		&& ArcPoints[1].Equals(FVector(UE_DOUBLE_INV_SQRT_2 * 10.0, UE_DOUBLE_INV_SQRT_2 * 10.0, 0.0), 1.e-8)
		&& ArcPoints[2].Equals(FVector(0.0, 10.0, 0.0), 1.e-8));
	const TArray<FVector> OpenArcPoints = UDirectiveUtilMathFunctionLibrary::GeneratePointsOnArc(
		FVector::ZeroVector, FRotator::ZeroRotator, 10.0, 3, 0.0, 90.0, false);
	TestTrue(TEXT("Open arc points exclude the requested endpoint"),
		OpenArcPoints.Num() == 3
		&& OpenArcPoints.Last().Equals(
			FVector(FMath::Cos(UE_DOUBLE_PI / 3.0) * 10.0, FMath::Sin(UE_DOUBLE_PI / 3.0) * 10.0, 0.0), 1.e-8));

	const TArray<FVector> DiscPoints = UDirectiveUtilMathFunctionLibrary::GeneratePointsOnDisc(
		PlaneCenter, PlaneRotation, 25.0, 1024, 27.0);
	const TArray<FVector> RepeatedDiscPoints = UDirectiveUtilMathFunctionLibrary::GeneratePointsOnDisc(
		PlaneCenter, PlaneRotation, 25.0, 1024, 27.0);
	bool bDiscValid = DiscPoints.Num() == 1024;
	for (const FVector& Point : DiscPoints)
	{
		const FVector Offset = Point - PlaneCenter;
		bDiscValid &= Offset.Size() < 25.0;
		bDiscValid &= FMath::IsNearlyZero(FVector::DotProduct(Offset, PlaneNormal), 1.e-8);
	}
	TestTrue(TEXT("Disc points are deterministic and remain inside the rotated disc"),
		bDiscValid && PointsEqual(DiscPoints, RepeatedDiscPoints));
	const TArray<FVector> SingleDiscPoint = UDirectiveUtilMathFunctionLibrary::GeneratePointsOnDisc(
		PlaneCenter, PlaneRotation, 25.0, 1, 27.0);
	TestTrue(TEXT("A single disc point is its center"),
		SingleDiscPoint.Num() == 1 && SingleDiscPoint[0] == PlaneCenter);

	const TArray<FVector> SpherePoints = UDirectiveUtilMathFunctionLibrary::GeneratePointsOnSphere(
		FVector::ZeroVector, PlaneRotation, 25.0, 1024, 27.0);
	const TArray<FVector> RepeatedSpherePoints = UDirectiveUtilMathFunctionLibrary::GeneratePointsOnSphere(
		FVector::ZeroVector, PlaneRotation, 25.0, 1024, 27.0);
	FVector SphereMean = FVector::ZeroVector;
	bool bSphereValid = SpherePoints.Num() == 1024;
	for (const FVector& Point : SpherePoints)
	{
		bSphereValid &= FMath::IsNearlyEqual(Point.Size(), 25.0, 1.e-8);
		SphereMean += Point;
	}
	if (!SpherePoints.IsEmpty())
	{
		SphereMean /= SpherePoints.Num();
	}
	TestTrue(TEXT("Sphere points are deterministic and remain on the surface"),
		bSphereValid && SphereMean.Size() < 0.01 && PointsEqual(SpherePoints, RepeatedSpherePoints));
	const TArray<FVector> SingleSpherePoint = UDirectiveUtilMathFunctionLibrary::GeneratePointsOnSphere(
		PlaneCenter, PlaneRotation, 25.0, 1, 27.0);
	TestTrue(TEXT("A single sphere point follows the rotated local Z axis"),
		SingleSpherePoint.Num() == 1
		&& SingleSpherePoint[0].Equals(PlaneCenter + PlaneNormal * 25.0, 1.e-8));

	const double Infinity = std::numeric_limits<double>::infinity();
	const FRotator InvalidRotation(Infinity, 0.0, 0.0);
	TestTrue(TEXT("Point generators reject non-finite values"),
		UDirectiveUtilMathFunctionLibrary::GenerateGridPoints2D(
			FVector::ZeroVector, FRotator::ZeroRotator, FIntPoint(2, 2), FVector2D(Infinity, 1.0), true).IsEmpty()
		&& UDirectiveUtilMathFunctionLibrary::GeneratePointsAlongDirection(
			FVector::ZeroVector, FVector::ForwardVector, 2, Infinity, false).IsEmpty()
		&& UDirectiveUtilMathFunctionLibrary::GeneratePointsOnCircle(
			FVector::ZeroVector, InvalidRotation, 1.0, 4, 0.0).IsEmpty()
		&& UDirectiveUtilMathFunctionLibrary::GeneratePointsOnArc(
			FVector::ZeroVector, FRotator::ZeroRotator, 1.0, 4, 0.0, Infinity, true).IsEmpty()
		&& UDirectiveUtilMathFunctionLibrary::GeneratePointsOnDisc(
			FVector(Infinity, 0.0, 0.0), FRotator::ZeroRotator, 1.0, 4, 0.0).IsEmpty()
		&& UDirectiveUtilMathFunctionLibrary::GeneratePointsOnSphere(
			FVector::ZeroVector, FRotator::ZeroRotator, Infinity, 4, 0.0).IsEmpty());
	TestTrue(TEXT("Point generators return empty arrays for non-positive counts"),
		UDirectiveUtilMathFunctionLibrary::GeneratePointsAlongDirection(
			FVector::ZeroVector, FVector::ForwardVector, 0, 1.0, false).IsEmpty()
		&& UDirectiveUtilMathFunctionLibrary::GeneratePointsBetweenLocations(
			FVector::ZeroVector, FVector::OneVector, -1, true).IsEmpty()
		&& UDirectiveUtilMathFunctionLibrary::GeneratePointsOnCircle(
			FVector::ZeroVector, FRotator::ZeroRotator, 1.0, 0, 0.0).IsEmpty());

	return !HasAnyErrors();
}
