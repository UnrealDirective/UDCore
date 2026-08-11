// Copyright (c) 2026 Unreal Directive. Licensed under the MIT License.

#include "Libraries/DirectiveUtilMathFunctionLibrary.h"

namespace
{
	double EaseBackIn(double t)
	{
		const double s = 1.70158;
		return t * t * ((s + 1.0) * t - s);
	}

	double EaseBackOut(double t)
	{
		const double s = 1.70158;
		t -= 1.0;
		return t * t * ((s + 1.0) * t + s) + 1.0;
	}

	double EaseBackInOut(double t)
	{
		const double s = 1.70158 * 1.525;
		t *= 2.0;
		if (t < 1.0)
		{
			return 0.5 * (t * t * ((s + 1.0) * t - s));
		}
		t -= 2.0;
		return 0.5 * (t * t * ((s + 1.0) * t + s) + 2.0);
	}

	double EaseElasticIn(double t)
	{
		if (t <= 0.0) { return 0.0; }
		if (t >= 1.0) { return 1.0; }
		const double p = 0.3;
		const double s = p / 4.0;
		t -= 1.0;
		return -(FMath::Pow(2.0, 10.0 * t) * FMath::Sin((t - s) * (2.0 * PI) / p));
	}

	double EaseElasticOut(double t)
	{
		if (t <= 0.0) { return 0.0; }
		if (t >= 1.0) { return 1.0; }
		const double p = 0.3;
		const double s = p / 4.0;
		return FMath::Pow(2.0, -10.0 * t) * FMath::Sin((t - s) * (2.0 * PI) / p) + 1.0;
	}

	double EaseElasticInOut(double t)
	{
		if (t <= 0.0) { return 0.0; }
		if (t >= 1.0) { return 1.0; }
		const double p = 0.3 * 1.5;
		const double s = p / 4.0;
		t *= 2.0;
		if (t < 1.0)
		{
			t -= 1.0;
			return -0.5 * (FMath::Pow(2.0, 10.0 * t) * FMath::Sin((t - s) * (2.0 * PI) / p));
		}
		t -= 1.0;
		return FMath::Pow(2.0, -10.0 * t) * FMath::Sin((t - s) * (2.0 * PI) / p) * 0.5 + 1.0;
	}

	double EaseBounceOut(double t)
	{
		const double n1 = 7.5625;
		const double d1 = 2.75;
		if (t < 1.0 / d1)
		{
			return n1 * t * t;
		}
		if (t < 2.0 / d1)
		{
			t -= 1.5 / d1;
			return n1 * t * t + 0.75;
		}
		if (t < 2.5 / d1)
		{
			t -= 2.25 / d1;
			return n1 * t * t + 0.9375;
		}
		t -= 2.625 / d1;
		return n1 * t * t + 0.984375;
	}

	double EaseBounceIn(double t)
	{
		return 1.0 - EaseBounceOut(1.0 - t);
	}

	double EaseBounceInOut(double t)
	{
		return t < 0.5
			? (1.0 - EaseBounceOut(1.0 - 2.0 * t)) * 0.5
			: (1.0 + EaseBounceOut(2.0 * t - 1.0)) * 0.5;
	}


	FTransform BlendEasedTransforms(const FTransform& A, const FTransform& B, const double Alpha)
	{
		FQuat Rotation = FQuat::Slerp(A.GetRotation(), B.GetRotation(), Alpha);
		Rotation.Normalize();
		return FTransform(Rotation,
			FMath::Lerp(A.GetLocation(), B.GetLocation(), Alpha),
			FMath::Lerp(A.GetScale3D(), B.GetScale3D(), Alpha));
	}

	template <typename ValueType, typename BlendType>
	TArray<ValueType> EaseArrays(const TArray<ValueType>& From, const TArray<ValueType>& To, const float Alpha,
		const EDirectiveUtilEaseType EaseType, const TArray<float>& PerElementAlphas, BlendType Blend)
	{
		const bool bPerElement = !PerElementAlphas.IsEmpty();
		if (From.Num() != To.Num() || !FMath::IsFinite(Alpha)
			|| (bPerElement && PerElementAlphas.Num() != From.Num()))
		{
			return {};
		}

		const float SharedEasedAlpha = UDirectiveUtilMathFunctionLibrary::EaseAlpha(Alpha, EaseType);
		TArray<ValueType> Result;
		Result.SetNumUninitialized(From.Num());
		for (int32 Index = 0; Index < From.Num(); ++Index)
		{
			float EasedAlpha = SharedEasedAlpha;
			if (bPerElement)
			{
				if (!FMath::IsFinite(PerElementAlphas[Index]))
				{
					return {};
				}
				EasedAlpha = UDirectiveUtilMathFunctionLibrary::EaseAlpha(PerElementAlphas[Index], EaseType);
			}
			if (From[Index].ContainsNaN() || To[Index].ContainsNaN())
			{
				return {};
			}
			Result[Index] = Blend(From[Index], To[Index], static_cast<double>(EasedAlpha));
		}
		return Result;
	}

}

