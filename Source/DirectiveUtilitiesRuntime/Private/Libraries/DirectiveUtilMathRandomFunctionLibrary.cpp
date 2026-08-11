// Copyright (c) 2026 Unreal Directive. Licensed under the MIT License.

#include "Libraries/DirectiveUtilMathFunctionLibrary.h"

namespace
{
	float GetUsableRandomWeight(const float Weight)
	{
		return FMath::IsFinite(Weight) && Weight > 0.0f ? Weight : 0.0f;
	}

	FVector2D MakeRandomPointInAnnulus(const double InnerRadius, const double OuterRadius,
		const double AngleSample, const double RadiusSample)
	{
		const double Angle = AngleSample * UE_TWO_PI;
		const double Radius = FMath::Sqrt(FMath::Lerp(
			InnerRadius * InnerRadius,
			OuterRadius * OuterRadius,
			RadiusSample));
		return FVector2D(FMath::Cos(Angle) * Radius, FMath::Sin(Angle) * Radius);
	}

	template <typename RandomFractionFunction>
	FVector MakeRandomPointInSphere(const double Radius, RandomFractionFunction&& RandomFraction)
	{
		FVector Point;
		double SizeSquared;
		do
		{
			const double X = static_cast<double>(RandomFraction()) * 2.0 - 1.0;
			const double Y = static_cast<double>(RandomFraction()) * 2.0 - 1.0;
			const double Z = static_cast<double>(RandomFraction()) * 2.0 - 1.0;
			Point = FVector(X, Y, Z);
			SizeSquared = Point.SizeSquared();
		}
		while (SizeSquared > 1.0);

		return Point * Radius;
	}
}

int32 UDirectiveUtilMathFunctionLibrary::GetRandomIndexFromWeights(const TArray<float>& Weights)
{
	double Total = 0.0;
	for (const float Weight : Weights)
	{
		Total += GetUsableRandomWeight(Weight);
	}

	if (Total <= 0.0)
	{
		return INDEX_NONE;
	}

	const double Roll = static_cast<double>(FMath::FRand()) * Total;
	double Accumulated = 0.0;
	int32 LastPositiveIndex = INDEX_NONE;
	for (int32 Index = 0; Index < Weights.Num(); ++Index)
	{
		const float Weight = GetUsableRandomWeight(Weights[Index]);
		if (Weight <= 0.0f)
		{
			continue;
		}
		LastPositiveIndex = Index;
		Accumulated += Weight;
		if (Roll < Accumulated)
		{
			return Index;
		}
	}

	return LastPositiveIndex;
}

int32 UDirectiveUtilMathFunctionLibrary::GetRandomIndexFromWeightsFromStream(FRandomStream& Stream, const TArray<float>& Weights)
{
	double Total = 0.0;
	for (const float Weight : Weights)
	{
		Total += GetUsableRandomWeight(Weight);
	}

	if (Total <= 0.0)
	{
		return INDEX_NONE;
	}

	const double Roll = static_cast<double>(Stream.FRand()) * Total;
	double Accumulated = 0.0;
	int32 LastPositiveIndex = INDEX_NONE;
	for (int32 Index = 0; Index < Weights.Num(); ++Index)
	{
		const float Weight = GetUsableRandomWeight(Weights[Index]);
		if (Weight <= 0.0f)
		{
			continue;
		}
		LastPositiveIndex = Index;
		Accumulated += Weight;
		if (Roll < Accumulated)
		{
			return Index;
		}
	}

	return LastPositiveIndex;
}

FVector2D UDirectiveUtilMathFunctionLibrary::RandomPointInCircle(const float Radius)
{
	if (!FMath::IsFinite(Radius))
	{
		return FVector2D::ZeroVector;
	}

	const double AbsoluteRadius = FMath::Abs(static_cast<double>(Radius));
	if (AbsoluteRadius == 0.0)
	{
		return FVector2D::ZeroVector;
	}
	const double AngleSample = FMath::FRand();
	const double RadiusSample = FMath::FRand();
	return MakeRandomPointInAnnulus(0.0, AbsoluteRadius, AngleSample, RadiusSample);
}

FVector2D UDirectiveUtilMathFunctionLibrary::RandomPointInCircleFromStream(FRandomStream& Stream, const float Radius)
{
	if (!FMath::IsFinite(Radius))
	{
		return FVector2D::ZeroVector;
	}

	const double AbsoluteRadius = FMath::Abs(static_cast<double>(Radius));
	if (AbsoluteRadius == 0.0)
	{
		return FVector2D::ZeroVector;
	}
	const double AngleSample = Stream.FRand();
	const double RadiusSample = Stream.FRand();
	return MakeRandomPointInAnnulus(0.0, AbsoluteRadius, AngleSample, RadiusSample);
}

FVector2D UDirectiveUtilMathFunctionLibrary::RandomPointInAnnulus(const float InnerRadius, const float OuterRadius)
{
	if (!FMath::IsFinite(InnerRadius) || !FMath::IsFinite(OuterRadius))
	{
		return FVector2D::ZeroVector;
	}

	const double FirstRadius = FMath::Abs(static_cast<double>(InnerRadius));
	const double SecondRadius = FMath::Abs(static_cast<double>(OuterRadius));
	const double Inner = FMath::Min(FirstRadius, SecondRadius);
	const double Outer = FMath::Max(FirstRadius, SecondRadius);
	if (Outer == 0.0)
	{
		return FVector2D::ZeroVector;
	}
	const double AngleSample = FMath::FRand();
	const double RadiusSample = FMath::FRand();
	return MakeRandomPointInAnnulus(Inner, Outer, AngleSample, RadiusSample);
}

FVector2D UDirectiveUtilMathFunctionLibrary::RandomPointInAnnulusFromStream(FRandomStream& Stream,
	const float InnerRadius, const float OuterRadius)
{
	if (!FMath::IsFinite(InnerRadius) || !FMath::IsFinite(OuterRadius))
	{
		return FVector2D::ZeroVector;
	}

	const double FirstRadius = FMath::Abs(static_cast<double>(InnerRadius));
	const double SecondRadius = FMath::Abs(static_cast<double>(OuterRadius));
	const double Inner = FMath::Min(FirstRadius, SecondRadius);
	const double Outer = FMath::Max(FirstRadius, SecondRadius);
	if (Outer == 0.0)
	{
		return FVector2D::ZeroVector;
	}
	const double AngleSample = Stream.FRand();
	const double RadiusSample = Stream.FRand();
	return MakeRandomPointInAnnulus(Inner, Outer, AngleSample, RadiusSample);
}

FVector UDirectiveUtilMathFunctionLibrary::RandomPointInSphere(const float Radius)
{
	if (!FMath::IsFinite(Radius))
	{
		return FVector::ZeroVector;
	}

	const double AbsoluteRadius = FMath::Abs(static_cast<double>(Radius));
	if (AbsoluteRadius == 0.0)
	{
		return FVector::ZeroVector;
	}
	return MakeRandomPointInSphere(AbsoluteRadius, []
	{
		return FMath::FRand();
	});
}

FVector UDirectiveUtilMathFunctionLibrary::RandomPointInSphereFromStream(FRandomStream& Stream, const float Radius)
{
	if (!FMath::IsFinite(Radius))
	{
		return FVector::ZeroVector;
	}

	const double AbsoluteRadius = FMath::Abs(static_cast<double>(Radius));
	if (AbsoluteRadius == 0.0)
	{
		return FVector::ZeroVector;
	}
	return MakeRandomPointInSphere(AbsoluteRadius, [&Stream]
	{
		return Stream.FRand();
	});
}
