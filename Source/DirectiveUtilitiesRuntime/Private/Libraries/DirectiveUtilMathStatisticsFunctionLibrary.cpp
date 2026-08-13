// Copyright (c) 2026 Unreal Directive. Licensed under the MIT License.

#include "Libraries/DirectiveUtilMathFunctionLibrary.h"

#include <limits>

namespace
{
	float GetUsableStatisticsWeight(const float Weight)
	{
		return FMath::IsFinite(Weight) && Weight > 0.0f ? Weight : 0.0f;
	}

	template <typename ValueType>
	ValueType SelectStatisticsNth(TArray<ValueType>& Values, const int32 NthIndex)
	{
		int32 Left = 0;
		int32 Right = Values.Num() - 1;
		int32 RemainingDepth = FMath::FloorLog2(static_cast<uint32>(Values.Num())) * 2;
		while (Left < Right)
		{
			if (RemainingDepth-- <= 0)
			{
				Values.Sort();
				return Values[NthIndex];
			}

			const int32 Middle = Left + (Right - Left) / 2;
			if (Values[Middle] < Values[Left])
			{
				Values.Swap(Middle, Left);
			}
			if (Values[Right] < Values[Left])
			{
				Values.Swap(Right, Left);
			}
			if (Values[Right] < Values[Middle])
			{
				Values.Swap(Right, Middle);
			}
			const ValueType Pivot = Values[Middle];

			int32 LessEnd = Left;
			int32 Current = Left;
			int32 GreaterStart = Right;
			while (Current <= GreaterStart)
			{
				if (Values[Current] < Pivot)
				{
					Values.Swap(LessEnd++, Current++);
				}
				else if (Pivot < Values[Current])
				{
					Values.Swap(Current, GreaterStart--);
				}
				else
				{
					++Current;
				}
			}

			if (NthIndex < LessEnd)
			{
				Right = LessEnd - 1;
			}
			else if (NthIndex > GreaterStart)
			{
				Left = GreaterStart + 1;
			}
			else
			{
				return Values[NthIndex];
			}
		}
		return Values[Left];
	}

	template <typename ValueType>
	double CalculateStatisticsMedian(TArray<ValueType>& Values)
	{
		const int32 Middle = Values.Num() / 2;
		const ValueType UpperMiddle = SelectStatisticsNth(Values, Middle);
		if (Values.Num() % 2 != 0)
		{
			return static_cast<double>(UpperMiddle);
		}

		ValueType LowerMiddle = Values[0];
		for (int32 Index = 1; Index < Middle; ++Index)
		{
			LowerMiddle = FMath::Max(LowerMiddle, Values[Index]);
		}
		return (static_cast<double>(LowerMiddle) + static_cast<double>(UpperMiddle)) * 0.5;
	}
}

float UDirectiveUtilMathFunctionLibrary::RoundToDecimals(const float Value, int32 Decimals)
{
	Decimals = FMath::Clamp(Decimals, 0, 10);
	if (Decimals == 0)
	{
		return FMath::RoundHalfFromZero(Value);
	}
	const double Factor = FMath::Pow(10.0, static_cast<double>(Decimals));
	return static_cast<float>(FMath::RoundHalfFromZero(static_cast<double>(Value) * Factor) / Factor);
}

FText UDirectiveUtilMathFunctionLibrary::RoundToDecimalsAsText(const float Value, int32 Decimals)
{
	Decimals = FMath::Clamp(Decimals, 0, 10);
	FNumberFormattingOptions Options;
	Options.MinimumFractionalDigits = 0;
	Options.MaximumFractionalDigits = Decimals;
	Options.RoundingMode = ERoundingMode::HalfFromZero;
	return FText::AsNumber(Value, &Options);
}

FText UDirectiveUtilMathFunctionLibrary::FormatBytes(const int64 Bytes, int32 Decimals)
{
	Decimals = FMath::Clamp(Decimals, 0, 3);

	static const TCHAR* Suffixes[] = { TEXT("B"), TEXT("KB"), TEXT("MB"), TEXT("GB"), TEXT("TB"), TEXT("PB") };
	const bool bNegative = Bytes < 0;
	double Value = FMath::Abs(static_cast<double>(Bytes));
	int32 SuffixIndex = 0;
	while (Value >= 1024.0 && SuffixIndex < UE_ARRAY_COUNT(Suffixes) - 1)
	{
		Value /= 1024.0;
		++SuffixIndex;
	}

	return FText::FromString(FString::Printf(TEXT("%s%.*f %s"),
		bNegative ? TEXT("-") : TEXT(""), SuffixIndex == 0 ? 0 : Decimals, Value, Suffixes[SuffixIndex]));
}

