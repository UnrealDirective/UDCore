// Copyright (c) 2026 Unreal Directive. Licensed under the MIT License.

#include "DirectiveUtilitiesRuntimeShippingSmoke.h"

#include "DirectiveUtilitiesRuntimeBenchmarkObject.h"
#include "HAL/FileManager.h"
#include "Libraries/DirectiveUtilArrayFunctionLibrary.h"
#include "Libraries/DirectiveUtilMathFunctionLibrary.h"
#include "Libraries/DirectiveUtilStringFunctionLibrary.h"
#include "Misc/CommandLine.h"
#include "Misc/FileHelper.h"
#include "Misc/Parse.h"
#include "Misc/Paths.h"
#include "UObject/UnrealType.h"

namespace
{
	struct FSmokeCheck
	{
		FString Name;
		bool bPassed;
	};

	void AddCheck(TArray<FSmokeCheck>& Checks, const TCHAR* Name, const bool bPassed)
	{
		Checks.Add({Name, bPassed});
	}
}

bool RunDirectiveUtilitiesRuntimeShippingSmoke(const FString& OutputPath)
{
	TArray<FSmokeCheck> Checks;
	const FArrayProperty* IntegerArrayProperty = FindFProperty<FArrayProperty>(
		UDirectiveUtilitiesRuntimeBenchmarkObject::StaticClass(),
		GET_MEMBER_NAME_CHECKED(UDirectiveUtilitiesRuntimeBenchmarkObject, IntegerValues));
	AddCheck(Checks, TEXT("array_property"), IntegerArrayProperty != nullptr);

	if (IntegerArrayProperty)
	{
		TArray<int32> Target{1, 2};
		const TArray<int32> Source{3, 4};
		UDirectiveUtilArrayFunctionLibrary::GenericArray_AppendOptimized(
			&Target, IntegerArrayProperty, &Source, IntegerArrayProperty);
		AddCheck(Checks, TEXT("array_append"), Target == TArray<int32>({1, 2, 3, 4}));

		const TArray<int32> Inserted{8, 9};
		const bool bInserted = UDirectiveUtilArrayFunctionLibrary::GenericArray_InsertOptimized(
			&Target, IntegerArrayProperty, &Inserted, IntegerArrayProperty, 1);
		AddCheck(Checks, TEXT("array_insert"), bInserted && Target == TArray<int32>({1, 8, 9, 2, 3, 4}));

		const int32 ItemToRemove = 9;
		const bool bRemoved = UDirectiveUtilArrayFunctionLibrary::GenericArray_RemoveAllOccurrences(
			&Target, IntegerArrayProperty, &ItemToRemove);
		AddCheck(Checks, TEXT("array_remove"), bRemoved && Target == TArray<int32>({1, 8, 2, 3, 4}));

		const TArray<int32> Grouped{4, 2, 4, 3, 2, 4};
		TArray<int32> Distinct;
		UDirectiveUtilArrayFunctionLibrary::GenericArray_GetDistinct(
			&Grouped, IntegerArrayProperty, &Distinct, IntegerArrayProperty);
		AddCheck(Checks, TEXT("array_distinct"), Distinct == TArray<int32>({4, 2, 3}));

		int32 MostCommon = 0;
		int32 MostCommonCount = 0;
		const bool bFoundMostCommon = UDirectiveUtilArrayFunctionLibrary::GenericArray_GetMostCommon(
			&Grouped, IntegerArrayProperty, &MostCommon, &MostCommonCount);
		AddCheck(Checks, TEXT("array_most_common"), bFoundMostCommon && MostCommon == 4 && MostCommonCount == 3);
	}

	AddCheck(Checks, TEXT("math_delta_angle"), FMath::IsNearlyEqual(
		UDirectiveUtilMathFunctionLibrary::DeltaAngle(350.0f, 10.0f), 20.0f));
	AddCheck(Checks, TEXT("math_statistics"), FMath::IsNearlyEqual(
		UDirectiveUtilMathFunctionLibrary::GetFloatArrayAverage({2.0f, 4.0f, 6.0f}), 4.0f));
	float RootMeanSquare = 0.0f;
	AddCheck(Checks, TEXT("math_root_mean_square"),
		UDirectiveUtilMathFunctionLibrary::GetFloatArrayRootMeanSquare({3.0f, 4.0f}, RootMeanSquare)
		&& FMath::IsNearlyEqual(RootMeanSquare, FMath::Sqrt(12.5f)));
	AddCheck(Checks, TEXT("math_grid"),
		UDirectiveUtilMathFunctionLibrary::GenerateGridPoints2D(
			FVector::ZeroVector, FRotator::ZeroRotator, FIntPoint(3, 2), FVector2D(100.0, 50.0)).Num() == 6);
	AddCheck(Checks, TEXT("math_hex"),
		UDirectiveUtilMathFunctionLibrary::GetHexNeighbors(FIntPoint::ZeroValue).Num() == 6);
	AddCheck(Checks, TEXT("math_sphere"),
		UDirectiveUtilMathFunctionLibrary::GeneratePointsOnSphere(
			FVector::ZeroVector, FRotator::ZeroRotator, 100.0, 32).Num() == 32);
	AddCheck(Checks, TEXT("math_ease"), FMath::IsNearlyEqual(
		UDirectiveUtilMathFunctionLibrary::EaseFloat(5.0f, 10.0f, 0.0f, EDirectiveUtilEaseType::BounceOut), 5.0f));

	AddCheck(Checks, TEXT("string_classification"),
		UDirectiveUtilStringFunctionLibrary::ContainsLetters(TEXT("abc123"))
		&& UDirectiveUtilStringFunctionLibrary::ContainsNumbers(TEXT("abc123"))
		&& !UDirectiveUtilStringFunctionLibrary::ContainsSpaces(TEXT("abc123")));
	AddCheck(Checks, TEXT("string_case"),
		UDirectiveUtilStringFunctionLibrary::ToCamelCase(TEXT("shipping smoke test")) == TEXT("shippingSmokeTest"));

	FString Revision;
	FParse::Value(FCommandLine::Get(), TEXT("DirectiveUtilitiesPerfRevision="), Revision);
	TArray<FString> Lines{
		TEXT("#configuration,Shipping"),
		FString::Printf(TEXT("#revision,%s"), *Revision),
		TEXT("check,passed")
	};
	bool bPassed = true;
	for (const FSmokeCheck& Check : Checks)
	{
		Lines.Add(FString::Printf(TEXT("%s,%s"), *Check.Name, Check.bPassed ? TEXT("true") : TEXT("false")));
		bPassed = bPassed && Check.bPassed;
	}

	IFileManager::Get().MakeDirectory(*FPaths::GetPath(OutputPath), true);
	const bool bSaved = FFileHelper::SaveStringArrayToFile(Lines, *OutputPath);
	if (!bPassed || !bSaved)
	{
		UE_LOG(LogTemp, Error, TEXT("SHIPPING_SMOKE_FAILED output=%s saved=%s"),
			*OutputPath, bSaved ? TEXT("true") : TEXT("false"));
	}
	return bPassed && bSaved;
}
