// Copyright (c) 2026 Unreal Directive. Licensed under the MIT License.

#include "Libraries/DirectiveUtilMathFunctionLibrary.h"

#include "Components/SplineComponent.h"
#include "Math/RotationMatrix.h"
#include "Misc/AutomationTest.h"

#include <limits>

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDirectiveUtilTransformArrayTest,
	"DirectiveUtilities.Math.TransformArrays",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::EngineFilter)

bool FDirectiveUtilTransformArrayTest::RunTest(const FString& Parameters)
{
	const TArray<FVector> Locations = {
		FVector(1.0, 2.0, 3.0),
		FVector(4.0, 5.0, 6.0),
		FVector(7.0, 8.0, 9.0)
	};
	const FRotator SharedRotator(10.0, 20.0, 30.0);
	const FQuat SharedRotation = SharedRotator.Quaternion();
	const FVector SharedScale(2.0, 3.0, 4.0);

	const TArray<FTransform> SharedTransforms = UDirectiveUtilMathFunctionLibrary::LocationsToTransforms(
		Locations, SharedRotator, SharedScale);
	bool bSharedTransformsValid = SharedTransforms.Num() == Locations.Num();
	for (int32 Index = 0; Index < SharedTransforms.Num(); ++Index)
	{
		bSharedTransformsValid &= SharedTransforms[Index].GetLocation() == Locations[Index];
		bSharedTransformsValid &= SharedTransforms[Index].GetRotation().Equals(SharedRotation, 1.e-12);
		bSharedTransformsValid &= SharedTransforms[Index].GetScale3D() == SharedScale;
	}
	TestTrue(TEXT("Locations to transforms preserves order and broadcasts rotation and scale"),
		bSharedTransformsValid);
	TestTrue(TEXT("Locations to transforms accepts an empty location array"),
		UDirectiveUtilMathFunctionLibrary::LocationsToTransforms(
			{}, SharedRotator, SharedScale).IsEmpty());

	TArray<FTransform> Transforms;
	TestTrue(TEXT("Empty attribute arrays use identity rotation and scale"),
		UDirectiveUtilMathFunctionLibrary::MakeTransformsFromArrays(Locations, {}, {}, Transforms));
	bool bIdentityAttributesValid = Transforms.Num() == Locations.Num();
	for (int32 Index = 0; Index < Transforms.Num(); ++Index)
	{
		bIdentityAttributesValid &= Transforms[Index].GetLocation() == Locations[Index];
		bIdentityAttributesValid &= Transforms[Index].GetRotation().Equals(FQuat::Identity, 1.e-12);
		bIdentityAttributesValid &= Transforms[Index].GetScale3D() == FVector::OneVector;
	}
	TestTrue(TEXT("Identity attributes preserve every location"), bIdentityAttributesValid);

	TestTrue(TEXT("Single attribute values broadcast across the location array"),
		UDirectiveUtilMathFunctionLibrary::MakeTransformsFromArrays(
			Locations, { SharedRotator }, { SharedScale }, Transforms));
	bool bBroadcastAttributesValid = Transforms.Num() == Locations.Num();
	for (const FTransform& Transform : Transforms)
	{
		bBroadcastAttributesValid &= Transform.GetRotation().Equals(SharedRotation, 1.e-12);
		bBroadcastAttributesValid &= Transform.GetScale3D() == SharedScale;
	}
	TestTrue(TEXT("Broadcast attributes are applied to every transform"), bBroadcastAttributesValid);

	const TArray<FRotator> Rotations = {
		FRotator::ZeroRotator,
		FRotator(0.0, 90.0, 0.0),
		FRotator(45.0, 0.0, 0.0)
	};
	const TArray<FVector> Scales = {
		FVector::OneVector,
		FVector(2.0),
		FVector(-1.0, 1.0, 0.5)
	};
	TestTrue(TEXT("Full attribute arrays map element by element"),
		UDirectiveUtilMathFunctionLibrary::MakeTransformsFromArrays(
			Locations, Rotations, Scales, Transforms));
	bool bPerTransformAttributesValid = Transforms.Num() == Locations.Num();
	for (int32 Index = 0; Index < Transforms.Num(); ++Index)
	{
		bPerTransformAttributesValid &= Transforms[Index].GetLocation() == Locations[Index];
		bPerTransformAttributesValid &= Transforms[Index].GetRotation().Equals(
			Rotations[Index].Quaternion(), 1.e-12);
		bPerTransformAttributesValid &= Transforms[Index].GetScale3D() == Scales[Index];
	}
	TestTrue(TEXT("Per-transform attributes preserve index alignment"), bPerTransformAttributesValid);

	Transforms = { FTransform::Identity };
	TestFalse(TEXT("Mismatched rotation counts are rejected"),
		UDirectiveUtilMathFunctionLibrary::MakeTransformsFromArrays(
			Locations, { FRotator::ZeroRotator, FRotator::ZeroRotator }, {}, Transforms));
	TestTrue(TEXT("A rejected attribute count clears the output"), Transforms.IsEmpty());
	TestFalse(TEXT("Mismatched scale counts are rejected"),
		UDirectiveUtilMathFunctionLibrary::MakeTransformsFromArrays(
			Locations, {}, { FVector::OneVector, FVector::OneVector }, Transforms));
	TestTrue(TEXT("A rejected scale count clears the output"), Transforms.IsEmpty());

	const double Infinity = std::numeric_limits<double>::infinity();
	const FRotator InvalidRotation(Infinity, 0.0, 0.0);
	TestTrue(TEXT("Locations to transforms rejects non-finite values"),
		UDirectiveUtilMathFunctionLibrary::LocationsToTransforms(
			{ FVector(Infinity, 0.0, 0.0) }, FRotator::ZeroRotator, FVector::OneVector).IsEmpty()
		&& UDirectiveUtilMathFunctionLibrary::LocationsToTransforms(
			Locations, InvalidRotation, FVector::OneVector).IsEmpty()
		&& UDirectiveUtilMathFunctionLibrary::LocationsToTransforms(
			Locations, FRotator::ZeroRotator, FVector(Infinity)).IsEmpty());
	TestFalse(TEXT("Transform arrays reject a non-finite location"),
		UDirectiveUtilMathFunctionLibrary::MakeTransformsFromArrays(
			{ FVector::ZeroVector, FVector(Infinity) }, {}, {}, Transforms));
	TestTrue(TEXT("A non-finite location clears partial output"), Transforms.IsEmpty());
	TestFalse(TEXT("Transform arrays reject a non-finite rotation"),
		UDirectiveUtilMathFunctionLibrary::MakeTransformsFromArrays(
			Locations, { InvalidRotation }, {}, Transforms));
	TestTrue(TEXT("A non-finite rotation leaves no output"), Transforms.IsEmpty());
	TestFalse(TEXT("Transform arrays reject a non-finite scale"),
		UDirectiveUtilMathFunctionLibrary::MakeTransformsFromArrays(
			Locations, {}, { FVector(Infinity) }, Transforms));
	TestTrue(TEXT("A non-finite scale leaves no output"), Transforms.IsEmpty());

	TestTrue(TEXT("Empty locations produce a valid empty transform array"),
		UDirectiveUtilMathFunctionLibrary::MakeTransformsFromArrays(
			{}, { SharedRotator }, { SharedScale }, Transforms));
	TestTrue(TEXT("An empty transform result contains no values"), Transforms.IsEmpty());

	const FVector LargeLocation(1000000.0, -2000000.0, 3000000.0);
	const FRotator WrappedRotation(-1080.0, 1440.0, 720.0);
	const FVector SignedScale(-2.0, 0.0, 4.0);
	const TArray<FTransform> OddTransforms = UDirectiveUtilMathFunctionLibrary::LocationsToTransforms(
		{LargeLocation, -LargeLocation}, WrappedRotation, SignedScale);
	TestTrue(TEXT("Locations to transforms preserves large locations and signed scales"),
		OddTransforms.Num() == 2
		&& OddTransforms[0].GetLocation() == LargeLocation
		&& OddTransforms[1].GetLocation() == -LargeLocation
		&& OddTransforms[0].GetScale3D() == SignedScale
		&& OddTransforms[1].GetScale3D() == SignedScale
		&& OddTransforms[0].GetRotation().Equals(WrappedRotation.Quaternion(), 1.e-12));

	const FVector SingleLocation(-7.0, 11.0, -13.0);
	const FRotator SingleRotation(17.0, -29.0, 43.0);
	const FVector SingleScale(0.0, -1.0, 2.0);
	TestTrue(TEXT("Single-element attribute arrays map without special-case drift"),
		UDirectiveUtilMathFunctionLibrary::MakeTransformsFromArrays(
			{SingleLocation}, {SingleRotation}, {SingleScale}, Transforms)
		&& Transforms.Num() == 1
		&& Transforms[0].GetLocation() == SingleLocation
		&& Transforms[0].GetRotation().Equals(SingleRotation.Quaternion(), 1.e-12)
		&& Transforms[0].GetScale3D() == SingleScale);

	Transforms = {FTransform::Identity};
	TestFalse(TEXT("Empty locations still reject an impossible rotation count"),
		UDirectiveUtilMathFunctionLibrary::MakeTransformsFromArrays(
			{}, {FRotator::ZeroRotator, SharedRotator}, {}, Transforms));
	TestTrue(TEXT("Rejected empty-location attributes clear the output"), Transforms.IsEmpty());
	Transforms = {FTransform::Identity};
	TestFalse(TEXT("Empty locations still reject an impossible scale count"),
		UDirectiveUtilMathFunctionLibrary::MakeTransformsFromArrays(
			{}, {}, {FVector::OneVector, SharedScale}, Transforms));
	TestTrue(TEXT("Rejected empty-location scales clear the output"), Transforms.IsEmpty());

	const FVector FacingTarget(100.0, -200.0, 300.0);
	const TArray<FVector> FacingLocations = {
		FacingTarget + FVector(10.0, 0.0, 0.0),
		FacingTarget + FVector(0.0, -20.0, 0.0),
		FacingTarget
	};
	const TArray<FTransform> FacingTransforms =
		UDirectiveUtilMathFunctionLibrary::LocationsToFacingTransforms(
			FacingLocations, FacingTarget, FVector::UpVector, FRotator::ZeroRotator, SharedScale, false);
	TestTrue(TEXT("Facing transforms preserve locations and face their target"),
		FacingTransforms.Num() == 3
		&& FacingTransforms[0].GetLocation() == FacingLocations[0]
		&& FacingTransforms[0].GetRotation().GetAxisX().Equals(FVector::BackwardVector, 1.e-8)
		&& FacingTransforms[1].GetRotation().GetAxisX().Equals(FVector::RightVector, 1.e-8)
		&& FacingTransforms[2].GetRotation().Equals(FQuat::Identity, 1.e-12)
		&& FacingTransforms[0].GetScale3D() == SharedScale);

	const TArray<FTransform> AwayTransforms =
		UDirectiveUtilMathFunctionLibrary::LocationsToFacingTransforms(
			FacingLocations, FacingTarget, FVector::UpVector, FRotator::ZeroRotator, FVector::OneVector, true);
	TestTrue(TEXT("Facing transforms can point away from their target"),
		AwayTransforms.Num() == 3
		&& AwayTransforms[0].GetRotation().GetAxisX().Equals(FVector::ForwardVector, 1.e-8)
		&& AwayTransforms[1].GetRotation().GetAxisX().Equals(FVector::LeftVector, 1.e-8));

	const FRotator FacingOffset(13.0, 17.0, 19.0);
	const FQuat ExpectedFacingOffset = FRotationMatrix::MakeFromXZ(
		FVector::BackwardVector, FVector::UpVector).ToQuat() * FacingOffset.Quaternion();
	const TArray<FTransform> OffsetFacingTransforms =
		UDirectiveUtilMathFunctionLibrary::LocationsToFacingTransforms(
			{FacingLocations[0]}, FacingTarget, FVector::UpVector, FacingOffset);
	TestTrue(TEXT("Facing transforms apply their rotation offset in local space"),
		OffsetFacingTransforms.Num() == 1
		&& OffsetFacingTransforms[0].GetRotation().Equals(ExpectedFacingOffset, 1.e-12));

	TestTrue(TEXT("Facing transforms reject invalid shared inputs"),
		UDirectiveUtilMathFunctionLibrary::LocationsToFacingTransforms(
			FacingLocations, FacingTarget, FVector::ZeroVector).IsEmpty()
		&& UDirectiveUtilMathFunctionLibrary::LocationsToFacingTransforms(
			FacingLocations, FVector(Infinity), FVector::UpVector).IsEmpty()
		&& UDirectiveUtilMathFunctionLibrary::LocationsToFacingTransforms(
			FacingLocations, FacingTarget, FVector::UpVector, InvalidRotation).IsEmpty()
		&& UDirectiveUtilMathFunctionLibrary::LocationsToFacingTransforms(
			FacingLocations, FacingTarget, FVector::UpVector, FRotator::ZeroRotator,
			FVector(Infinity)).IsEmpty());

	const FVector RadialCenter(1000.0, -2000.0, 3000.0);
	const FRotator RadialPlane(17.0, 31.0, 43.0);
	const FQuat RadialPlaneQuaternion = RadialPlane.Quaternion();
	const FVector RadialNormal = RadialPlaneQuaternion.GetAxisZ();
	const TArray<FVector> CircleLocations = UDirectiveUtilMathFunctionLibrary::GeneratePointsOnCircle(
		RadialCenter, RadialPlane, 25.0, 12, 11.0);
	const TArray<FTransform> InwardCircleTransforms =
		UDirectiveUtilMathFunctionLibrary::GenerateTransformsOnCircle(
			RadialCenter, RadialPlane, 25.0, 12, 11.0,
			EDirectiveUtilRadialOrientation::FaceCenter, FRotator::ZeroRotator, SharedScale);
	bool bInwardCircleValid = InwardCircleTransforms.Num() == CircleLocations.Num();
	for (int32 Index = 0; Index < InwardCircleTransforms.Num(); ++Index)
	{
		const FVector Inward = (RadialCenter - CircleLocations[Index]).GetSafeNormal();
		bInwardCircleValid &= InwardCircleTransforms[Index].GetLocation().Equals(CircleLocations[Index], 1.e-8);
		bInwardCircleValid &= InwardCircleTransforms[Index].GetRotation().GetAxisX().Equals(Inward, 1.e-8);
		bInwardCircleValid &= InwardCircleTransforms[Index].GetScale3D() == SharedScale;
	}
	TestTrue(TEXT("Circle transforms match point locations and face their center"), bInwardCircleValid);

	const TArray<FTransform> OutwardCircleTransforms =
		UDirectiveUtilMathFunctionLibrary::GenerateTransformsOnCircle(
			RadialCenter, RadialPlane, 25.0, 12, 11.0,
			EDirectiveUtilRadialOrientation::FaceAwayFromCenter);
	const TArray<FTransform> ForwardCircleTransforms =
		UDirectiveUtilMathFunctionLibrary::GenerateTransformsOnCircle(
			RadialCenter, RadialPlane, 25.0, 12, 11.0,
			EDirectiveUtilRadialOrientation::FollowPath);
	const TArray<FTransform> ReverseCircleTransforms =
		UDirectiveUtilMathFunctionLibrary::GenerateTransformsOnCircle(
			RadialCenter, RadialPlane, 25.0, 12, 11.0,
			EDirectiveUtilRadialOrientation::FaceAgainstPath);
	bool bCircleOrientationsValid = OutwardCircleTransforms.Num() == CircleLocations.Num()
		&& ForwardCircleTransforms.Num() == CircleLocations.Num()
		&& ReverseCircleTransforms.Num() == CircleLocations.Num();
	for (int32 Index = 0; Index < CircleLocations.Num() && bCircleOrientationsValid; ++Index)
	{
		const FVector Radial = (CircleLocations[Index] - RadialCenter).GetSafeNormal();
		const FVector Tangent = FVector::CrossProduct(RadialNormal, Radial).GetSafeNormal();
		bCircleOrientationsValid &= OutwardCircleTransforms[Index].GetRotation().GetAxisX().Equals(Radial, 1.e-8);
		bCircleOrientationsValid &= ForwardCircleTransforms[Index].GetRotation().GetAxisX().Equals(Tangent, 1.e-8);
		bCircleOrientationsValid &= ReverseCircleTransforms[Index].GetRotation().GetAxisX().Equals(-Tangent, 1.e-8);
	}
	TestTrue(TEXT("Circle transforms support outward and both path orientations"), bCircleOrientationsValid);

	const TArray<FTransform> FixedCircleTransforms =
		UDirectiveUtilMathFunctionLibrary::GenerateTransformsOnCircle(
			RadialCenter, RadialPlane, 25.0, 4, 0.0,
			EDirectiveUtilRadialOrientation::Fixed, FacingOffset);
	FQuat ExpectedFixedRotation = RadialPlaneQuaternion * FacingOffset.Quaternion();
	ExpectedFixedRotation.Normalize();
	TestTrue(TEXT("Fixed circle transforms preserve the plane rotation and local offset"),
		FixedCircleTransforms.Num() == 4
		&& FixedCircleTransforms[0].GetRotation().Equals(ExpectedFixedRotation, 1.e-12)
		&& FixedCircleTransforms[3].GetRotation().Equals(ExpectedFixedRotation, 1.e-12));

	const TArray<FTransform> NegativeArcTransforms =
		UDirectiveUtilMathFunctionLibrary::GenerateTransformsOnArc(
			RadialCenter, RadialPlane, 25.0, 3, 0.0, -90.0, true,
			EDirectiveUtilRadialOrientation::FollowPath);
	TestTrue(TEXT("A negative arc reverses follow-path orientation"),
		NegativeArcTransforms.Num() == 3
		&& NegativeArcTransforms[0].GetRotation().GetAxisX().Equals(
			-RadialPlaneQuaternion.GetAxisY(), 1.e-8));
	const TArray<FVector> NegativeArcLocations = UDirectiveUtilMathFunctionLibrary::GeneratePointsOnArc(
		RadialCenter, RadialPlane, 25.0, 3, 0.0, -90.0, true);
	TestTrue(TEXT("Arc transforms match translated rotated point generation"),
		NegativeArcTransforms.Num() == NegativeArcLocations.Num()
		&& NegativeArcTransforms[0].GetLocation().Equals(NegativeArcLocations[0], 1.e-8)
		&& NegativeArcTransforms[1].GetLocation().Equals(NegativeArcLocations[1], 1.e-8)
		&& NegativeArcTransforms[2].GetLocation().Equals(NegativeArcLocations[2], 1.e-8));

	const TArray<FTransform> ZeroRadiusArcTransforms =
		UDirectiveUtilMathFunctionLibrary::GenerateTransformsOnArc(
			RadialCenter, RadialPlane, 0.0, 3, 0.0, 90.0, true,
			EDirectiveUtilRadialOrientation::FaceCenter);
	TestTrue(TEXT("Zero-radius arc transforms retain a deterministic radial orientation"),
		ZeroRadiusArcTransforms.Num() == 3
		&& ZeroRadiusArcTransforms[0].GetLocation() == RadialCenter
		&& ZeroRadiusArcTransforms[0].GetRotation().GetAxisX().Equals(
			-RadialPlaneQuaternion.GetAxisX(), 1.e-8));

	TestTrue(TEXT("Radial transform generators reject invalid input"),
		UDirectiveUtilMathFunctionLibrary::GenerateTransformsOnCircle(
			RadialCenter, RadialPlane, Infinity, 3).IsEmpty()
		&& UDirectiveUtilMathFunctionLibrary::GenerateTransformsOnArc(
			RadialCenter, RadialPlane, 1.0, 3, 0.0, Infinity).IsEmpty()
		&& UDirectiveUtilMathFunctionLibrary::GenerateTransformsOnCircle(
			RadialCenter, RadialPlane, 1.0, 3, 0.0,
			static_cast<EDirectiveUtilRadialOrientation>(255)).IsEmpty());

	USplineComponent* Spline = NewObject<USplineComponent>();
	Spline->SetSplinePoints({FVector::ZeroVector, FVector(100.0, 0.0, 0.0)},
		ESplineCoordinateSpace::Local, false);
	Spline->SetSplinePointType(0, ESplinePointType::Linear, false);
	Spline->SetSplinePointType(1, ESplinePointType::Linear, false);
	Spline->SetScaleAtSplinePoint(0, FVector(1.0, 2.0, 3.0), false);
	Spline->SetScaleAtSplinePoint(1, FVector(3.0, 4.0, 5.0), true);
	const FVector SplineScaleMultiplier(2.0, 0.5, -1.0);
	const TArray<FTransform> SplineTransforms =
		UDirectiveUtilMathFunctionLibrary::GenerateTransformsAlongSpline(
			Spline, 30.0, true, EDirectiveUtilSplineSpacingMode::Fixed,
			ESplineCoordinateSpace::World, true, FacingOffset, SplineScaleMultiplier);
	const double SplineDistances[] = {0.0, 30.0, 60.0, 90.0, 100.0};
	bool bSplineTransformsValid = SplineTransforms.Num() == 5;
	for (int32 Index = 0; Index < SplineTransforms.Num(); ++Index)
	{
		FTransform Expected = Spline->GetTransformAtDistanceAlongSpline(
			static_cast<float>(SplineDistances[Index]), ESplineCoordinateSpace::World, true);
		FQuat ExpectedRotation = Expected.GetRotation() * FacingOffset.Quaternion();
		ExpectedRotation.Normalize();
		bSplineTransformsValid &= SplineTransforms[Index].GetLocation().Equals(Expected.GetLocation(), 1.e-8);
		bSplineTransformsValid &= SplineTransforms[Index].GetRotation().Equals(ExpectedRotation, 1.e-8);
		bSplineTransformsValid &= SplineTransforms[Index].GetScale3D().Equals(
			Expected.GetScale3D() * SplineScaleMultiplier, 1.e-8);
	}
	TestTrue(TEXT("Spline transforms preserve sampling, spline rotation, and spline scale"),
		bSplineTransformsValid);

	const TArray<FTransform> UnscaledSplineTransforms =
		UDirectiveUtilMathFunctionLibrary::GenerateTransformsAlongSpline(
			Spline, 1000.0, true, EDirectiveUtilSplineSpacingMode::Fixed,
			ESplineCoordinateSpace::World, false, FRotator::ZeroRotator, SharedScale);
	TestTrue(TEXT("Spline scale can be replaced by a shared multiplier"),
		UnscaledSplineTransforms.Num() == 2
		&& UnscaledSplineTransforms[0].GetScale3D() == SharedScale
		&& UnscaledSplineTransforms[1].GetScale3D() == SharedScale);

	const TArray<FTransform> CountedSplineTransforms =
		UDirectiveUtilMathFunctionLibrary::GenerateTransformsAlongSplineByCount(Spline, 3, true);
	TestTrue(TEXT("Spline transforms by count include both exact endpoints"),
		CountedSplineTransforms.Num() == 3
		&& CountedSplineTransforms[0].GetLocation().Equals(FVector::ZeroVector, 1.e-4)
		&& CountedSplineTransforms[1].GetLocation().Equals(FVector(50.0, 0.0, 0.0), 1.e-4)
		&& CountedSplineTransforms[2].GetLocation().Equals(FVector(100.0, 0.0, 0.0), 1.e-4));

	USplineComponent* SinglePointSpline = NewObject<USplineComponent>();
	SinglePointSpline->SetSplinePoints({FVector(3.0, 4.0, 5.0)}, ESplineCoordinateSpace::Local, true);
	const TArray<FTransform> SinglePointSplineTransforms =
		UDirectiveUtilMathFunctionLibrary::GenerateTransformsAlongSpline(
			SinglePointSpline, 10.0);
	TestTrue(TEXT("A zero-length spline returns one transform"),
		SinglePointSplineTransforms.Num() == 1
		&& SinglePointSplineTransforms[0].GetLocation().Equals(FVector(3.0, 4.0, 5.0), 1.e-8));

	TestTrue(TEXT("Spline transform generation rejects invalid input"),
		UDirectiveUtilMathFunctionLibrary::GenerateTransformsAlongSpline(
			nullptr, 10.0).IsEmpty()
		&& UDirectiveUtilMathFunctionLibrary::GenerateTransformsAlongSpline(
			Spline, 0.0).IsEmpty()
		&& UDirectiveUtilMathFunctionLibrary::GenerateTransformsAlongSpline(
			Spline, 10.0, true, EDirectiveUtilSplineSpacingMode::Fixed,
			ESplineCoordinateSpace::World, true, InvalidRotation).IsEmpty()
		&& UDirectiveUtilMathFunctionLibrary::GenerateTransformsAlongSpline(
			Spline, 10.0, true, EDirectiveUtilSplineSpacingMode::Fixed,
			ESplineCoordinateSpace::World, true, FRotator::ZeroRotator, FVector(Infinity)).IsEmpty()
		&& UDirectiveUtilMathFunctionLibrary::GenerateTransformsAlongSplineByCount(
			Spline, 0).IsEmpty()
		&& UDirectiveUtilMathFunctionLibrary::GenerateTransformsAlongSplineByCount(
			Spline, 3, true, ESplineCoordinateSpace::World, true, InvalidRotation).IsEmpty());

	const TArray<FTransform> HexGridTransforms =
		UDirectiveUtilMathFunctionLibrary::GenerateRectangularHexGridTransforms(
			RadialCenter, FRotator::ZeroRotator, FIntPoint(2, 2), 25.0,
			EDirectiveUtilHexOrientation::PointyTop, 0.0, false, FacingOffset, SharedScale);
	const TArray<FVector> HexGridPoints = UDirectiveUtilMathFunctionLibrary::GenerateRectangularHexGrid(
		RadialCenter, FRotator::ZeroRotator, FIntPoint(2, 2), 25.0,
		EDirectiveUtilHexOrientation::PointyTop, 0.0, false);
	bool bHexTransformsValid = HexGridTransforms.Num() == 4 && HexGridPoints.Num() == 4;
	for (int32 Index = 0; bHexTransformsValid && Index < HexGridTransforms.Num(); ++Index)
	{
		bHexTransformsValid &= HexGridTransforms[Index].GetLocation().Equals(HexGridPoints[Index], 1.e-9)
			&& HexGridTransforms[Index].GetRotation().Equals(FacingOffset.Quaternion(), 1.e-8)
			&& HexGridTransforms[Index].GetScale3D() == SharedScale;
	}
	TestTrue(TEXT("Rectangular hex grid transforms share instance rotation and scale over grid cells"),
		bHexTransformsValid);
	TestEqual(TEXT("Hexagonal hex grid transforms cover the requested rings"),
		UDirectiveUtilMathFunctionLibrary::GenerateHexagonalHexGridTransforms(
			RadialCenter, FRotator::ZeroRotator, 1, 25.0).Num(), 7);
	TestTrue(TEXT("Hex grid transform generators reject invalid input"),
		UDirectiveUtilMathFunctionLibrary::GenerateRectangularHexGridTransforms(
			RadialCenter, FRotator::ZeroRotator, FIntPoint(2, 2), 25.0,
			EDirectiveUtilHexOrientation::PointyTop, 0.0, true, InvalidRotation).IsEmpty()
		&& UDirectiveUtilMathFunctionLibrary::GenerateHexagonalHexGridTransforms(
			RadialCenter, FRotator::ZeroRotator, 1, 0.0).IsEmpty());

	const TArray<FVector> NoiseBaseLocations = {
		FVector::ZeroVector, FVector(37.0, 11.0, 5.0), FVector(250.0, -90.0, 40.0)
	};
	const TArray<FTransform> NoiseBaseTransforms = UDirectiveUtilMathFunctionLibrary::LocationsToTransforms(
		NoiseBaseLocations, FacingOffset, SharedScale);
	const TArray<FTransform> NoisedTransforms = UDirectiveUtilMathFunctionLibrary::OffsetTransformsByNoise(
		NoiseBaseTransforms, 100.0, 25.0);
	const TArray<FVector> NoisedLocations = UDirectiveUtilMathFunctionLibrary::OffsetLocationsByNoise(
		NoiseBaseLocations, 100.0, 25.0);
	bool bNoiseTransformsValid = NoisedTransforms.Num() == 3 && NoisedLocations.Num() == 3;
	for (int32 Index = 0; bNoiseTransformsValid && Index < NoisedTransforms.Num(); ++Index)
	{
		bNoiseTransformsValid &= NoisedTransforms[Index].GetLocation().Equals(NoisedLocations[Index], 1.e-9)
			&& NoisedTransforms[Index].GetRotation().Equals(NoiseBaseTransforms[Index].GetRotation(), 1.e-9)
			&& NoisedTransforms[Index].GetScale3D() == NoiseBaseTransforms[Index].GetScale3D();
	}
	TestTrue(TEXT("Transform noise offsets match location noise offsets and preserve rotation and scale"),
		bNoiseTransformsValid);
	TestTrue(TEXT("Transform noise offsets reject invalid input"),
		UDirectiveUtilMathFunctionLibrary::OffsetTransformsByNoise(NoiseBaseTransforms, -1.0, 25.0).IsEmpty());

	const TArray<FVector> EaseFromLocations = {
		FVector::ZeroVector, FVector(10.0, 0.0, 0.0), FVector(20.0, 0.0, 0.0)
	};
	const TArray<FVector> EaseToLocations = {
		FVector(0.0, 10.0, 0.0), FVector(10.0, 10.0, 0.0), FVector(20.0, 10.0, 0.0)
	};
	const TArray<FVector> EasedLocations = UDirectiveUtilMathFunctionLibrary::EaseLocationArrays(
		EaseFromLocations, EaseToLocations, 0.5f, EDirectiveUtilEaseType::Linear, {});
	TestTrue(TEXT("Eased location arrays blend element-wise"),
		EasedLocations.Num() == 3
		&& EasedLocations[0].Equals(FVector(0.0, 5.0, 0.0), 1.e-4)
		&& EasedLocations[2].Equals(FVector(20.0, 5.0, 0.0), 1.e-4));
	const TArray<FVector> StaggeredLocations = UDirectiveUtilMathFunctionLibrary::EaseLocationArrays(
		EaseFromLocations, EaseToLocations, 0.0f, EDirectiveUtilEaseType::Linear, { 0.0f, 0.5f, 1.0f });
	TestTrue(TEXT("Per-element alphas stagger the blend"),
		StaggeredLocations.Num() == 3
		&& StaggeredLocations[0].Equals(EaseFromLocations[0], 1.e-4)
		&& StaggeredLocations[1].Equals(FVector(10.0, 5.0, 0.0), 1.e-4)
		&& StaggeredLocations[2].Equals(EaseToLocations[2], 1.e-4));
	TestTrue(TEXT("Eased arrays reject mismatched lengths"),
		UDirectiveUtilMathFunctionLibrary::EaseLocationArrays(
			EaseFromLocations, { FVector::ZeroVector }, 0.5f, EDirectiveUtilEaseType::Linear, {}).IsEmpty()
		&& UDirectiveUtilMathFunctionLibrary::EaseLocationArrays(
			EaseFromLocations, EaseToLocations, 0.5f, EDirectiveUtilEaseType::Linear, { 0.5f, 0.5f }).IsEmpty());

	const TArray<FTransform> EaseFromTransforms = UDirectiveUtilMathFunctionLibrary::LocationsToTransforms(
		EaseFromLocations, FRotator::ZeroRotator, FVector::OneVector);
	const TArray<FTransform> EaseToTransforms = UDirectiveUtilMathFunctionLibrary::LocationsToTransforms(
		EaseToLocations, FRotator(0.0, 90.0, 0.0), FVector(3.0));
	const TArray<FTransform> EasedTransforms = UDirectiveUtilMathFunctionLibrary::EaseTransformArrays(
		EaseFromTransforms, EaseToTransforms, 0.5f, EDirectiveUtilEaseType::Linear, {});
	TestTrue(TEXT("Eased transform arrays blend location, rotation, and scale element-wise"),
		EasedTransforms.Num() == 3
		&& EasedTransforms[1].GetLocation().Equals(FVector(10.0, 5.0, 0.0), 1.e-4)
		&& EasedTransforms[1].GetRotation().Equals(FRotator(0.0, 45.0, 0.0).Quaternion(), 1.e-4)
		&& EasedTransforms[1].GetScale3D().Equals(FVector(2.0), 1.e-4));

	const TArray<FVector> SamplePath = {
		FVector::ZeroVector, FVector(10.0, 0.0, 0.0), FVector(10.0, 10.0, 0.0)
	};
	TestTrue(TEXT("Location array sampling is distance-weighted"),
		UDirectiveUtilMathFunctionLibrary::SampleLocationArray(SamplePath, 0.75f).Equals(
			FVector(10.0, 5.0, 0.0), 1.e-4)
		&& UDirectiveUtilMathFunctionLibrary::SampleLocationArray(SamplePath, 0.0f).Equals(
			SamplePath[0], 1.e-4)
		&& UDirectiveUtilMathFunctionLibrary::SampleLocationArray(SamplePath, 1.0f).Equals(
			SamplePath.Last(), 1.e-4)
		&& UDirectiveUtilMathFunctionLibrary::SampleLocationArray(SamplePath, 1.5f).Equals(
			SamplePath.Last(), 1.e-4));
	TestTrue(TEXT("Closed-loop sampling wraps alpha back to the start"),
		UDirectiveUtilMathFunctionLibrary::SampleLocationArray(SamplePath, 1.0f, true).Equals(
			SamplePath[0], 1.e-4));
	TestTrue(TEXT("Degenerate location array sampling returns the only point or zero"),
		UDirectiveUtilMathFunctionLibrary::SampleLocationArray({ FVector(3.0, 4.0, 5.0) }, 0.7f).Equals(
			FVector(3.0, 4.0, 5.0), 1.e-4)
		&& UDirectiveUtilMathFunctionLibrary::SampleLocationArray({}, 0.5f).IsZero());

	const TArray<FTransform> SampleTransformPath = {
		FTransform(FRotator::ZeroRotator, FVector::ZeroVector, FVector::OneVector),
		FTransform(FRotator(0.0, 90.0, 0.0), FVector(10.0, 0.0, 0.0), FVector(3.0))
	};
	const FTransform SampledTransform = UDirectiveUtilMathFunctionLibrary::SampleTransformArray(
		SampleTransformPath, 0.5f);
	TestTrue(TEXT("Transform array sampling blends location, rotation, and scale"),
		SampledTransform.GetLocation().Equals(FVector(5.0, 0.0, 0.0), 1.e-4)
		&& SampledTransform.GetRotation().Equals(FRotator(0.0, 45.0, 0.0).Quaternion(), 1.e-4)
		&& SampledTransform.GetScale3D().Equals(FVector(2.0), 1.e-4));
	TestTrue(TEXT("Empty transform array sampling returns the identity"),
		UDirectiveUtilMathFunctionLibrary::SampleTransformArray({}, 0.5f).Equals(FTransform::Identity));

	return !HasAnyErrors();
}
