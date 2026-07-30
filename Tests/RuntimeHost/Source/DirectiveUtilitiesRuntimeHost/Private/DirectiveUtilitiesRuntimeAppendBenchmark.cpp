// Copyright (c) 2026 Unreal Directive. Licensed under the MIT License.

#include "DirectiveUtilitiesRuntimeAppendBenchmark.h"

#include "DirectiveUtilitiesRuntimeBenchmarkObject.h"
#include "HAL/FileManager.h"
#include "HAL/PlatformMisc.h"
#include "HAL/PlatformProperties.h"
#include "HAL/PlatformTime.h"
#include "Kismet/KismetArrayLibrary.h"
#include "Libraries/DirectiveUtilArrayFunctionLibrary.h"
#include "Misc/CommandLine.h"
#include "Misc/DateTime.h"
#include "Misc/EngineVersion.h"
#include "Misc/FileHelper.h"
#include "Misc/Parse.h"
#include "Misc/Paths.h"
#include "UObject/UnrealType.h"

namespace
{
	struct FAppendBenchmarkResult
	{
		FString ElementType;
		FString CopyPath;
		FString Scenario;
		int32 SourceCount = 0;
		int32 InitialTargetCount = 0;
		double BeforeMedianMilliseconds = 0.0;
		double BeforeMinimumMilliseconds = 0.0;
		double BeforeMaximumMilliseconds = 0.0;
		double AfterMedianMilliseconds = 0.0;
		double AfterMinimumMilliseconds = 0.0;
		double AfterMaximumMilliseconds = 0.0;
		int32 SampleCount = 0;
		int32 IterationsPerSample = 0;
		bool bOutputsMatch = true;
	};

	TArray<int32> MakeSequentialIntegers(const int32 Count)
	{
		TArray<int32> Values;
		Values.SetNumUninitialized(Count);
		for (int32 Index = 0; Index < Count; ++Index)
		{
			Values[Index] = Index;
		}
		return Values;
	}

	TArray<bool> MakeBooleans(const int32 Count)
	{
		TArray<bool> Values;
		Values.SetNumUninitialized(Count);
		for (int32 Index = 0; Index < Count; ++Index)
		{
			Values[Index] = Index % 2 == 0;
		}
		return Values;
	}

	TArray<float> MakeFloats(const int32 Count)
	{
		TArray<float> Values;
		Values.SetNumUninitialized(Count);
		for (int32 Index = 0; Index < Count; ++Index)
		{
			Values[Index] = static_cast<float>(Index) * 0.5f;
		}
		return Values;
	}

	TArray<FVector> MakeVectors(const int32 Count)
	{
		TArray<FVector> Values;
		Values.SetNumUninitialized(Count);
		for (int32 Index = 0; Index < Count; ++Index)
		{
			Values[Index] = FVector(Index, Index + 1, Index + 2);
		}
		return Values;
	}

	TArray<FString> MakeStrings(const int32 Count)
	{
		TArray<FString> Values;
		Values.Reserve(Count);
		for (int32 Index = 0; Index < Count; ++Index)
		{
			Values.Add(FString::Printf(TEXT("Value%07d"), Index));
		}
		return Values;
	}

	TArray<TObjectPtr<UObject>> MakeObjectReferences(
		const int32 Count,
		const TArray<TObjectPtr<UObject>>& ObjectPool)
	{
		TArray<TObjectPtr<UObject>> Values;
		Values.SetNumUninitialized(Count);
		for (int32 Index = 0; Index < Count; ++Index)
		{
			Values[Index] = ObjectPool[Index % ObjectPool.Num()];
		}
		return Values;
	}

	int32 GetIterationsPerSample(const int32 ElementCount)
	{
		if (ElementCount <= 1000)
		{
			return 256;
		}
		if (ElementCount <= 10000)
		{
			return 64;
		}
		if (ElementCount <= 100000)
		{
			return 8;
		}
		return 1;
	}

