// Copyright (c) 2026 Unreal Directive. Licensed under the MIT License.

#include "Libraries/DirectiveUtilMathFunctionLibrary.h"

#include "Components/SplineComponent.h"
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

	bool PointsEqualAfterTranslation(const TArray<FVector>& BasePoints, const TArray<FVector>& TranslatedPoints,
		const FVector& Translation, const double Tolerance = 1.e-6)
	{
		if (BasePoints.Num() != TranslatedPoints.Num())
		{
			return false;
		}
		for (int32 Index = 0; Index < BasePoints.Num(); ++Index)
		{
			if (!(TranslatedPoints[Index] - Translation).Equals(BasePoints[Index], Tolerance))
			{
				return false;
			}
		}
		return true;
	}

	bool PointsMatchLocation(const TArray<FVector>& Points, const FVector& Location, const double Tolerance = 1.e-9)
	{
		for (const FVector& Point : Points)
		{
			if (!Point.Equals(Location, Tolerance))
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

	const double HexRadius = 10.0;
	const FVector PointyQ = UDirectiveUtilMathFunctionLibrary::HexCoordinateToLocation(
		FIntPoint(1, 0), FVector::ZeroVector, FRotator::ZeroRotator, HexRadius,
		EDirectiveUtilHexOrientation::PointyTop);
	const FVector PointyR = UDirectiveUtilMathFunctionLibrary::HexCoordinateToLocation(
		FIntPoint(0, 1), FVector::ZeroVector, FRotator::ZeroRotator, HexRadius,
		EDirectiveUtilHexOrientation::PointyTop);
	TestTrue(TEXT("Pointy-top axial coordinates use the expected basis"),
		PointyQ.Equals(FVector(UE_DOUBLE_SQRT_3 * HexRadius, 0.0, 0.0), 1.e-9)
		&& PointyR.Equals(FVector(UE_DOUBLE_SQRT_3 * 0.5 * HexRadius, 15.0, 0.0), 1.e-9));

	const FVector FlatQ = UDirectiveUtilMathFunctionLibrary::HexCoordinateToLocation(
		FIntPoint(1, 0), FVector::ZeroVector, FRotator::ZeroRotator, HexRadius,
		EDirectiveUtilHexOrientation::FlatTop);
	const FVector FlatR = UDirectiveUtilMathFunctionLibrary::HexCoordinateToLocation(
		FIntPoint(0, 1), FVector::ZeroVector, FRotator::ZeroRotator, HexRadius,
		EDirectiveUtilHexOrientation::FlatTop);
	TestTrue(TEXT("Flat-top axial coordinates use the expected basis"),
		FlatQ.Equals(FVector(15.0, UE_DOUBLE_SQRT_3 * 0.5 * HexRadius, 0.0), 1.e-9)
		&& FlatR.Equals(FVector(0.0, UE_DOUBLE_SQRT_3 * HexRadius, 0.0), 1.e-9));

	const FVector HexOrigin(11.0, 13.0, 17.0);
	const FRotator HexRotation(23.0, 37.0, 11.0);
	const FVector HexPlaneNormal = HexRotation.Quaternion().GetAxisZ();
	for (const EDirectiveUtilHexOrientation HexOrientation : {
		EDirectiveUtilHexOrientation::PointyTop, EDirectiveUtilHexOrientation::FlatTop })
	{
		const FIntPoint Coordinate(-7, 4);
		const FVector Location = UDirectiveUtilMathFunctionLibrary::HexCoordinateToLocation(
			Coordinate, HexOrigin, HexRotation, 25.0, HexOrientation, 3.0);
		TestEqual(TEXT("Hex coordinate conversion round trips through a rotated layout"),
			UDirectiveUtilMathFunctionLibrary::LocationToHexCoordinate(
				Location, HexOrigin, HexRotation, 25.0, HexOrientation, 3.0), Coordinate);
		TestEqual(TEXT("Location conversion projects onto the hex plane"),
			UDirectiveUtilMathFunctionLibrary::LocationToHexCoordinate(
				Location + HexPlaneNormal * 500.0, HexOrigin, HexRotation, 25.0, HexOrientation, 3.0), Coordinate);
	}

	const FVector GappedHex = UDirectiveUtilMathFunctionLibrary::HexCoordinateToLocation(
		FIntPoint(1, 0), FVector::ZeroVector, FRotator::ZeroRotator, HexRadius,
		EDirectiveUtilHexOrientation::PointyTop, 2.0);
	TestTrue(TEXT("Hex gap adds to the adjacent edge distance"),
		FMath::IsNearlyEqual(GappedHex.Size(), UE_DOUBLE_SQRT_3 * HexRadius + 2.0, 1.e-9));

	const TArray<FIntPoint> HexNeighbors = UDirectiveUtilMathFunctionLibrary::GetHexNeighbors(FIntPoint(3, -2));
	const TArray<FIntPoint> ExpectedHexNeighbors = {
		FIntPoint(4, -2), FIntPoint(4, -3), FIntPoint(3, -3),
		FIntPoint(2, -2), FIntPoint(2, -1), FIntPoint(3, -1)
	};
	TestTrue(TEXT("Hex neighbors use stable axial direction order"), HexNeighbors == ExpectedHexNeighbors);
	TestEqual(TEXT("Hex distance counts axial grid steps"),
		UDirectiveUtilMathFunctionLibrary::GetHexDistance(FIntPoint(0, 0), FIntPoint(3, -5)), 5LL);
	TestEqual(TEXT("Hex distance uses 64-bit intermediates"),
		UDirectiveUtilMathFunctionLibrary::GetHexDistance(
			FIntPoint(MAX_int32, MAX_int32), FIntPoint(MIN_int32, MIN_int32)), 8589934590LL);

	const TArray<FVector> RectangularHexGrid = UDirectiveUtilMathFunctionLibrary::GenerateRectangularHexGrid(
		FVector::ZeroVector, FRotator::ZeroRotator, FIntPoint(3, 2), HexRadius,
		EDirectiveUtilHexOrientation::PointyTop, 0.0, true);
	TestTrue(TEXT("Rectangular hex grid is centered and ordered by row then column"),
		RectangularHexGrid.Num() == 6
		&& RectangularHexGrid[0].Equals(-RectangularHexGrid.Last(), 1.e-9)
		&& RectangularHexGrid[1].X < RectangularHexGrid[2].X
		&& RectangularHexGrid[2].Y < RectangularHexGrid[3].Y);
	const TArray<FVector> UncenteredHexGrid = UDirectiveUtilMathFunctionLibrary::GenerateRectangularHexGrid(
		HexOrigin, FRotator::ZeroRotator, FIntPoint(2, 2), HexRadius,
		EDirectiveUtilHexOrientation::FlatTop, 0.0, false);
	TestTrue(TEXT("Uncentered rectangular hex grid starts at its origin"),
		UncenteredHexGrid.Num() == 4 && UncenteredHexGrid[0] == HexOrigin);

	const TArray<FVector> HexagonalGrid = UDirectiveUtilMathFunctionLibrary::GenerateHexagonalHexGrid(
		FVector::ZeroVector, FRotator::ZeroRotator, 2, HexRadius,
		EDirectiveUtilHexOrientation::PointyTop);
	TSet<FIntPoint> HexagonalCoordinates;
	bool bHexagonalGridValid = HexagonalGrid.Num() == 19;
	for (const FVector& Point : HexagonalGrid)
	{
		const FIntPoint Coordinate = UDirectiveUtilMathFunctionLibrary::LocationToHexCoordinate(
			Point, FVector::ZeroVector, FRotator::ZeroRotator, HexRadius,
			EDirectiveUtilHexOrientation::PointyTop);
		bHexagonalGridValid &= UDirectiveUtilMathFunctionLibrary::GetHexDistance(FIntPoint::ZeroValue, Coordinate) <= 2;
		HexagonalCoordinates.Add(Coordinate);
	}
	TestTrue(TEXT("Hexagonal grid contains every coordinate through its requested radius"),
		bHexagonalGridValid && HexagonalCoordinates.Num() == 19);
	TestEqual(TEXT("A zero-radius hexagonal grid contains its center"),
		UDirectiveUtilMathFunctionLibrary::GenerateHexagonalHexGrid(
			HexOrigin, FRotator::ZeroRotator, 0, HexRadius).Num(), 1);
	TestTrue(TEXT("Hex generators reject invalid layouts and counts"),
		UDirectiveUtilMathFunctionLibrary::GenerateRectangularHexGrid(
			FVector::ZeroVector, FRotator::ZeroRotator, FIntPoint(0, 2), HexRadius).IsEmpty()
		&& UDirectiveUtilMathFunctionLibrary::GenerateRectangularHexGrid(
			FVector::ZeroVector, FRotator::ZeroRotator, FIntPoint(2, 2), 0.0).IsEmpty()
		&& UDirectiveUtilMathFunctionLibrary::GenerateHexagonalHexGrid(
			FVector::ZeroVector, FRotator::ZeroRotator, -1, HexRadius).IsEmpty()
		&& UDirectiveUtilMathFunctionLibrary::GenerateHexagonalHexGrid(
			FVector::ZeroVector, FRotator::ZeroRotator, 30000, HexRadius).IsEmpty()
		&& UDirectiveUtilMathFunctionLibrary::HexCoordinateToLocation(
			FIntPoint(1, 0), FVector::ZeroVector, FRotator::ZeroRotator, HexRadius,
			EDirectiveUtilHexOrientation::PointyTop, -UE_DOUBLE_SQRT_3 * HexRadius).IsZero()
		&& UDirectiveUtilMathFunctionLibrary::GetHexNeighbors(FIntPoint(MAX_int32, 0)).IsEmpty());

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

	USplineComponent* Spline = NewObject<USplineComponent>();
	Spline->SetSplinePoints({ FVector::ZeroVector, FVector(100.0, 0.0, 0.0) },
		ESplineCoordinateSpace::Local, false);
	Spline->SetSplinePointType(0, ESplinePointType::Linear, false);
	Spline->SetSplinePointType(1, ESplinePointType::Linear, true);
	const TArray<FVector> SplinePoints = UDirectiveUtilMathFunctionLibrary::GeneratePointsAlongSpline(
		Spline, 30.0, true);
	TestTrue(TEXT("Spline points use fixed spacing and append the exact open endpoint"),
		SplinePoints.Num() == 5
		&& SplinePoints[0].Equals(FVector::ZeroVector)
		&& SplinePoints[1].Equals(FVector(30.0, 0.0, 0.0), 1.e-4)
		&& SplinePoints[3].Equals(FVector(90.0, 0.0, 0.0), 1.e-4)
		&& SplinePoints[4].Equals(FVector(100.0, 0.0, 0.0), 1.e-4));
	TestEqual(TEXT("Spline endpoint can be excluded"),
		UDirectiveUtilMathFunctionLibrary::GeneratePointsAlongSpline(Spline, 30.0, false).Num(), 4);
	Spline->SetClosedLoop(true, true);
	const TArray<FVector> ClosedSplinePoints = UDirectiveUtilMathFunctionLibrary::GeneratePointsAlongSpline(
		Spline, 30.0, true);
	TestTrue(TEXT("Closed spline sampling does not repeat its first point"),
		ClosedSplinePoints.Num() > 1 && !ClosedSplinePoints[0].Equals(ClosedSplinePoints.Last(), 1.e-4));
	USplineComponent* SinglePointSpline = NewObject<USplineComponent>();
	SinglePointSpline->SetSplinePoints({ FVector(3.0, 4.0, 5.0) }, ESplineCoordinateSpace::Local, true);
	const TArray<FVector> ZeroLengthSplinePoints = UDirectiveUtilMathFunctionLibrary::GeneratePointsAlongSpline(
		SinglePointSpline, 10.0, true);
	TestTrue(TEXT("A zero-length spline returns its only point"),
		ZeroLengthSplinePoints.Num() == 1 && ZeroLengthSplinePoints[0].Equals(FVector(3.0, 4.0, 5.0)));
	TestTrue(TEXT("Spline sampling rejects invalid input"),
		UDirectiveUtilMathFunctionLibrary::GeneratePointsAlongSpline(nullptr, 10.0, true).IsEmpty()
		&& UDirectiveUtilMathFunctionLibrary::GeneratePointsAlongSpline(Spline, 0.0, true).IsEmpty());

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

	const FVector LargeTranslation(1000000.0, -2000000.0, 3000000.0);
	const FRotator AuditRotation(-37.0, 123.0, 71.0);
	TestTrue(TEXT("A translated center offsets every generated arc point exactly once"),
		PointsEqualAfterTranslation(
			UDirectiveUtilMathFunctionLibrary::GeneratePointsOnArc(
				FVector::ZeroVector, AuditRotation, -13.5, 11, 1080.25, -450.5, true),
			UDirectiveUtilMathFunctionLibrary::GeneratePointsOnArc(
				LargeTranslation, AuditRotation, -13.5, 11, 1080.25, -450.5, true),
			LargeTranslation));

	TestTrue(TEXT("Translated origins preserve every spatial generator's local offsets"),
		PointsEqualAfterTranslation(
			UDirectiveUtilMathFunctionLibrary::GenerateGridPoints2D(
				FVector::ZeroVector, AuditRotation, FIntPoint(4, 3), FVector2D(-7.0, 11.0), true),
			UDirectiveUtilMathFunctionLibrary::GenerateGridPoints2D(
				LargeTranslation, AuditRotation, FIntPoint(4, 3), FVector2D(-7.0, 11.0), true),
			LargeTranslation)
		&& PointsEqualAfterTranslation(
			UDirectiveUtilMathFunctionLibrary::GenerateGridPoints3D(
				FVector::ZeroVector, AuditRotation, FIntVector(3, 2, 2), FVector(5.0, -7.0, 0.0), false),
			UDirectiveUtilMathFunctionLibrary::GenerateGridPoints3D(
				LargeTranslation, AuditRotation, FIntVector(3, 2, 2), FVector(5.0, -7.0, 0.0), false),
			LargeTranslation)
		&& PointsEqualAfterTranslation(
			UDirectiveUtilMathFunctionLibrary::GeneratePointsAlongDirection(
				FVector::ZeroVector, FVector(-2.0, 3.0, -5.0), 7, -3.25, true),
			UDirectiveUtilMathFunctionLibrary::GeneratePointsAlongDirection(
				LargeTranslation, FVector(-2.0, 3.0, -5.0), 7, -3.25, true),
			LargeTranslation)
		&& PointsEqualAfterTranslation(
			UDirectiveUtilMathFunctionLibrary::GeneratePointsBetweenLocations(
				FVector(-11.0, 5.0, 8.0), FVector(17.0, -9.0, 3.0), 8, false),
			UDirectiveUtilMathFunctionLibrary::GeneratePointsBetweenLocations(
				LargeTranslation + FVector(-11.0, 5.0, 8.0),
				LargeTranslation + FVector(17.0, -9.0, 3.0), 8, false),
			LargeTranslation)
		&& PointsEqualAfterTranslation(
			UDirectiveUtilMathFunctionLibrary::GeneratePointsOnCircle(
				FVector::ZeroVector, AuditRotation, -13.5, 17, -725.25),
			UDirectiveUtilMathFunctionLibrary::GeneratePointsOnCircle(
				LargeTranslation, AuditRotation, -13.5, 17, -725.25),
			LargeTranslation)
		&& PointsEqualAfterTranslation(
			UDirectiveUtilMathFunctionLibrary::GeneratePointsOnDisc(
				FVector::ZeroVector, AuditRotation, -13.5, 257, 1080.25),
			UDirectiveUtilMathFunctionLibrary::GeneratePointsOnDisc(
				LargeTranslation, AuditRotation, -13.5, 257, 1080.25),
			LargeTranslation)
		&& PointsEqualAfterTranslation(
			UDirectiveUtilMathFunctionLibrary::GeneratePointsOnSphere(
				FVector::ZeroVector, AuditRotation, -13.5, 257, -1080.25),
			UDirectiveUtilMathFunctionLibrary::GeneratePointsOnSphere(
				LargeTranslation, AuditRotation, -13.5, 257, -1080.25),
			LargeTranslation));

	for (const EDirectiveUtilHexOrientation Orientation : {
		EDirectiveUtilHexOrientation::PointyTop, EDirectiveUtilHexOrientation::FlatTop })
	{
		TestTrue(TEXT("Translated origins preserve rectangular and hexagonal grid offsets"),
			PointsEqualAfterTranslation(
				UDirectiveUtilMathFunctionLibrary::GenerateRectangularHexGrid(
					FVector::ZeroVector, AuditRotation, FIntPoint(4, 3), 9.5, Orientation, -1.25, true),
				UDirectiveUtilMathFunctionLibrary::GenerateRectangularHexGrid(
					LargeTranslation, AuditRotation, FIntPoint(4, 3), 9.5, Orientation, -1.25, true),
				LargeTranslation)
			&& PointsEqualAfterTranslation(
				UDirectiveUtilMathFunctionLibrary::GenerateHexagonalHexGrid(
					FVector::ZeroVector, AuditRotation, 4, 9.5, Orientation, -1.25),
				UDirectiveUtilMathFunctionLibrary::GenerateHexagonalHexGrid(
					LargeTranslation, AuditRotation, 4, 9.5, Orientation, -1.25),
				LargeTranslation));

		for (const FIntPoint Coordinate : {
			FIntPoint::ZeroValue, FIntPoint(-17, 29), FIntPoint(1234, -987), FIntPoint(-4096, -2048) })
		{
			const FVector Location = UDirectiveUtilMathFunctionLibrary::HexCoordinateToLocation(
				Coordinate, LargeTranslation, AuditRotation, 9.5, Orientation, -1.25);
			TestEqual(TEXT("Odd signed hex coordinates round trip through translated rotated layouts"),
				UDirectiveUtilMathFunctionLibrary::LocationToHexCoordinate(
					Location, LargeTranslation, AuditRotation, 9.5, Orientation, -1.25), Coordinate);
		}
	}

	const TArray<FVector> ZeroRadiusCircle = UDirectiveUtilMathFunctionLibrary::GeneratePointsOnCircle(
		LargeTranslation, AuditRotation, 0.0, 9, 123.0);
	const TArray<FVector> ZeroRadiusArc = UDirectiveUtilMathFunctionLibrary::GeneratePointsOnArc(
		LargeTranslation, AuditRotation, 0.0, 9, -30.0, -720.0, false);
	const TArray<FVector> ZeroRadiusDisc = UDirectiveUtilMathFunctionLibrary::GeneratePointsOnDisc(
		LargeTranslation, AuditRotation, 0.0, 9, 123.0);
	const TArray<FVector> ZeroRadiusSphere = UDirectiveUtilMathFunctionLibrary::GeneratePointsOnSphere(
		LargeTranslation, AuditRotation, 0.0, 9, 123.0);
	TestTrue(TEXT("Zero-radius generators preserve their requested count at the center"),
		ZeroRadiusCircle.Num() == 9 && PointsMatchLocation(ZeroRadiusCircle, LargeTranslation)
		&& ZeroRadiusArc.Num() == 9 && PointsMatchLocation(ZeroRadiusArc, LargeTranslation)
		&& ZeroRadiusDisc.Num() == 9 && PointsMatchLocation(ZeroRadiusDisc, LargeTranslation)
		&& ZeroRadiusSphere.Num() == 9 && PointsMatchLocation(ZeroRadiusSphere, LargeTranslation));

	TestTrue(TEXT("Degenerate linear generators preserve their requested count and location"),
		PointsMatchLocation(UDirectiveUtilMathFunctionLibrary::GenerateGridPoints3D(
			LargeTranslation, AuditRotation, FIntVector(2, 3, 4), FVector::ZeroVector, true), LargeTranslation)
		&& PointsMatchLocation(UDirectiveUtilMathFunctionLibrary::GeneratePointsAlongDirection(
			LargeTranslation, FVector(2.0, -3.0, 4.0), 7, 0.0, true), LargeTranslation)
		&& PointsMatchLocation(UDirectiveUtilMathFunctionLibrary::GeneratePointsBetweenLocations(
			LargeTranslation, LargeTranslation, 7, false), LargeTranslation));

	USplineComponent* TransformedSpline = NewObject<USplineComponent>();
	TransformedSpline->SetWorldLocation(LargeTranslation);
	TransformedSpline->SetWorldRotation(AuditRotation);
	TransformedSpline->SetWorldScale3D(FVector(2.0, 3.0, 0.5));
	TransformedSpline->SetSplinePoints({ FVector::ZeroVector, FVector(100.0, 0.0, 0.0) },
		ESplineCoordinateSpace::Local, false);
	TransformedSpline->SetSplinePointType(0, ESplinePointType::Linear, false);
	TransformedSpline->SetSplinePointType(1, ESplinePointType::Linear, true);
	const TArray<FVector> TransformedSplinePoints = UDirectiveUtilMathFunctionLibrary::GeneratePointsAlongSpline(
		TransformedSpline, 60.0, true);
	TestEqual(TEXT("Scaled spline sampling produces the expected point count"), TransformedSplinePoints.Num(), 5);
	if (TransformedSplinePoints.Num() == 5)
	{
		TestTrue(TEXT("Spline sampling returns its world-space start"),
			TransformedSplinePoints[0].Equals(LargeTranslation, 1.e-4));
		TestTrue(TEXT("Spline sampling preserves regular world-space intervals"),
			FMath::IsNearlyEqual(FVector::Distance(TransformedSplinePoints[0], TransformedSplinePoints[1]), 60.0, 1.e-4)
			&& FMath::IsNearlyEqual(FVector::Distance(TransformedSplinePoints[1], TransformedSplinePoints[2]), 60.0, 1.e-4)
			&& FMath::IsNearlyEqual(FVector::Distance(TransformedSplinePoints[2], TransformedSplinePoints[3]), 60.0, 1.e-4));
		TestTrue(TEXT("Spline sampling appends its exact world-space endpoint"),
			TransformedSplinePoints.Last().Equals(TransformedSpline->GetLocationAtDistanceAlongSpline(
				TransformedSpline->GetSplineLength(), ESplineCoordinateSpace::World), 1.e-4));
	}

	const double Infinity = std::numeric_limits<double>::infinity();
	const double NaN = std::numeric_limits<double>::quiet_NaN();
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
			FVector::ZeroVector, FRotator::ZeroRotator, Infinity, 4, 0.0).IsEmpty()
		&& UDirectiveUtilMathFunctionLibrary::GenerateRectangularHexGrid(
			FVector::ZeroVector, FRotator::ZeroRotator, FIntPoint(2, 2), Infinity).IsEmpty()
		&& UDirectiveUtilMathFunctionLibrary::GeneratePointsAlongSpline(Spline, Infinity, true).IsEmpty());
	TestTrue(TEXT("Every spatial generator rejects a non-finite origin or center"),
		UDirectiveUtilMathFunctionLibrary::GenerateGridPoints3D(
			FVector(NaN, 0.0, 0.0), FRotator::ZeroRotator, FIntVector(1), FVector::OneVector, true).IsEmpty()
		&& UDirectiveUtilMathFunctionLibrary::GeneratePointsBetweenLocations(
			FVector::ZeroVector, FVector(Infinity, 0.0, 0.0), 2, true).IsEmpty()
		&& UDirectiveUtilMathFunctionLibrary::GeneratePointsOnArc(
			FVector(NaN, 0.0, 0.0), FRotator::ZeroRotator, 1.0, 2, 0.0, 90.0, true).IsEmpty()
		&& UDirectiveUtilMathFunctionLibrary::GenerateHexagonalHexGrid(
			FVector(Infinity, 0.0, 0.0), FRotator::ZeroRotator, 1, 1.0).IsEmpty());
	TestTrue(TEXT("Spline sampling rejects negative spacing"),
		UDirectiveUtilMathFunctionLibrary::GeneratePointsAlongSpline(Spline, -1.0, true).IsEmpty());
	TestTrue(TEXT("Point generators return empty arrays for non-positive counts"),
		UDirectiveUtilMathFunctionLibrary::GeneratePointsAlongDirection(
			FVector::ZeroVector, FVector::ForwardVector, 0, 1.0, false).IsEmpty()
		&& UDirectiveUtilMathFunctionLibrary::GeneratePointsBetweenLocations(
			FVector::ZeroVector, FVector::OneVector, -1, true).IsEmpty()
		&& UDirectiveUtilMathFunctionLibrary::GeneratePointsOnCircle(
			FVector::ZeroVector, FRotator::ZeroRotator, 1.0, 0, 0.0).IsEmpty());

	return !HasAnyErrors();
}
