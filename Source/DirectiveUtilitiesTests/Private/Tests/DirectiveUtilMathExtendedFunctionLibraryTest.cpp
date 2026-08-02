// Copyright (c) 2026 Unreal Directive. Licensed under the MIT License.

#include "Libraries/DirectiveUtilMathFunctionLibrary.h"
#include "Misc/AutomationTest.h"

#include <limits>

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDirectiveUtilMathExtendedFunctionLibraryTest,
	"DirectiveUtilities.Math.ExtendedFunctionLibrary",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::EngineFilter)

bool FDirectiveUtilMathExtendedFunctionLibraryTest::RunTest(const FString& Parameters)
{
	float FloatResult = 0.0f;
	float Strength = 0.0f;
	TestTrue("Angle array average accepts values across the degree seam",
		UDirectiveUtilMathFunctionLibrary::GetAngleArrayAverage({350.0f, 10.0f}, FloatResult, Strength));
	TestTrue("Angle array average crosses the degree seam",
		FMath::IsNearlyZero(FloatResult, 1.e-4f));
	TestTrue("Angle array average reports concentration",
		FMath::IsNearlyEqual(Strength, FMath::Cos(FMath::DegreesToRadians(10.0f)), 1.e-4f));
	TestFalse("Angle array average rejects an undefined antipodal mean",
		UDirectiveUtilMathFunctionLibrary::GetAngleArrayAverage({0.0f, 180.0f}, FloatResult, Strength));
	TestFalse("Angle array average rejects non-finite values",
		UDirectiveUtilMathFunctionLibrary::GetAngleArrayAverage(
			{0.0f, std::numeric_limits<float>::quiet_NaN()}, FloatResult, Strength));

	TestTrue("Weighted float average accepts aligned arrays",
		UDirectiveUtilMathFunctionLibrary::GetWeightedFloatArrayAverage(
			{10.0f, 20.0f}, {1.0f, 3.0f}, FloatResult));
	TestTrue("Weighted float average applies weights",
		FMath::IsNearlyEqual(FloatResult, 17.5f, 1.e-4f));
	TestTrue("Weighted float average ignores negative weights",
		UDirectiveUtilMathFunctionLibrary::GetWeightedFloatArrayAverage(
			{10.0f, 20.0f}, {-1.0f, 2.0f}, FloatResult)
		&& FMath::IsNearlyEqual(FloatResult, 20.0f, 1.e-4f));
	TestFalse("Weighted float average rejects mismatched arrays",
		UDirectiveUtilMathFunctionLibrary::GetWeightedFloatArrayAverage(
			{10.0f}, {1.0f, 2.0f}, FloatResult));

	FVector VectorResult = FVector::ZeroVector;
	TestTrue("Weighted vector average accepts aligned arrays",
		UDirectiveUtilMathFunctionLibrary::GetWeightedVectorArrayAverage(
			{FVector::ForwardVector, FVector::RightVector}, {1.0f, 1.0f}, VectorResult));
	TestTrue("Weighted vector average applies weights",
		VectorResult.Equals(FVector(0.5, 0.5, 0.0), 1.e-6));
	TestTrue("Weighted vector average preserves large finite values",
		UDirectiveUtilMathFunctionLibrary::GetWeightedVectorArrayAverage(
			{FVector(1.e300, 0.0, 0.0)}, {1.e20f}, VectorResult)
		&& FMath::IsNearlyEqual(VectorResult.X / 1.e300, 1.0, 1.e-12));
	TestFalse("Weighted vector average rejects non-finite values",
		UDirectiveUtilMathFunctionLibrary::GetWeightedVectorArrayAverage(
			{FVector::ForwardVector, FVector(std::numeric_limits<double>::infinity(), 0.0, 0.0)},
			{1.0f, 1.0f}, VectorResult));

	TArray<float> FloatArrayResult;
	TestTrue("Float array normalization accepts finite values",
		UDirectiveUtilMathFunctionLibrary::NormalizeFloatArrayToRange(
			{2.0f, 4.0f, 6.0f}, -1.0f, 1.0f, FloatArrayResult));
	TestTrue("Float array normalization maps the full range",
		FloatArrayResult.Num() == 3
		&& FMath::IsNearlyEqual(FloatArrayResult[0], -1.0f)
		&& FMath::IsNearlyZero(FloatArrayResult[1])
		&& FMath::IsNearlyEqual(FloatArrayResult[2], 1.0f));
	TestTrue("Float array normalization accepts reversed output bounds",
		UDirectiveUtilMathFunctionLibrary::NormalizeFloatArrayToRange(
			{2.0f, 4.0f, 6.0f}, 1.0f, -1.0f, FloatArrayResult)
		&& FMath::IsNearlyEqual(FloatArrayResult[0], 1.0f)
		&& FMath::IsNearlyEqual(FloatArrayResult[2], -1.0f));
	TestTrue("Float array normalization maps a constant array to the output minimum",
		UDirectiveUtilMathFunctionLibrary::NormalizeFloatArrayToRange(
			{4.0f, 4.0f}, 5.0f, 10.0f, FloatArrayResult)
		&& FloatArrayResult == TArray<float>({5.0f, 5.0f}));
	TestFalse("Float array normalization rejects an empty array",
		UDirectiveUtilMathFunctionLibrary::NormalizeFloatArrayToRange(
			{}, 0.0f, 1.0f, FloatArrayResult));
	TArray<float> InPlaceValues = {2.0f, 4.0f, 6.0f};
	TestTrue("Float array normalization supports the same input and output array",
		UDirectiveUtilMathFunctionLibrary::NormalizeFloatArrayToRange(
			InPlaceValues, -1.0f, 1.0f, InPlaceValues)
		&& InPlaceValues == TArray<float>({-1.0f, 0.0f, 1.0f}));

	TestTrue("Weight normalization accepts positive weights",
		UDirectiveUtilMathFunctionLibrary::NormalizeWeights({1.0f, 3.0f}, FloatArrayResult));
	TestTrue("Weight normalization sums to one",
		FloatArrayResult.Num() == 2
		&& FMath::IsNearlyEqual(FloatArrayResult[0], 0.25f)
		&& FMath::IsNearlyEqual(FloatArrayResult[1], 0.75f));
	TestTrue("Weight normalization clears negative weights",
		UDirectiveUtilMathFunctionLibrary::NormalizeWeights({-2.0f, 2.0f}, FloatArrayResult)
		&& FMath::IsNearlyZero(FloatArrayResult[0])
		&& FMath::IsNearlyEqual(FloatArrayResult[1], 1.0f));
	TestFalse("Weight normalization rejects all-zero weights",
		UDirectiveUtilMathFunctionLibrary::NormalizeWeights({0.0f, -1.0f}, FloatArrayResult));
	TArray<float> InPlaceWeights = {1.0f, 3.0f};
	TestTrue("Weight normalization supports the same input and output array",
		UDirectiveUtilMathFunctionLibrary::NormalizeWeights(InPlaceWeights, InPlaceWeights)
		&& InPlaceWeights == TArray<float>({0.25f, 0.75f}));

	TestTrue("Float array percentile accepts finite values",
		UDirectiveUtilMathFunctionLibrary::GetFloatArrayPercentile(
			{4.0f, 1.0f, 3.0f, 2.0f}, 25.0f, FloatResult));
	TestTrue("Float array percentile interpolates adjacent values",
		FMath::IsNearlyEqual(FloatResult, 1.75f, 1.e-4f));
	TestTrue("Float array percentile clamps above one hundred",
		UDirectiveUtilMathFunctionLibrary::GetFloatArrayPercentile(
			{1.0f, 4.0f}, 125.0f, FloatResult)
		&& FMath::IsNearlyEqual(FloatResult, 4.0f));
	TestTrue("Float array percentile uses the Type 7 sample position",
		UDirectiveUtilMathFunctionLibrary::GetFloatArrayPercentile(
			{10.0f, 1.0f, 8.0f, 2.0f, 7.0f, 3.0f, 6.0f, 4.0f, 9.0f, 5.0f}, 40.0f, FloatResult)
		&& FMath::IsNearlyEqual(FloatResult, 4.6f, 1.e-4f));
	TestFalse("Float array percentile rejects non-finite values",
		UDirectiveUtilMathFunctionLibrary::GetFloatArrayPercentile(
			{1.0f, std::numeric_limits<float>::infinity()}, 50.0f, FloatResult));

	TestTrue("Root mean square accepts finite values",
		UDirectiveUtilMathFunctionLibrary::GetFloatArrayRootMeanSquare({3.0f, 4.0f}, FloatResult));
	TestTrue("Root mean square uses the arithmetic mean of squares",
		FMath::IsNearlyEqual(FloatResult, FMath::Sqrt(12.5f), 1.e-4f));
	TestFalse("Root mean square rejects an empty array",
		UDirectiveUtilMathFunctionLibrary::GetFloatArrayRootMeanSquare({}, FloatResult));

	TestTrue("Smooth Step clamps below its range",
		FMath::IsNearlyZero(UDirectiveUtilMathFunctionLibrary::SmoothStep(-1.0f, 0.0f, 1.0f)));
	TestTrue("Smooth Step reaches its midpoint",
		FMath::IsNearlyEqual(UDirectiveUtilMathFunctionLibrary::SmoothStep(0.5f, 0.0f, 1.0f), 0.5f));
	TestTrue("Smooth Step accepts reversed bounds",
		FMath::IsNearlyEqual(UDirectiveUtilMathFunctionLibrary::SmoothStep(0.25f, 1.0f, 0.0f), 0.15625f));
	TestTrue("Smooth Step treats equal bounds as a step",
		FMath::IsNearlyEqual(UDirectiveUtilMathFunctionLibrary::SmoothStep(2.0f, 2.0f, 2.0f), 1.0f));
	TestTrue("Smoother Step reaches its midpoint",
		FMath::IsNearlyEqual(UDirectiveUtilMathFunctionLibrary::SmootherStep(0.5f, 0.0f, 1.0f), 0.5f));
	TestTrue("Smoother Step has quintic shaping",
		FMath::IsNearlyEqual(UDirectiveUtilMathFunctionLibrary::SmootherStep(0.25f, 0.0f, 1.0f), 0.103515625f));

	TestTrue("Range Falloff applies linear attenuation",
		FMath::IsNearlyEqual(UDirectiveUtilMathFunctionLibrary::RangeFalloff(5.0f, 0.0f, 10.0f), 0.5f));
	TestTrue("Range Falloff applies its exponent",
		FMath::IsNearlyEqual(UDirectiveUtilMathFunctionLibrary::RangeFalloff(5.0f, 0.0f, 10.0f, 2.0f), 0.25f));
	TestTrue("Range Falloff remains one inside the inner radius",
		FMath::IsNearlyEqual(UDirectiveUtilMathFunctionLibrary::RangeFalloff(2.0f, 3.0f, 10.0f), 1.0f));
	TestTrue("Range Falloff reaches zero at the outer radius",
		FMath::IsNearlyZero(UDirectiveUtilMathFunctionLibrary::RangeFalloff(10.0f, 3.0f, 10.0f)));

	double Distance = 0.0;
	TestTrue("Direction and distance accepts distinct points",
		UDirectiveUtilMathFunctionLibrary::GetDirectionAndDistance(
			FVector::ZeroVector, FVector(3.0, 4.0, 0.0), VectorResult, Distance));
	TestTrue("Direction and distance returns a unit direction",
		VectorResult.Equals(FVector(0.6, 0.8, 0.0), 1.e-6));
	TestTrue("Direction and distance returns the length",
		FMath::IsNearlyEqual(Distance, 5.0));
	TestFalse("Direction and distance rejects equal points",
		UDirectiveUtilMathFunctionLibrary::GetDirectionAndDistance(
			FVector::ZeroVector, FVector::ZeroVector, VectorResult, Distance));
	TestTrue("Direction and distance handles large finite coordinates",
		UDirectiveUtilMathFunctionLibrary::GetDirectionAndDistance(
			FVector::ZeroVector, FVector(1.e200, 0.0, 0.0), VectorResult, Distance)
		&& VectorResult.Equals(FVector::ForwardVector, 1.e-12)
		&& FMath::IsNearlyEqual(Distance / 1.e200, 1.0, 1.e-12));

	TestTrue("Signed angle handles large finite vectors",
		FMath::IsNearlyEqual(UDirectiveUtilMathFunctionLibrary::SignedAngleBetweenVectors(
			FVector(1.e200, 0.0, 0.0), FVector(0.0, 1.e200, 0.0), FVector::UpVector), 90.0f, 1.e-4f));
	TestTrue("Direction Within Cone handles large finite vectors",
		UDirectiveUtilMathFunctionLibrary::IsDirectionWithinCone(
			FVector(1.e200, 0.0, 0.0), FVector(1.e200, 0.0, 0.0), 0.0f));
	TestTrue("Direction Within Cone includes an identical non-axis direction at zero width",
		UDirectiveUtilMathFunctionLibrary::IsDirectionWithinCone(
			FVector(3.e200, 2.e200, 1.e200), FVector(3.e200, 2.e200, 1.e200), 0.0f));
	TestFalse("Direction Within Cone excludes a measurable angle from a zero-width cone",
		UDirectiveUtilMathFunctionLibrary::IsDirectionWithinCone(
			FVector(FMath::Cos(FMath::DegreesToRadians(0.005)), FMath::Sin(FMath::DegreesToRadians(0.005)), 0.0),
			FVector::ForwardVector, 0.0f));

	const FVector2D RotatedPoint = UDirectiveUtilMathFunctionLibrary::RotatePointAroundPivot2D(
		FVector2D(2.0, 1.0), FVector2D(1.0, 1.0), 90.0f);
	TestTrue("Rotate Point Around Pivot 2D preserves the pivot offset",
		RotatedPoint.Equals(FVector2D(1.0, 2.0), 1.e-6));
	TestTrue("Rotate Point Around Pivot 2D rejects non-finite input",
		UDirectiveUtilMathFunctionLibrary::RotatePointAroundPivot2D(
			FVector2D(std::numeric_limits<double>::infinity(), 0.0), FVector2D::ZeroVector, 90.0f).IsZero());

	TestTrue("Signed Distance To Plane is positive in front of the plane",
		FMath::IsNearlyEqual(UDirectiveUtilMathFunctionLibrary::SignedDistanceToPlane(
			FVector(0.0, 0.0, 5.0), FVector(0.0, 0.0, 2.0), FVector::UpVector), 3.0));
	TestTrue("Signed Distance To Plane follows the normal direction",
		FMath::IsNearlyEqual(UDirectiveUtilMathFunctionLibrary::SignedDistanceToPlane(
			FVector(0.0, 0.0, 5.0), FVector(0.0, 0.0, 2.0), -FVector::UpVector), -3.0));
	TestTrue("Signed Distance To Plane rejects a zero normal",
		FMath::IsNearlyZero(UDirectiveUtilMathFunctionLibrary::SignedDistanceToPlane(
			FVector::UpVector, FVector::ZeroVector, FVector::ZeroVector)));
	TestTrue("Signed Distance To Plane handles a large finite normal",
		FMath::IsNearlyEqual(UDirectiveUtilMathFunctionLibrary::SignedDistanceToPlane(
			FVector::ForwardVector, FVector::ZeroVector, FVector(1.e200, 0.0, 0.0)), 1.0, 1.e-12));

	TestTrue("Point Within Cone includes a point inside the cone",
		UDirectiveUtilMathFunctionLibrary::IsPointWithinCone(
			FVector(10.0, 0.0, 0.0), FVector::ZeroVector, FVector::ForwardVector, 10.0f, 20.0));
	TestFalse("Point Within Cone excludes a point outside the cone",
		UDirectiveUtilMathFunctionLibrary::IsPointWithinCone(
			FVector(0.0, 10.0, 0.0), FVector::ZeroVector, FVector::ForwardVector, 10.0f, 20.0));
	TestFalse("Point Within Cone applies the maximum distance",
		UDirectiveUtilMathFunctionLibrary::IsPointWithinCone(
			FVector(10.0, 0.0, 0.0), FVector::ZeroVector, FVector::ForwardVector, 10.0f, 5.0));
	TestTrue("Point Within Cone treats zero maximum distance as unlimited",
		UDirectiveUtilMathFunctionLibrary::IsPointWithinCone(
			FVector(10.0, 0.0, 0.0), FVector::ZeroVector, FVector::ForwardVector, 10.0f));
	TestFalse("Point Within Cone applies maximum distance to large finite coordinates",
		UDirectiveUtilMathFunctionLibrary::IsPointWithinCone(
			FVector(1.5e200, 0.0, 0.0), FVector::ZeroVector, FVector::ForwardVector, 180.0f, 1.e200));

	bool bCircleSamplesValid = true;
	bool bAnnulusSamplesValid = true;
	bool bSphereSamplesValid = true;
	for (int32 Index = 0; Index < 100; ++Index)
	{
		const FVector2D CirclePoint = UDirectiveUtilMathFunctionLibrary::RandomPointInCircle(5.0f);
		const FVector2D AnnulusPoint = UDirectiveUtilMathFunctionLibrary::RandomPointInAnnulus(2.0f, 5.0f);
		const FVector SpherePoint = UDirectiveUtilMathFunctionLibrary::RandomPointInSphere(5.0f);
		bCircleSamplesValid &= CirclePoint.Size() <= 5.0 + 1.e-6;
		bAnnulusSamplesValid &= AnnulusPoint.Size() >= 2.0 - 1.e-6 && AnnulusPoint.Size() <= 5.0 + 1.e-6;
		bSphereSamplesValid &= SpherePoint.Size() <= 5.0 + 1.e-6;
	}
	TestTrue("Random Point In Circle stays within its radius", bCircleSamplesValid);
	TestTrue("Random Point In Annulus stays between its radii", bAnnulusSamplesValid);
	TestTrue("Random Point In Sphere stays within its radius", bSphereSamplesValid);

	FRandomStream FirstStream(12345);
	FRandomStream SecondStream(12345);
	TestTrue("Random Point In Circle stream variant is deterministic",
		UDirectiveUtilMathFunctionLibrary::RandomPointInCircleFromStream(FirstStream, 5.0f).Equals(
			UDirectiveUtilMathFunctionLibrary::RandomPointInCircleFromStream(SecondStream, 5.0f), 1.e-9));
	TestTrue("Random Point In Annulus stream variant is deterministic",
		UDirectiveUtilMathFunctionLibrary::RandomPointInAnnulusFromStream(FirstStream, 2.0f, 5.0f).Equals(
			UDirectiveUtilMathFunctionLibrary::RandomPointInAnnulusFromStream(SecondStream, 2.0f, 5.0f), 1.e-9));
	TestTrue("Random Point In Sphere stream variant is deterministic",
		UDirectiveUtilMathFunctionLibrary::RandomPointInSphereFromStream(FirstStream, 5.0f).Equals(
			UDirectiveUtilMathFunctionLibrary::RandomPointInSphereFromStream(SecondStream, 5.0f), 1.e-9));

	FRandomStream ExpectedCircleStream(24680);
	const double ExpectedCircleAngle = static_cast<double>(ExpectedCircleStream.FRand()) * UE_TWO_PI;
	const double ExpectedCircleRadius = FMath::Sqrt(static_cast<double>(ExpectedCircleStream.FRand())) * 5.0;
	const FVector2D ExpectedCirclePoint(
		FMath::Cos(ExpectedCircleAngle) * ExpectedCircleRadius,
		FMath::Sin(ExpectedCircleAngle) * ExpectedCircleRadius);
	FRandomStream ActualCircleStream(24680);
	TestTrue("Random Point In Circle consumes angle before radius",
		UDirectiveUtilMathFunctionLibrary::RandomPointInCircleFromStream(ActualCircleStream, 5.0f).Equals(
			ExpectedCirclePoint, 1.e-12)
		&& ActualCircleStream.GetCurrentSeed() == ExpectedCircleStream.GetCurrentSeed());

	FRandomStream ExpectedAnnulusStream(13579);
	const double ExpectedAnnulusAngle = static_cast<double>(ExpectedAnnulusStream.FRand()) * UE_TWO_PI;
	const double ExpectedAnnulusRadius = FMath::Sqrt(FMath::Lerp(
		4.0, 25.0, static_cast<double>(ExpectedAnnulusStream.FRand())));
	const FVector2D ExpectedAnnulusPoint(
		FMath::Cos(ExpectedAnnulusAngle) * ExpectedAnnulusRadius,
		FMath::Sin(ExpectedAnnulusAngle) * ExpectedAnnulusRadius);
	FRandomStream ActualAnnulusStream(13579);
	TestTrue("Random Point In Annulus consumes angle before radius",
		UDirectiveUtilMathFunctionLibrary::RandomPointInAnnulusFromStream(ActualAnnulusStream, 2.0f, 5.0f).Equals(
			ExpectedAnnulusPoint, 1.e-12)
		&& ActualAnnulusStream.GetCurrentSeed() == ExpectedAnnulusStream.GetCurrentSeed());

	FRandomStream ExpectedSphereStream(97531);
	FVector ExpectedSpherePoint;
	double ExpectedSphereSizeSquared;
	do
	{
		const double X = static_cast<double>(ExpectedSphereStream.FRand()) * 2.0 - 1.0;
		const double Y = static_cast<double>(ExpectedSphereStream.FRand()) * 2.0 - 1.0;
		const double Z = static_cast<double>(ExpectedSphereStream.FRand()) * 2.0 - 1.0;
		ExpectedSpherePoint = FVector(X, Y, Z);
		ExpectedSphereSizeSquared = ExpectedSpherePoint.SizeSquared();
	}
	while (ExpectedSphereSizeSquared > 1.0);
	ExpectedSpherePoint *= 5.0;
	FRandomStream ActualSphereStream(97531);
	TestTrue("Random Point In Sphere consumes coordinates in XYZ order",
		UDirectiveUtilMathFunctionLibrary::RandomPointInSphereFromStream(ActualSphereStream, 5.0f).Equals(
			ExpectedSpherePoint, 1.e-12)
		&& ActualSphereStream.GetCurrentSeed() == ExpectedSphereStream.GetCurrentSeed());

	FRandomStream UnchangedStream(86420);
	const int32 UnchangedSeed = UnchangedStream.GetCurrentSeed();
	UDirectiveUtilMathFunctionLibrary::RandomPointInCircleFromStream(UnchangedStream, 0.0f);
	UDirectiveUtilMathFunctionLibrary::RandomPointInAnnulusFromStream(UnchangedStream, 0.0f, 0.0f);
	UDirectiveUtilMathFunctionLibrary::RandomPointInSphereFromStream(
		UnchangedStream, std::numeric_limits<float>::infinity());
	TestEqual("Invalid and zero radii do not advance random streams",
		UnchangedStream.GetCurrentSeed(), UnchangedSeed);

	FRandomStream DistributionStream(112358);
	double CircleDistributionMean = 0.0;
	double AnnulusDistributionMean = 0.0;
	double SphereDistributionMean = 0.0;
	constexpr int32 DistributionSampleCount = 10000;
	for (int32 Index = 0; Index < DistributionSampleCount; ++Index)
	{
		const FVector2D CirclePoint = UDirectiveUtilMathFunctionLibrary::RandomPointInCircleFromStream(
			DistributionStream, 5.0f);
		const FVector2D AnnulusPoint = UDirectiveUtilMathFunctionLibrary::RandomPointInAnnulusFromStream(
			DistributionStream, 2.0f, 5.0f);
		const FVector SpherePoint = UDirectiveUtilMathFunctionLibrary::RandomPointInSphereFromStream(
			DistributionStream, 5.0f);
		CircleDistributionMean += CirclePoint.SizeSquared() / 25.0;
		AnnulusDistributionMean += (AnnulusPoint.SizeSquared() - 4.0) / 21.0;
		SphereDistributionMean += FMath::Pow(SpherePoint.Size() / 5.0, 3.0);
	}
	CircleDistributionMean /= DistributionSampleCount;
	AnnulusDistributionMean /= DistributionSampleCount;
	SphereDistributionMean /= DistributionSampleCount;
	TestTrue("Random Point In Circle is uniform by area",
		FMath::IsNearlyEqual(CircleDistributionMean, 0.5, 0.02));
	TestTrue("Random Point In Annulus is uniform by area",
		FMath::IsNearlyEqual(AnnulusDistributionMean, 0.5, 0.02));
	TestTrue("Random Point In Sphere is uniform by volume",
		FMath::IsNearlyEqual(SphereDistributionMean, 0.5, 0.02));
	TestTrue("Random point functions reject non-finite radii",
		UDirectiveUtilMathFunctionLibrary::RandomPointInCircle(
			std::numeric_limits<float>::quiet_NaN()).IsZero()
		&& UDirectiveUtilMathFunctionLibrary::RandomPointInAnnulus(
			0.0f, std::numeric_limits<float>::infinity()).IsZero()
		&& UDirectiveUtilMathFunctionLibrary::RandomPointInSphere(
			std::numeric_limits<float>::infinity()).IsZero());

	return !HasAnyErrors();
}