FText UDirectiveUtilMathFunctionLibrary::FormatDuration(const float Seconds, const bool bIncludeSeconds)
{
	if (!FMath::IsFinite(Seconds))
	{
		return FText::FromString(TEXT("0s"));
	}

	const double AbsoluteSeconds = FMath::Abs(static_cast<double>(Seconds));
	const int64 TotalSeconds = AbsoluteSeconds >= static_cast<double>(TNumericLimits<int64>::Max())
		? TNumericLimits<int64>::Max()
		: static_cast<int64>(AbsoluteSeconds);
	const int64 VisibleSeconds = bIncludeSeconds ? TotalSeconds : (TotalSeconds / 60) * 60;
	const bool bNegative = Seconds < 0.0f && VisibleSeconds > 0;

	const int64 UnitValues[] = { TotalSeconds / 86400, (TotalSeconds / 3600) % 24, (TotalSeconds / 60) % 60, TotalSeconds % 60 };
	static const TCHAR* UnitSuffixes[] = { TEXT("d"), TEXT("h"), TEXT("m"), TEXT("s") };
	const int32 NumUnits = bIncludeSeconds ? 4 : 3;

	int32 FirstUnit = NumUnits - 1;
	for (int32 Index = 0; Index < NumUnits; ++Index)
	{
		if (UnitValues[Index] != 0)
		{
			FirstUnit = Index;
			break;
		}
	}
	int32 LastUnit = FirstUnit;
	for (int32 Index = NumUnits - 1; Index >= FirstUnit; --Index)
	{
		if (UnitValues[Index] != 0)
		{
			LastUnit = Index;
			break;
		}
	}

	FString Result = bNegative ? TEXT("-") : TEXT("");
	for (int32 Index = FirstUnit; Index <= LastUnit; ++Index)
	{
		if (Index == FirstUnit)
		{
			Result += FString::Printf(TEXT("%lld%s"), UnitValues[Index], UnitSuffixes[Index]);
		}
		else
		{
			Result += FString::Printf(TEXT(" %02lld%s"), UnitValues[Index], UnitSuffixes[Index]);
		}
	}
	return FText::FromString(Result);
}

FText UDirectiveUtilMathFunctionLibrary::FormatRelativeTime(const FDateTime& Timestamp)
{
	const FTimespan Delta = FDateTime::Now() - Timestamp;
	const bool bFuture = Delta.GetTicks() < 0;
	// Round first so timestamps near the current second stay in the expected bucket.
	const int64 SecondsAbs = static_cast<int64>(FMath::RoundToDouble(FMath::Abs(Delta.GetTotalSeconds())));

	if (SecondsAbs < 60)
	{
		return FText::FromString(TEXT("just now"));
	}

	int64 Count;
	const TCHAR* Unit;
	if (SecondsAbs < 3600)
	{
		Count = SecondsAbs / 60;
		Unit = TEXT("minute");
	}
	else if (SecondsAbs < 86400)
	{
		Count = SecondsAbs / 3600;
		Unit = TEXT("hour");
	}
	else
	{
		Count = SecondsAbs / 86400;
		Unit = TEXT("day");
	}

	const FString Quantity = FString::Printf(TEXT("%lld %s%s"), Count, Unit, Count == 1 ? TEXT("") : TEXT("s"));
	return FText::FromString(bFuture
		? FString::Printf(TEXT("in %s"), *Quantity)
		: FString::Printf(TEXT("%s ago"), *Quantity));
}

int64 UDirectiveUtilMathFunctionLibrary::GetIntArraySum(const TArray<int32>& Values)
{
	int64 Sum = 0;
	for (const int32 Value : Values)
	{
		Sum += Value;
	}
	return Sum;
}

float UDirectiveUtilMathFunctionLibrary::GetIntArrayAverage(const TArray<int32>& Values)
{
	if (Values.IsEmpty())
	{
		return 0.0f;
	}
	return static_cast<float>(static_cast<double>(GetIntArraySum(Values)) / Values.Num());
}

float UDirectiveUtilMathFunctionLibrary::GetIntArrayMedian(const TArray<int32>& Values)
{
	if (Values.IsEmpty())
	{
		return 0.0f;
	}

	TArray<int32> WorkingValues = Values;
	return static_cast<float>(CalculateStatisticsMedian(WorkingValues));
}

float UDirectiveUtilMathFunctionLibrary::GetIntArrayStandardDeviation(const TArray<int32>& Values)
{
	if (Values.IsEmpty())
	{
		return 0.0f;
	}

	const double Mean = static_cast<double>(GetIntArraySum(Values)) / Values.Num();
	double SquaredDeltaSum = 0.0;
	for (const int32 Value : Values)
	{
		const double Delta = static_cast<double>(Value) - Mean;
		SquaredDeltaSum += Delta * Delta;
	}
	return static_cast<float>(FMath::Sqrt(SquaredDeltaSum / Values.Num()));
}