	template<typename ValueType>
	FAppendBenchmarkResult MeasureAppend(
		const FString& ElementType,
		const FString& Scenario,
		const TArray<ValueType>& InitialTarget,
		const TArray<ValueType>& Source,
		const int32 SampleCount,
		const FArrayProperty* ArrayProperty)
	{
		const int32 IterationsPerSample = GetIterationsPerSample(Source.Num());
		TArray<TArray<ValueType>> BeforeTargets;
		TArray<TArray<ValueType>> AfterTargets;
		TArray<ValueType> BeforeSource;
		TArray<ValueType> AfterSource;
		auto Prepare = [&]()
		{
			BeforeTargets.SetNum(IterationsPerSample);
			AfterTargets.SetNum(IterationsPerSample);
			for (int32 Index = 0; Index < IterationsPerSample; ++Index)
			{
				BeforeTargets[Index] = InitialTarget;
				AfterTargets[Index] = InitialTarget;
			}
			BeforeSource = Source;
			AfterSource = Source;
		};
		auto RunBefore = [&]()
		{
			for (TArray<ValueType>& Target : BeforeTargets)
			{
				UKismetArrayLibrary::GenericArray_Append(
					&Target,
					ArrayProperty,
					&BeforeSource,
					ArrayProperty);
			}
		};
		auto RunAfter = [&]()
		{
			for (TArray<ValueType>& Target : AfterTargets)
			{
				UDirectiveUtilArrayFunctionLibrary::GenericArray_AppendOptimized(
					&Target,
					ArrayProperty,
					&AfterSource,
					ArrayProperty);
			}
		};
		auto TimeOperation = [IterationsPerSample](auto&& Operation)
		{
			const uint64 StartCycles = FPlatformTime::Cycles64();
			Operation();
			return FPlatformTime::ToMilliseconds64(FPlatformTime::Cycles64() - StartCycles)
				/ IterationsPerSample;
		};

		Prepare();
		RunBefore();
		RunAfter();
		bool bOutputsMatch = BeforeTargets == AfterTargets
			&& BeforeSource == Source
			&& AfterSource == Source;

		TArray<double> BeforeSamples;
		TArray<double> AfterSamples;
		BeforeSamples.Reserve(SampleCount);
		AfterSamples.Reserve(SampleCount);
		for (int32 SampleIndex = 0; SampleIndex < SampleCount; ++SampleIndex)
		{
			Prepare();
			if (SampleIndex % 2 == 0)
			{
				BeforeSamples.Add(TimeOperation(RunBefore));
				AfterSamples.Add(TimeOperation(RunAfter));
			}
			else
			{
				AfterSamples.Add(TimeOperation(RunAfter));
				BeforeSamples.Add(TimeOperation(RunBefore));
			}
			bOutputsMatch = bOutputsMatch
				&& BeforeTargets == AfterTargets
				&& BeforeSource == Source
				&& AfterSource == Source;
		}

		BeforeSamples.Sort();
		AfterSamples.Sort();
		FAppendBenchmarkResult Result;
		Result.ElementType = ElementType;
		Result.CopyPath = ArrayProperty->Inner->HasAnyPropertyFlags(CPF_IsPlainOldData)
			? TEXT("bulk")
			: TEXT("property");
		Result.Scenario = Scenario;
		Result.SourceCount = Source.Num();
		Result.InitialTargetCount = InitialTarget.Num();
		Result.BeforeMedianMilliseconds = BeforeSamples[BeforeSamples.Num() / 2];
		Result.BeforeMinimumMilliseconds = BeforeSamples[0];
		Result.BeforeMaximumMilliseconds = BeforeSamples.Last();
		Result.AfterMedianMilliseconds = AfterSamples[AfterSamples.Num() / 2];
		Result.AfterMinimumMilliseconds = AfterSamples[0];
		Result.AfterMaximumMilliseconds = AfterSamples.Last();
		Result.SampleCount = SampleCount;
		Result.IterationsPerSample = IterationsPerSample;
		Result.bOutputsMatch = bOutputsMatch;
		return Result;
	}

