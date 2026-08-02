// Copyright (c) 2026 Unreal Directive. Licensed under the MIT License.

#include "Libraries/DirectiveUtilMathFunctionLibrary.h"

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

	return !HasAnyErrors();
}