float UDirectiveUtilMathFunctionLibrary::GetFloatArraySum(const TArray<float>& Values)
{
	double Sum = 0.0;
	for (const float Value : Values)
	{
		Sum += static_cast<double>(Value);
	}
	return static_cast<float>(Sum);
}

float UDirectiveUtilMathFunctionLibrary::GetFloatArrayAverage(const TArray<float>& Values)
{
	if (Values.IsEmpty())
	{
		return 0.0f;
	}

	double Sum = 0.0;
	for (const float Value : Values)
	{
		Sum += static_cast<double>(Value);
	}
	return static_cast<float>(Sum / Values.Num());
}

float UDirectiveUtilMathFunctionLibrary::GetFloatArrayMedian(const TArray<float>& Values)
{
	if (Values.IsEmpty())
	{
		return 0.0f;
	}

	TArray<float> WorkingValues = Values;
	if (WorkingValues.ContainsByPredicate([](const float Value) { return FMath::IsNaN(Value); }))
	{
		return std::numeric_limits<float>::quiet_NaN();
	}
	return static_cast<float>(CalculateStatisticsMedian(WorkingValues));
}

float UDirectiveUtilMathFunctionLibrary::GetFloatArrayStandardDeviation(const TArray<float>& Values)
{
	if (Values.IsEmpty())
	{
		return 0.0f;
	}

	double Sum = 0.0;
	for (const float Value : Values)
	{
		Sum += static_cast<double>(Value);
	}
	const double Mean = Sum / Values.Num();

	double SquaredDeltaSum = 0.0;
	for (const float Value : Values)
	{
		const double Delta = static_cast<double>(Value) - Mean;
		SquaredDeltaSum += Delta * Delta;
	}
	return static_cast<float>(FMath::Sqrt(SquaredDeltaSum / Values.Num()));
}

bool UDirectiveUtilMathFunctionLibrary::GetAngleArrayAverage(const TArray<float>& Angles,
	float& AverageAngle, float& ResultantStrength)
{
	AverageAngle = 0.0f;
	ResultantStrength = 0.0f;
	if (Angles.IsEmpty())
	{
		return false;
	}

	double SineSum = 0.0;
	double CosineSum = 0.0;
	for (const float Angle : Angles)
	{
		if (!FMath::IsFinite(Angle))
		{
			return false;
		}

		const double Radians = FMath::DegreesToRadians(FMath::Fmod(static_cast<double>(Angle), 360.0));
		SineSum += FMath::Sin(Radians);
		CosineSum += FMath::Cos(Radians);
	}

	const double Magnitude = FMath::Sqrt(SineSum * SineSum + CosineSum * CosineSum);
	ResultantStrength = static_cast<float>(FMath::Clamp(Magnitude / Angles.Num(), 0.0, 1.0));
	if (ResultantStrength <= UE_DOUBLE_SMALL_NUMBER)
	{
		ResultantStrength = 0.0f;
		return false;
	}

	AverageAngle = static_cast<float>(FMath::RadiansToDegrees(FMath::Atan2(SineSum, CosineSum)));
	return true;
}

bool UDirectiveUtilMathFunctionLibrary::GetWeightedFloatArrayAverage(const TArray<float>& Values,
	const TArray<float>& Weights, float& Average)
{
	Average = 0.0f;
	if (Values.IsEmpty() || Values.Num() != Weights.Num())
	{
		return false;
	}

	double WeightedSum = 0.0;
	double WeightSum = 0.0;
	for (int32 Index = 0; Index < Values.Num(); ++Index)
	{
		if (!FMath::IsFinite(Values[Index]))
		{
			return false;
		}

		const double Weight = GetUsableStatisticsWeight(Weights[Index]);
		WeightedSum += static_cast<double>(Values[Index]) * Weight;
		WeightSum += Weight;
	}

	if (WeightSum <= 0.0)
	{
		return false;
	}

	Average = static_cast<float>(WeightedSum / WeightSum);
	return FMath::IsFinite(Average);
}

bool UDirectiveUtilMathFunctionLibrary::GetWeightedVectorArrayAverage(const TArray<FVector>& Values,
	const TArray<float>& Weights, FVector& Average)
{
	Average = FVector::ZeroVector;
	if (Values.IsEmpty() || Values.Num() != Weights.Num())
	{
		return false;
	}

	FVector RunningAverage = FVector::ZeroVector;
	double WeightSum = 0.0;
	for (int32 Index = 0; Index < Values.Num(); ++Index)
	{
		if (Values[Index].ContainsNaN())
		{
			return false;
		}

		const double Weight = GetUsableStatisticsWeight(Weights[Index]);
		if (Weight > 0.0)
		{
			const double NewWeightSum = WeightSum + Weight;
			RunningAverage = FMath::LerpStable(RunningAverage, Values[Index], Weight / NewWeightSum);
			WeightSum = NewWeightSum;
		}
	}

	if (WeightSum <= 0.0 || RunningAverage.ContainsNaN())
	{
		return false;
	}

	Average = RunningAverage;
	return true;
}