float UDirectiveUtilMathFunctionLibrary::EaseAlpha(const float Alpha, const EDirectiveUtilEaseType EaseType)
{
	const double t = static_cast<double>(FMath::Clamp(Alpha, 0.0f, 1.0f));
	switch (EaseType)
	{
	case EDirectiveUtilEaseType::BackIn: return static_cast<float>(EaseBackIn(t));
	case EDirectiveUtilEaseType::BackOut: return static_cast<float>(EaseBackOut(t));
	case EDirectiveUtilEaseType::BackInOut: return static_cast<float>(EaseBackInOut(t));
	case EDirectiveUtilEaseType::ElasticIn: return static_cast<float>(EaseElasticIn(t));
	case EDirectiveUtilEaseType::ElasticOut: return static_cast<float>(EaseElasticOut(t));
	case EDirectiveUtilEaseType::ElasticInOut: return static_cast<float>(EaseElasticInOut(t));
	case EDirectiveUtilEaseType::BounceIn: return static_cast<float>(EaseBounceIn(t));
	case EDirectiveUtilEaseType::BounceOut: return static_cast<float>(EaseBounceOut(t));
	case EDirectiveUtilEaseType::BounceInOut: return static_cast<float>(EaseBounceInOut(t));
	case EDirectiveUtilEaseType::Linear:
	default: return static_cast<float>(t);
	}
}

float UDirectiveUtilMathFunctionLibrary::EaseFloat(const float A, const float B, const float Alpha, const EDirectiveUtilEaseType EaseType)
{
	return FMath::Lerp(A, B, EaseAlpha(Alpha, EaseType));
}

FVector UDirectiveUtilMathFunctionLibrary::EaseVector(const FVector& A, const FVector& B, const float Alpha, const EDirectiveUtilEaseType EaseType)
{
	return FMath::Lerp(A, B, static_cast<double>(EaseAlpha(Alpha, EaseType)));
}

FRotator UDirectiveUtilMathFunctionLibrary::EaseRotator(const FRotator& A, const FRotator& B, const float Alpha, const EDirectiveUtilEaseType EaseType)
{
	return FQuat::Slerp(A.Quaternion(), B.Quaternion(), EaseAlpha(Alpha, EaseType)).Rotator();
}

FLinearColor UDirectiveUtilMathFunctionLibrary::EaseColor(const FLinearColor& A, const FLinearColor& B, const float Alpha, const EDirectiveUtilEaseType EaseType)
{
	return FMath::Lerp(A, B, EaseAlpha(Alpha, EaseType));
}

FTransform UDirectiveUtilMathFunctionLibrary::EaseTransform(const FTransform& A, const FTransform& B,
	const float Alpha, const EDirectiveUtilEaseType EaseType)
{
	return BlendEasedTransforms(A, B, static_cast<double>(EaseAlpha(Alpha, EaseType)));
}

TArray<FVector> UDirectiveUtilMathFunctionLibrary::EaseLocationArrays(const TArray<FVector>& From,
	const TArray<FVector>& To, const float Alpha, const EDirectiveUtilEaseType EaseType,
	const TArray<float>& PerElementAlphas)
{
	return EaseArrays(From, To, Alpha, EaseType, PerElementAlphas,
		[](const FVector& A, const FVector& B, const double EasedAlpha)
		{
			return FMath::Lerp(A, B, EasedAlpha);
		});
}

TArray<FTransform> UDirectiveUtilMathFunctionLibrary::EaseTransformArrays(const TArray<FTransform>& From,
	const TArray<FTransform>& To, const float Alpha, const EDirectiveUtilEaseType EaseType,
	const TArray<float>& PerElementAlphas)
{
	return EaseArrays(From, To, Alpha, EaseType, PerElementAlphas, &BlendEasedTransforms);
}