	template<typename ValueType, typename MakeValuesType>
	void AddTypeBenchmarks(
		const FString& ElementType,
		const FArrayProperty* ArrayProperty,
		MakeValuesType&& MakeValues,
		const int32 SampleCount,
		TArray<FAppendBenchmarkResult>& Results)
	{
		for (const int32 ElementCount : {1000, 10000, 100000, 250000, 1000000})
		{
			const TArray<ValueType> Source = MakeValues(ElementCount);
			Results.Add(MeasureAppend(
				ElementType,
				TEXT("empty_target"),
				TArray<ValueType>(),
				Source,
				SampleCount,
				ArrayProperty));
			Results.Add(MeasureAppend(
				ElementType,
				TEXT("populated_target"),
				MakeValues(ElementCount),
				Source,
				SampleCount,
				ArrayProperty));
		}
	}

	FString SanitizeMetadata(FString Value)
	{
		Value.ReplaceInline(TEXT(","), TEXT(";"));
		Value.ReplaceInline(TEXT("\r"), TEXT(" "));
		Value.ReplaceInline(TEXT("\n"), TEXT(" "));
		return Value;
	}
}

bool RunDirectiveUtilitiesRuntimeAppendBenchmark(const FString& OutputPath)
{
	const FArrayProperty* BoolArrayProperty = FindFProperty<FArrayProperty>(
		UDirectiveUtilitiesRuntimeBenchmarkObject::StaticClass(),
		GET_MEMBER_NAME_CHECKED(UDirectiveUtilitiesRuntimeBenchmarkObject, BoolValues));
	const FArrayProperty* IntegerArrayProperty = FindFProperty<FArrayProperty>(
		UDirectiveUtilitiesRuntimeBenchmarkObject::StaticClass(),
		GET_MEMBER_NAME_CHECKED(UDirectiveUtilitiesRuntimeBenchmarkObject, IntegerValues));
	const FArrayProperty* FloatArrayProperty = FindFProperty<FArrayProperty>(
		UDirectiveUtilitiesRuntimeBenchmarkObject::StaticClass(),
		GET_MEMBER_NAME_CHECKED(UDirectiveUtilitiesRuntimeBenchmarkObject, FloatValues));
	const FArrayProperty* VectorArrayProperty = FindFProperty<FArrayProperty>(
		UDirectiveUtilitiesRuntimeBenchmarkObject::StaticClass(),
		GET_MEMBER_NAME_CHECKED(UDirectiveUtilitiesRuntimeBenchmarkObject, VectorValues));
	const FArrayProperty* StringArrayProperty = FindFProperty<FArrayProperty>(
		UDirectiveUtilitiesRuntimeBenchmarkObject::StaticClass(),
		GET_MEMBER_NAME_CHECKED(UDirectiveUtilitiesRuntimeBenchmarkObject, StringValues));
	const FArrayProperty* ObjectArrayProperty = FindFProperty<FArrayProperty>(
		UDirectiveUtilitiesRuntimeBenchmarkObject::StaticClass(),
		GET_MEMBER_NAME_CHECKED(UDirectiveUtilitiesRuntimeBenchmarkObject, ObjectValues));
	if (!BoolArrayProperty
		|| !IntegerArrayProperty
		|| !FloatArrayProperty
		|| !VectorArrayProperty
		|| !StringArrayProperty
		|| !ObjectArrayProperty)
	{
		UE_LOG(LogTemp, Error, TEXT("APPEND_SHIPPING_BENCHMARK_FAILED array property unavailable"));
		return false;
	}

	constexpr int32 SampleCount = 7;
	TArray<FAppendBenchmarkResult> Results;
	AddTypeBenchmarks<bool>(
		TEXT("bool"),
		BoolArrayProperty,
		MakeBooleans,
		SampleCount,
		Results);
	AddTypeBenchmarks<int32>(
		TEXT("int32"),
		IntegerArrayProperty,
		MakeSequentialIntegers,
		SampleCount,
		Results);
	AddTypeBenchmarks<float>(
		TEXT("float"),
		FloatArrayProperty,
		MakeFloats,
		SampleCount,
		Results);
	AddTypeBenchmarks<FVector>(
		TEXT("FVector"),
		VectorArrayProperty,
		MakeVectors,
		SampleCount,
		Results);
	AddTypeBenchmarks<FString>(
		TEXT("FString"),
		StringArrayProperty,
		MakeStrings,
		SampleCount,
		Results);

	UDirectiveUtilitiesRuntimeBenchmarkObject* ObjectOwner =
		NewObject<UDirectiveUtilitiesRuntimeBenchmarkObject>();
	ObjectOwner->AddToRoot();
	TArray<TObjectPtr<UObject>> ObjectPool;
	ObjectPool.Reserve(16);
	for (int32 Index = 0; Index < 16; ++Index)
	{
		ObjectPool.Add(NewObject<UObject>(ObjectOwner));
	}
	AddTypeBenchmarks<TObjectPtr<UObject>>(
		TEXT("UObject"),
		ObjectArrayProperty,
		[&ObjectPool](const int32 Count)
		{
			return MakeObjectReferences(Count, ObjectPool);
		},
		SampleCount,
		Results);
	ObjectOwner->RemoveFromRoot();

	for (const FAppendBenchmarkResult& Result : Results)
	{
		if (!Result.bOutputsMatch)
		{
			UE_LOG(
				LogTemp,
				Error,
				TEXT("APPEND_SHIPPING_BENCHMARK_FAILED output mismatch source=%d scenario=%s"),
				Result.SourceCount,
				*Result.Scenario);
			return false;
		}
	}

	FString Revision;
	FParse::Value(FCommandLine::Get(), TEXT("DirectiveUtilitiesPerfRevision="), Revision);
	FString Csv;
	Csv += FString::Printf(TEXT("#engine,%s\n"), *SanitizeMetadata(FEngineVersion::Current().ToString()));
	Csv += FString::Printf(TEXT("#platform,%s\n"), *SanitizeMetadata(FPlatformProperties::IniPlatformName()));
	Csv += FString::Printf(TEXT("#cpu,%s\n"), *SanitizeMetadata(FPlatformMisc::GetCPUBrand()));
	Csv += TEXT("#configuration,Shipping\n");
	Csv += FString::Printf(TEXT("#timestamp_utc,%s\n"), *FDateTime::UtcNow().ToIso8601());
	Csv += FString::Printf(TEXT("#revision,%s\n"), *SanitizeMetadata(Revision));
	Csv += TEXT("element_type,copy_path,scenario,source_count,initial_target_count,before_median_ms,before_min_ms,before_max_ms,after_median_ms,after_min_ms,after_max_ms,samples,iterations_per_sample,speedup,time_reduction_percent\n");
	for (const FAppendBenchmarkResult& Result : Results)
	{
		const double Speedup = Result.AfterMedianMilliseconds > 0.0
			? Result.BeforeMedianMilliseconds / Result.AfterMedianMilliseconds
			: 0.0;
		const double TimeReductionPercent = Result.BeforeMedianMilliseconds > 0.0
			? ((Result.BeforeMedianMilliseconds - Result.AfterMedianMilliseconds) / Result.BeforeMedianMilliseconds) * 100.0
			: 0.0;
		Csv += FString::Printf(
			TEXT("%s,%s,%s,%d,%d,%.9f,%.9f,%.9f,%.9f,%.9f,%.9f,%d,%d,%.4f,%.2f\n"),
			*Result.ElementType,
			*Result.CopyPath,
			*Result.Scenario,
			Result.SourceCount,
			Result.InitialTargetCount,
			Result.BeforeMedianMilliseconds,
			Result.BeforeMinimumMilliseconds,
			Result.BeforeMaximumMilliseconds,
			Result.AfterMedianMilliseconds,
			Result.AfterMinimumMilliseconds,
			Result.AfterMaximumMilliseconds,
			Result.SampleCount,
			Result.IterationsPerSample,
			Speedup,
			TimeReductionPercent);
	}

	const FString AbsoluteOutputPath = FPaths::ConvertRelativePathToFull(OutputPath);
	IFileManager::Get().MakeDirectory(*FPaths::GetPath(AbsoluteOutputPath), true);
	if (!FFileHelper::SaveStringToFile(Csv, *AbsoluteOutputPath))
	{
		UE_LOG(
			LogTemp,
			Error,
			TEXT("APPEND_SHIPPING_BENCHMARK_FAILED unable to save %s"),
			*AbsoluteOutputPath);
		return false;
	}

	UE_LOG(
		LogTemp,
		Display,
		TEXT("APPEND_SHIPPING_BENCHMARK_COMPLETE output=%s"),
		*AbsoluteOutputPath);
	return true;
}