bool UDirectiveUtilMathFunctionLibrary::NormalizeFloatArrayToRange(const TArray<float>& Values,
	const float OutputMinimum, const float OutputMaximum, TArray<float>& NormalizedValues)
{
	TArray<float> ValuesCopy;
	const TArray<float>* SourceValues = &Values;
	if (&Values == &NormalizedValues)
	{
		ValuesCopy = Values;
		SourceValues = &ValuesCopy;
	}

	NormalizedValues.Reset();
	if (SourceValues->IsEmpty() || !FMath::IsFinite(OutputMinimum) || !FMath::IsFinite(OutputMaximum))
	{
		return false;
	}

	float InputMinimum = (*SourceValues)[0];
	float InputMaximum = (*SourceValues)[0];
	for (const float Value : *SourceValues)
	{
		if (!FMath::IsFinite(Value))
		{
			return false;
		}
		InputMinimum = FMath::Min(InputMinimum, Value);
		InputMaximum = FMath::Max(InputMaximum, Value);
	}

	NormalizedValues.SetNumUninitialized(SourceValues->Num());
	if (InputMinimum == InputMaximum)
	{
		NormalizedValues.Init(OutputMinimum, SourceValues->Num());
		return true;
	}

	const double Scale = (static_cast<double>(OutputMaximum) - OutputMinimum)
		/ (static_cast<double>(InputMaximum) - InputMinimum);
	for (int32 Index = 0; Index < SourceValues->Num(); ++Index)
	{
		NormalizedValues[Index] = static_cast<float>(OutputMinimum
			+ (static_cast<double>((*SourceValues)[Index]) - InputMinimum) * Scale);
	}
	return true;
}

bool UDirectiveUtilMathFunctionLibrary::NormalizeWeights(const TArray<float>& Weights,
	TArray<float>& NormalizedWeights)
{
	TArray<float> WeightsCopy;
	const TArray<float>* SourceWeights = &Weights;
	if (&Weights == &NormalizedWeights)
	{
		WeightsCopy = Weights;
		SourceWeights = &WeightsCopy;
	}

	NormalizedWeights.Reset();
	if (SourceWeights->IsEmpty())
	{
		return false;
	}

	double WeightSum = 0.0;
	for (const float Weight : *SourceWeights)
	{
		WeightSum += GetUsableStatisticsWeight(Weight);
	}
	if (WeightSum <= 0.0)
	{
		return false;
	}

	NormalizedWeights.SetNumUninitialized(SourceWeights->Num());
	for (int32 Index = 0; Index < SourceWeights->Num(); ++Index)
	{
		NormalizedWeights[Index] = static_cast<float>(GetUsableStatisticsWeight((*SourceWeights)[Index]) / WeightSum);
	}
	return true;
}

bool UDirectiveUtilMathFunctionLibrary::GetFloatArrayPercentile(const TArray<float>& Values,
	const float Percentile, float& Value)
{
	Value = 0.0f;
	if (Values.IsEmpty() || !FMath::IsFinite(Percentile))
	{
		return false;
	}

	for (const float Candidate : Values)
	{
		if (!FMath::IsFinite(Candidate))
		{
			return false;
		}
	}
	TArray<float> WorkingValues = Values;

	const double Position = FMath::Clamp(static_cast<double>(Percentile), 0.0, 100.0)
		* 0.01 * (WorkingValues.Num() - 1);
	const int32 LowerIndex = FMath::FloorToInt(Position);
	const int32 UpperIndex = FMath::CeilToInt(Position);
	const float LowerValue = SelectStatisticsNth(WorkingValues, LowerIndex);
	if (LowerIndex == UpperIndex)
	{
		Value = LowerValue;
		return true;
	}
	const float UpperValue = SelectStatisticsNth(WorkingValues, UpperIndex);
	Value = static_cast<float>(FMath::Lerp(
		static_cast<double>(LowerValue),
		static_cast<double>(UpperValue),
		Position - LowerIndex));
	return true;
}

bool UDirectiveUtilMathFunctionLibrary::GetFloatArrayRootMeanSquare(const TArray<float>& Values,
	float& RootMeanSquare)
{
	RootMeanSquare = 0.0f;
	if (Values.IsEmpty())
	{
		return false;
	}

	double SquaredSum = 0.0;
	for (const float Value : Values)
	{
		if (!FMath::IsFinite(Value))
		{
			return false;
		}
		SquaredSum += static_cast<double>(Value) * Value;
	}

	RootMeanSquare = static_cast<float>(FMath::Sqrt(SquaredSum / Values.Num()));
	return FMath::IsFinite(RootMeanSquare);
}
