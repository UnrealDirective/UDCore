// Copyright (c) 2026 Unreal Directive. Licensed under the MIT License.

#include "Libraries/DirectiveUtilArrayFunctionLibrary.h"
#include "Libraries/DirectiveUtilMathFunctionLibrary.h"
#include "Libraries/DirectiveUtilStringFunctionLibrary.h"
#include "Tests/DirectiveUtilTestObject.h"

#include "Algo/Reverse.h"
#include "HAL/FileManager.h"
#include "HAL/PlatformMisc.h"
#include "HAL/PlatformProperties.h"
#include "HAL/PlatformTime.h"
#include "Interfaces/IPluginManager.h"
#include "Kismet/KismetArrayLibrary.h"
#include "Misc/AutomationTest.h"
#include "Misc/CommandLine.h"
#include "Misc/DateTime.h"
#include "Misc/EngineVersion.h"
#include "Misc/FileHelper.h"
#include "Misc/Parse.h"
#include "Misc/Paths.h"
#include "UObject/UnrealType.h"

namespace DirectiveUtilRuntimePerformance
{
	struct FResult
	{
		FString Name;
		int32 ElementCount = 0;
		int32 Parameter = 0;
		double MedianMilliseconds = 0.0;
		double MinimumMilliseconds = 0.0;
		double MaximumMilliseconds = 0.0;
		int32 SampleCount = 0;

		FString GetKey() const
		{
			return FString::Printf(TEXT("%s|%d|%d"), *Name, ElementCount, Parameter);
		}
	};

	struct FComparisonResult
	{
		FString ElementType;
		FString Pattern;
		int32 ElementCount = 0;
		int32 MatchCount = 0;
		double BeforeMedianMilliseconds = 0.0;
		double BeforeMinimumMilliseconds = 0.0;
		double BeforeMaximumMilliseconds = 0.0;
		double AfterMedianMilliseconds = 0.0;
		double AfterMinimumMilliseconds = 0.0;
		double AfterMaximumMilliseconds = 0.0;
		int32 SampleCount = 0;
		bool bOutputsMatch = true;
	};

	struct FAppendComparisonResult
	{
		FString ElementType;
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
		bool bOutputsMatch = true;
	};

	template <typename PrepareType, typename OperationType>
	FResult Measure(
		const FString& Name,
		const int32 ElementCount,
		const int32 Parameter,
		const int32 SampleCount,
		PrepareType&& Prepare,
		OperationType&& Operation)
	{
		Prepare();
		Operation();
		const double WarmupStartSeconds = FPlatformTime::Seconds();
		do
		{
			Prepare();
			Operation();
		}
		while (FPlatformTime::Seconds() - WarmupStartSeconds < 0.01);

		TArray<double> Samples;
		Samples.Reserve(SampleCount);
		for (int32 SampleIndex = 0; SampleIndex < SampleCount; ++SampleIndex)
		{
			Prepare();
			const uint64 StartCycles = FPlatformTime::Cycles64();
			Operation();
			const uint64 ElapsedCycles = FPlatformTime::Cycles64() - StartCycles;
			Samples.Add(FPlatformTime::ToMilliseconds64(ElapsedCycles));
		}

		Samples.Sort();
		FResult Result;
		Result.Name = Name;
		Result.ElementCount = ElementCount;
		Result.Parameter = Parameter;
		Result.MedianMilliseconds = Samples[Samples.Num() / 2];
		Result.MinimumMilliseconds = Samples[0];
		Result.MaximumMilliseconds = Samples.Last();
		Result.SampleCount = SampleCount;
		return Result;
	}

	template <typename ArrayType, typename BeforeOperationType, typename AfterOperationType>
	FAppendComparisonResult MeasureAppendComparison(
		const FString& ElementType,
		const FString& Scenario,
		const ArrayType& InitialTarget,
		const ArrayType& Source,
		const int32 SampleCount,
		BeforeOperationType&& BeforeOperation,
		AfterOperationType&& AfterOperation)
	{
		ArrayType BeforeTarget;
		ArrayType AfterTarget;
		ArrayType BeforeSource;
		ArrayType AfterSource;
		auto Prepare = [&]()
		{
			BeforeTarget = InitialTarget;
			AfterTarget = InitialTarget;
			BeforeSource = Source;
			AfterSource = Source;
		};
		auto RunBefore = [&]()
		{
			BeforeOperation(BeforeTarget, BeforeSource);
		};
		auto RunAfter = [&]()
		{
			AfterOperation(AfterTarget, AfterSource);
		};

		Prepare();
		RunBefore();
		RunAfter();
		bool bOutputsMatch = BeforeTarget == AfterTarget
			&& BeforeSource == Source
			&& AfterSource == Source;

		TArray<double> BeforeSamples;
		TArray<double> AfterSamples;
		BeforeSamples.Reserve(SampleCount);
		AfterSamples.Reserve(SampleCount);
		auto TimeOperation = [](auto&& Operation)
		{
			const uint64 StartCycles = FPlatformTime::Cycles64();
			Operation();
			return FPlatformTime::ToMilliseconds64(FPlatformTime::Cycles64() - StartCycles);
		};

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
				&& BeforeTarget == AfterTarget
				&& BeforeSource == Source
				&& AfterSource == Source;
		}

		BeforeSamples.Sort();
		AfterSamples.Sort();
		FAppendComparisonResult Result;
		Result.ElementType = ElementType;
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
		Result.bOutputsMatch = bOutputsMatch;
		return Result;
	}

	template <typename ArrayType, typename BeforeOperationType, typename AfterOperationType>
	FComparisonResult MeasureComparison(
		const FString& ElementType,
		const FString& Pattern,
		const ArrayType& Source,
		const int32 MatchCount,
		const int32 SampleCount,
		BeforeOperationType&& BeforeOperation,
		AfterOperationType&& AfterOperation)
	{
		ArrayType BeforeValues = Source;
		ArrayType AfterValues = Source;
		bool bBeforeRemoved = BeforeOperation(BeforeValues);
		bool bAfterRemoved = AfterOperation(AfterValues);

		TArray<double> BeforeSamples;
		TArray<double> AfterSamples;
		BeforeSamples.Reserve(SampleCount);
		AfterSamples.Reserve(SampleCount);

		bool bOutputsMatch = bBeforeRemoved == bAfterRemoved && BeforeValues == AfterValues;
		auto TimeOperation = [](auto&& Operation)
		{
			const uint64 StartCycles = FPlatformTime::Cycles64();
			const bool bRemoved = Operation();
			const uint64 ElapsedCycles = FPlatformTime::Cycles64() - StartCycles;
			return TPair<double, bool>(FPlatformTime::ToMilliseconds64(ElapsedCycles), bRemoved);
		};

		for (int32 SampleIndex = 0; SampleIndex < SampleCount; ++SampleIndex)
		{
			BeforeValues = Source;
			AfterValues = Source;

			TPair<double, bool> BeforeTiming;
			TPair<double, bool> AfterTiming;
			if (SampleIndex % 2 == 0)
			{
				BeforeTiming = TimeOperation([&]() { return BeforeOperation(BeforeValues); });
				AfterTiming = TimeOperation([&]() { return AfterOperation(AfterValues); });
			}
			else
			{
				AfterTiming = TimeOperation([&]() { return AfterOperation(AfterValues); });
				BeforeTiming = TimeOperation([&]() { return BeforeOperation(BeforeValues); });
			}

			BeforeSamples.Add(BeforeTiming.Key);
			AfterSamples.Add(AfterTiming.Key);
			bOutputsMatch = bOutputsMatch
				&& BeforeTiming.Value == AfterTiming.Value
				&& BeforeValues == AfterValues;
		}

		BeforeSamples.Sort();
		AfterSamples.Sort();
		FComparisonResult Result;
		Result.ElementType = ElementType;
		Result.Pattern = Pattern;
		Result.ElementCount = Source.Num();
		Result.MatchCount = MatchCount;
		Result.BeforeMedianMilliseconds = BeforeSamples[BeforeSamples.Num() / 2];
		Result.BeforeMinimumMilliseconds = BeforeSamples[0];
		Result.BeforeMaximumMilliseconds = BeforeSamples.Last();
		Result.AfterMedianMilliseconds = AfterSamples[AfterSamples.Num() / 2];
		Result.AfterMinimumMilliseconds = AfterSamples[0];
		Result.AfterMaximumMilliseconds = AfterSamples.Last();
		Result.SampleCount = SampleCount;
		Result.bOutputsMatch = bOutputsMatch;
		return Result;
	}

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

	TArray<int32> MakeShuffledIndices(const int32 Count)
	{
		TArray<int32> Indices = MakeSequentialIntegers(Count);
		FRandomStream RandomStream(1729);
		for (int32 Index = Count - 1; Index > 0; --Index)
		{
			Indices.Swap(Index, RandomStream.RandRange(0, Index));
		}
		return Indices;
	}

	TArray<int32> MakeRepeatingIntegers(const int32 Count, const int32 DistinctCount)
	{
		TArray<int32> Values;
		Values.Reserve(Count);
		for (int32 Index = 0; Index < Count; ++Index)
		{
			Values.Add(Index % DistinctCount);
		}
		return Values;
	}

	bool IsRemovalMatch(const FString& Pattern, const int32 Index, const int32 Count)
	{
		if (Pattern == TEXT("single_tail"))
		{
			return Index == Count - 1;
		}
		if (Pattern == TEXT("every_64"))
		{
			return Index % 64 == 63;
		}
		if (Pattern == TEXT("clustered"))
		{
			return Index >= Count / 3 && Index < Count * 2 / 3;
		}
		if (Pattern == TEXT("alternating"))
		{
			return Index % 2 == 0;
		}
		return Pattern == TEXT("all");
	}

	TArray<int32> MakeRemovalIntegers(
		const int32 Count,
		const FString& Pattern,
		const int32 ItemToRemove,
		int32& OutMatchCount)
	{
		TArray<int32> Values;
		Values.SetNumUninitialized(Count);
		OutMatchCount = 0;
		for (int32 Index = 0; Index < Count; ++Index)
		{
			const bool bMatches = Pattern != TEXT("no_match") && IsRemovalMatch(Pattern, Index, Count);
			Values[Index] = bMatches ? ItemToRemove : Index + 1;
			OutMatchCount += bMatches ? 1 : 0;
		}
		return Values;
	}

	TArray<FString> MakeRemovalStrings(
		const int32 Count,
		const FString& Pattern,
		const FString& ItemToRemove,
		int32& OutMatchCount)
	{
		TArray<FString> Values;
		Values.Reserve(Count);
		OutMatchCount = 0;
		for (int32 Index = 0; Index < Count; ++Index)
		{
			const bool bMatches = Pattern != TEXT("no_match") && IsRemovalMatch(Pattern, Index, Count);
			Values.Add(bMatches ? ItemToRemove : FString::Printf(TEXT("Value%06d"), Index));
			OutMatchCount += bMatches ? 1 : 0;
		}
		return Values;
	}

	TArray<FString> MakeNaturalSortStrings(const int32 Count)
	{
		const TArray<int32> Indices = MakeShuffledIndices(Count);
		TArray<FString> Values;
		Values.Reserve(Count);
		for (const int32 Index : Indices)
		{
			Values.Add(FString::Printf(TEXT("Item%d"), Index));
		}
		return Values;
	}

	TArray<FName> MakeNaturalSortNames(const int32 Count)
	{
		const TArray<int32> Indices = MakeShuffledIndices(Count);
		TArray<FName> Values;
		Values.Reserve(Count);
		for (const int32 Index : Indices)
		{
			Values.Add(FName(*FString::Printf(TEXT("Actor%d"), Index)));
		}
		return Values;
	}

	TArray<FString> MakeStringMatchCandidates(const int32 Count)
	{
		TArray<FString> Candidates;
		Candidates.Reserve(Count);
		for (int32 Index = 0; Index < Count; ++Index)
		{
			Candidates.Add(FString::Printf(TEXT("DirectiveUtilityCandidate%05d"), Index));
		}
		return Candidates;
	}

	FString GetOutputPath()
	{
		FString OutputPath;
		if (!FParse::Value(FCommandLine::Get(), TEXT("DirectiveUtilitiesPerfOutput="), OutputPath))
		{
			OutputPath = FPaths::ProjectSavedDir() / TEXT("Automation/DirectiveUtilities/RuntimePerformance.csv");
		}
		return FPaths::ConvertRelativePathToFull(OutputPath);
	}

	FString GetComparisonOutputPath()
	{
		FString OutputPath;
		if (!FParse::Value(FCommandLine::Get(), TEXT("DirectiveUtilitiesPerfComparisonOutput="), OutputPath))
		{
			const FString RuntimeOutputPath = GetOutputPath();
			OutputPath = FPaths::GetPath(RuntimeOutputPath)
				/ (FPaths::GetBaseFilename(RuntimeOutputPath) + TEXT("-remove-all-comparison.csv"));
		}
		return FPaths::ConvertRelativePathToFull(OutputPath);
	}

	FString GetAppendComparisonOutputPath()
	{
		FString OutputPath;
		if (!FParse::Value(FCommandLine::Get(), TEXT("DirectiveUtilitiesPerfAppendComparisonOutput="), OutputPath))
		{
			const FString RuntimeOutputPath = GetOutputPath();
			OutputPath = FPaths::GetPath(RuntimeOutputPath)
				/ (FPaths::GetBaseFilename(RuntimeOutputPath) + TEXT("-append-comparison.csv"));
		}
		return FPaths::ConvertRelativePathToFull(OutputPath);
	}

	FString GetInsertComparisonOutputPath()
	{
		const FString RuntimeOutputPath = GetOutputPath();
		return FPaths::ConvertRelativePathToFull(
			FPaths::GetPath(RuntimeOutputPath)
			/ (FPaths::GetBaseFilename(RuntimeOutputPath) + TEXT("-insert-comparison.csv")));
	}

	FString GetRemoveIndicesComparisonOutputPath()
	{
		const FString RuntimeOutputPath = GetOutputPath();
		return FPaths::ConvertRelativePathToFull(
			FPaths::GetPath(RuntimeOutputPath)
			/ (FPaths::GetBaseFilename(RuntimeOutputPath) + TEXT("-remove-indices-comparison.csv")));
	}

	FString GetBuildConfigurationName()
	{
#if UE_BUILD_DEBUG
		return TEXT("Debug");
#elif UE_BUILD_DEVELOPMENT
		return TEXT("Development");
#elif UE_BUILD_TEST
		return TEXT("Test");
#elif UE_BUILD_SHIPPING
		return TEXT("Shipping");
#else
		return TEXT("Unknown");
#endif
	}

	FString GetPluginVersion()
	{
		const TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin(TEXT("DirectiveUtilities"));
		return Plugin.IsValid() ? Plugin->GetDescriptor().VersionName : TEXT("Unknown");
	}

	FString SanitizeMetadata(FString Value)
	{
		Value.ReplaceInline(TEXT(","), TEXT(";"));
		Value.ReplaceInline(TEXT("\r"), TEXT(" "));
		Value.ReplaceInline(TEXT("\n"), TEXT(" "));
		return Value;
	}

	bool LoadBaseline(TMap<FString, double>& OutMedians, FString& OutPath)
	{
		if (!FParse::Value(FCommandLine::Get(), TEXT("DirectiveUtilitiesPerfBaseline="), OutPath))
		{
			return true;
		}

		OutPath = FPaths::ConvertRelativePathToFull(OutPath);
		FString Contents;
		if (!FFileHelper::LoadFileToString(Contents, *OutPath))
		{
			return false;
		}

		TArray<FString> Lines;
		Contents.ParseIntoArrayLines(Lines);
		for (const FString& Line : Lines)
		{
			if (Line.IsEmpty() || Line.StartsWith(TEXT("#")) || Line.StartsWith(TEXT("benchmark,")))
			{
				continue;
			}

			TArray<FString> Fields;
			Line.ParseIntoArray(Fields, TEXT(","), false);
			if (Fields.Num() < 4)
			{
				continue;
			}

			const FString Key = FString::Printf(
				TEXT("%s|%d|%d"),
				*Fields[0],
				FCString::Atoi(*Fields[1]),
				FCString::Atoi(*Fields[2]));
			OutMedians.Add(Key, FCString::Atod(*Fields[3]));
		}
		return true;
	}

	FString BuildCsv(
		const TArray<FResult>& Results,
		const TMap<FString, double>& BaselineMedians,
		const FString& BaselinePath)
	{
		FString Csv;
		Csv += FString::Printf(TEXT("#engine,%s\n"), *FEngineVersion::Current().ToString());
		Csv += FString::Printf(TEXT("#platform,%hs\n"), FPlatformProperties::PlatformName());
		Csv += FString::Printf(TEXT("#timestamp_utc,%s\n"), *FDateTime::UtcNow().ToIso8601());
		if (!BaselinePath.IsEmpty())
		{
			Csv += FString::Printf(TEXT("#baseline,%s\n"), *BaselinePath);
		}
		Csv += TEXT("benchmark,element_count,parameter,median_ms,min_ms,max_ms,samples,baseline_median_ms,speedup,change_percent\n");

		for (const FResult& Result : Results)
		{
			FString BaselineMedian;
			FString Speedup;
			FString ChangePercent;
			if (const double* Baseline = BaselineMedians.Find(Result.GetKey()))
			{
				BaselineMedian = FString::Printf(TEXT("%.9f"), *Baseline);
				if (*Baseline > 0.0 && Result.MedianMilliseconds > 0.0)
				{
					Speedup = FString::Printf(TEXT("%.4f"), *Baseline / Result.MedianMilliseconds);
					ChangePercent = FString::Printf(
						TEXT("%.2f"),
						((*Baseline - Result.MedianMilliseconds) / *Baseline) * 100.0);
				}
			}

			Csv += FString::Printf(
				TEXT("%s,%d,%d,%.9f,%.9f,%.9f,%d,%s,%s,%s\n"),
				*Result.Name,
				Result.ElementCount,
				Result.Parameter,
				Result.MedianMilliseconds,
				Result.MinimumMilliseconds,
				Result.MaximumMilliseconds,
				Result.SampleCount,
				*BaselineMedian,
				*Speedup,
				*ChangePercent);
		}
		return Csv;
	}

	FString BuildComparisonCsv(const TArray<FComparisonResult>& Results)
	{
		FString Revision;
		FParse::Value(FCommandLine::Get(), TEXT("DirectiveUtilitiesPerfRevision="), Revision);

		FString Csv;
		Csv += FString::Printf(TEXT("#engine,%s\n"), *SanitizeMetadata(FEngineVersion::Current().ToString()));
		Csv += FString::Printf(TEXT("#platform,%hs\n"), FPlatformProperties::PlatformName());
		Csv += FString::Printf(TEXT("#cpu,%s\n"), *SanitizeMetadata(FPlatformMisc::GetCPUBrand().TrimStartAndEnd()));
		Csv += FString::Printf(TEXT("#configuration,%s\n"), *GetBuildConfigurationName());
		Csv += FString::Printf(TEXT("#plugin_version,%s\n"), *SanitizeMetadata(GetPluginVersion()));
		Csv += FString::Printf(TEXT("#timestamp_utc,%s\n"), *FDateTime::UtcNow().ToIso8601());
		Csv += FString::Printf(TEXT("#revision,%s\n"), *SanitizeMetadata(Revision));
		Csv += TEXT("element_type,pattern,element_count,match_count,before_median_ms,before_min_ms,before_max_ms,after_median_ms,after_min_ms,after_max_ms,samples,speedup,time_reduction_percent\n");

		for (const FComparisonResult& Result : Results)
		{
			const double Speedup = Result.AfterMedianMilliseconds > 0.0
				? Result.BeforeMedianMilliseconds / Result.AfterMedianMilliseconds
				: 0.0;
			const double TimeReductionPercent = Result.BeforeMedianMilliseconds > 0.0
				? ((Result.BeforeMedianMilliseconds - Result.AfterMedianMilliseconds) / Result.BeforeMedianMilliseconds) * 100.0
				: 0.0;
			Csv += FString::Printf(
				TEXT("%s,%s,%d,%d,%.9f,%.9f,%.9f,%.9f,%.9f,%.9f,%d,%.4f,%.2f\n"),
				*Result.ElementType,
				*Result.Pattern,
				Result.ElementCount,
				Result.MatchCount,
				Result.BeforeMedianMilliseconds,
				Result.BeforeMinimumMilliseconds,
				Result.BeforeMaximumMilliseconds,
				Result.AfterMedianMilliseconds,
				Result.AfterMinimumMilliseconds,
				Result.AfterMaximumMilliseconds,
				Result.SampleCount,
				Speedup,
				TimeReductionPercent);
		}
		return Csv;
	}

	FString BuildAppendComparisonCsv(const TArray<FAppendComparisonResult>& Results)
	{
		FString Revision;
		FParse::Value(FCommandLine::Get(), TEXT("DirectiveUtilitiesPerfRevision="), Revision);

		FString Csv;
		Csv += FString::Printf(TEXT("#engine,%s\n"), *SanitizeMetadata(FEngineVersion::Current().ToString()));
		Csv += FString::Printf(TEXT("#platform,%hs\n"), FPlatformProperties::PlatformName());
		Csv += FString::Printf(TEXT("#cpu,%s\n"), *SanitizeMetadata(FPlatformMisc::GetCPUBrand().TrimStartAndEnd()));
		Csv += FString::Printf(TEXT("#configuration,%s\n"), *GetBuildConfigurationName());
		Csv += FString::Printf(TEXT("#plugin_version,%s\n"), *SanitizeMetadata(GetPluginVersion()));
		Csv += FString::Printf(TEXT("#timestamp_utc,%s\n"), *FDateTime::UtcNow().ToIso8601());
		Csv += FString::Printf(TEXT("#revision,%s\n"), *SanitizeMetadata(Revision));
		Csv += TEXT("element_type,scenario,source_count,initial_target_count,before_median_ms,before_min_ms,before_max_ms,after_median_ms,after_min_ms,after_max_ms,samples,speedup,time_reduction_percent\n");

		for (const FAppendComparisonResult& Result : Results)
		{
			const double Speedup = Result.AfterMedianMilliseconds > 0.0
				? Result.BeforeMedianMilliseconds / Result.AfterMedianMilliseconds
				: 0.0;
			const double TimeReductionPercent = Result.BeforeMedianMilliseconds > 0.0
				? ((Result.BeforeMedianMilliseconds - Result.AfterMedianMilliseconds) / Result.BeforeMedianMilliseconds) * 100.0
				: 0.0;
			Csv += FString::Printf(
				TEXT("%s,%s,%d,%d,%.9f,%.9f,%.9f,%.9f,%.9f,%.9f,%d,%.4f,%.2f\n"),
				*Result.ElementType,
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
				Speedup,
				TimeReductionPercent);
		}
		return Csv;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDirectiveUtilRuntimePerformanceTest,
	"Performance.DirectiveUtilities.Runtime",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::PerfFilter)

bool FDirectiveUtilRuntimePerformanceTest::RunTest(const FString& Parameters)
{
	using namespace DirectiveUtilRuntimePerformance;

	UDirectiveUtilTestObject* TestObject = NewObject<UDirectiveUtilTestObject>();
	FArrayProperty* ArrayProperty = FindFProperty<FArrayProperty>(
		UDirectiveUtilTestObject::StaticClass(),
		GET_MEMBER_NAME_CHECKED(UDirectiveUtilTestObject, TestArray));
	if (!TestNotNull(TEXT("Integer array property is available"), ArrayProperty))
	{
		return false;
	}
	FArrayProperty* StringArrayProperty = FindFProperty<FArrayProperty>(
		UDirectiveUtilTestObject::StaticClass(),
		GET_MEMBER_NAME_CHECKED(UDirectiveUtilTestObject, TestStringArray));
	if (!TestNotNull(TEXT("String array property is available"), StringArrayProperty))
	{
		return false;
	}

	constexpr int32 SampleCount = 7;
	TArray<FResult> Results;
	TArray<FComparisonResult> ComparisonResults;
	TArray<FAppendComparisonResult> AppendComparisonResults;
	TArray<FAppendComparisonResult> InsertComparisonResults;
	TArray<FAppendComparisonResult> RemoveIndicesComparisonResults;
	for (const int32 ElementCount : {1000, 10000, 100000, 250000, 1000000})
	{
		const TArray<int32> Source = MakeSequentialIntegers(ElementCount);
		const TArray<int32> EmptyTarget;
		const TArray<int32> PopulatedTarget = MakeSequentialIntegers(ElementCount);
		auto BeforeAppend = [&](TArray<int32>& Target, TArray<int32>& AppendSource)
		{
			UKismetArrayLibrary::GenericArray_Append(
				&Target,
				ArrayProperty,
				&AppendSource,
				ArrayProperty);
		};
		auto AfterAppend = [&](TArray<int32>& Target, TArray<int32>& AppendSource)
		{
			UDirectiveUtilArrayFunctionLibrary::GenericArray_AppendOptimized(
				&Target,
				ArrayProperty,
				&AppendSource,
				ArrayProperty);
		};
		AppendComparisonResults.Add(MeasureAppendComparison(
			TEXT("int32"),
			TEXT("empty_target"),
			EmptyTarget,
			Source,
			SampleCount,
			BeforeAppend,
			AfterAppend));
		AppendComparisonResults.Add(MeasureAppendComparison(
			TEXT("int32"),
			TEXT("populated_target"),
			PopulatedTarget,
			Source,
			SampleCount,
			BeforeAppend,
			AfterAppend));
	}

	for (const int32 ElementCount : {1000, 10000, 100000})
	{
		TArray<FString> Source;
		Source.Reserve(ElementCount);
		for (int32 Index = 0; Index < ElementCount; ++Index)
		{
			Source.Add(FString::Printf(TEXT("Value%06d"), Index));
		}
		const TArray<FString> EmptyTarget;
		AppendComparisonResults.Add(MeasureAppendComparison(
			TEXT("FString"),
			TEXT("empty_target"),
			EmptyTarget,
			Source,
			SampleCount,
			[&](TArray<FString>& Target, TArray<FString>& AppendSource)
			{
				UKismetArrayLibrary::GenericArray_Append(
					&Target,
					StringArrayProperty,
					&AppendSource,
					StringArrayProperty);
			},
			[&](TArray<FString>& Target, TArray<FString>& AppendSource)
			{
				UDirectiveUtilArrayFunctionLibrary::GenericArray_AppendOptimized(
					&Target,
					StringArrayProperty,
					&AppendSource,
					StringArrayProperty);
			}));
	}

	for (const int32 TargetCount : {256, 1024, 4096, 16384})
	{
		const TArray<int32> InitialTarget = MakeSequentialIntegers(TargetCount);
		TArray<int32> InsertSource = MakeSequentialIntegers(FMath::Max(1, TargetCount / 4));
		for (int32& Value : InsertSource)
		{
			Value += TargetCount;
		}

		for (const TPair<FString, int32>& Scenario : {
			TPair<FString, int32>(TEXT("front"), 0),
			TPair<FString, int32>(TEXT("middle"), TargetCount / 2),
			TPair<FString, int32>(TEXT("end"), TargetCount)
		})
		{
			const int32 InsertIndex = Scenario.Value;
			InsertComparisonResults.Add(MeasureAppendComparison(
				TEXT("int32"),
				Scenario.Key,
				InitialTarget,
				InsertSource,
				SampleCount,
				[&](TArray<int32>& Target, TArray<int32>& Source)
				{
					for (int32 SourceIndex = 0; SourceIndex < Source.Num(); ++SourceIndex)
					{
						UKismetArrayLibrary::GenericArray_Insert(
							&Target,
							ArrayProperty,
							&Source[SourceIndex],
							InsertIndex + SourceIndex);
					}
				},
				[&](TArray<int32>& Target, TArray<int32>& Source)
				{
					UDirectiveUtilArrayFunctionLibrary::GenericArray_InsertOptimized(
						&Target,
						ArrayProperty,
						&Source,
						ArrayProperty,
						InsertIndex);
				}));
		}

		TArray<int32> RemovalIndices;
		for (int32 Index = 1; Index < TargetCount; Index += 4)
		{
			RemovalIndices.Add(Index);
		}
		RemoveIndicesComparisonResults.Add(MeasureAppendComparison(
			TEXT("int32"),
			TEXT("every_4"),
			InitialTarget,
			RemovalIndices,
			SampleCount,
			[&](TArray<int32>& Target, TArray<int32>& Indices)
			{
				for (int32 IndexOffset = Indices.Num() - 1; IndexOffset >= 0; --IndexOffset)
				{
					UKismetArrayLibrary::GenericArray_Remove(
						&Target,
						ArrayProperty,
						Indices[IndexOffset]);
				}
			},
			[&](TArray<int32>& Target, TArray<int32>& Indices)
			{
				UDirectiveUtilArrayFunctionLibrary::GenericArray_RemoveAtIndices(
					&Target,
					ArrayProperty,
					Indices);
			}));
	}

	const TArray<FString> RemovalPatterns = {
		TEXT("no_match"),
		TEXT("single_tail"),
		TEXT("every_64"),
		TEXT("clustered"),
		TEXT("alternating"),
		TEXT("all")
	};
	constexpr int32 IntegerToRemove = 0;
	for (const int32 ElementCount : {256, 1024, 4096, 16384})
	{
		for (const FString& Pattern : RemovalPatterns)
		{
			int32 MatchCount = 0;
			const TArray<int32> Source = MakeRemovalIntegers(
				ElementCount,
				Pattern,
				IntegerToRemove,
				MatchCount);
			ComparisonResults.Add(MeasureComparison(
				TEXT("int32"),
				Pattern,
				Source,
				MatchCount,
				SampleCount,
				[&](TArray<int32>& Values)
				{
					return UKismetArrayLibrary::GenericArray_RemoveItem(&Values, ArrayProperty, &IntegerToRemove);
				},
				[&](TArray<int32>& Values)
				{
					return UDirectiveUtilArrayFunctionLibrary::GenericArray_RemoveAllOccurrences(
						&Values,
						ArrayProperty,
						&IntegerToRemove);
				}));
		}
	}

	// The scaling curve uses every_64 only to keep the stock path's runtime bounded.
	for (const int32 ElementCount : {1000, 10000, 100000, 250000, 1000000})
	{
		int32 MatchCount = 0;
		const TArray<int32> Source = MakeRemovalIntegers(
			ElementCount,
			TEXT("every_64"),
			IntegerToRemove,
			MatchCount);
		ComparisonResults.Add(MeasureComparison(
			TEXT("int32"),
			TEXT("every_64"),
			Source,
			MatchCount,
			SampleCount,
			[&](TArray<int32>& Values)
			{
				return UKismetArrayLibrary::GenericArray_RemoveItem(&Values, ArrayProperty, &IntegerToRemove);
			},
			[&](TArray<int32>& Values)
			{
				return UDirectiveUtilArrayFunctionLibrary::GenericArray_RemoveAllOccurrences(
					&Values,
					ArrayProperty,
					&IntegerToRemove);
			}));
	}

	const FString StringToRemove = TEXT("REMOVE");
	for (const int32 ElementCount : {256, 1024, 4096})
	{
		for (const FString& Pattern : RemovalPatterns)
		{
			int32 MatchCount = 0;
			const TArray<FString> Source = MakeRemovalStrings(
				ElementCount,
				Pattern,
				StringToRemove,
				MatchCount);
			ComparisonResults.Add(MeasureComparison(
				TEXT("FString"),
				Pattern,
				Source,
				MatchCount,
				SampleCount,
				[&](TArray<FString>& Values)
				{
					return UKismetArrayLibrary::GenericArray_RemoveItem(&Values, StringArrayProperty, &StringToRemove);
				},
				[&](TArray<FString>& Values)
				{
					return UDirectiveUtilArrayFunctionLibrary::GenericArray_RemoveAllOccurrences(
						&Values,
						StringArrayProperty,
						&StringToRemove);
				}));
		}
	}

	for (const int32 ElementCount : {16, 256, 1024, 4096, 16384, 65536})
	{
		const TArray<int32> Source = MakeSequentialIntegers(ElementCount);
		const int32 DenseDistinctCount = FMath::Max(1, FMath::Min(64, ElementCount / 4));
		const TArray<int32> DenseSource = MakeRepeatingIntegers(ElementCount, DenseDistinctCount);
		TArray<int32> Output;
		int32 MostCommonItem = INDEX_NONE;
		int32 MostCommonCount = 0;

		Results.Add(Measure(
			TEXT("RemoveDuplicatesUnique"), ElementCount, 0, SampleCount,
			[&]() { TestObject->TestArray = Source; },
			[&]() { UDirectiveUtilArrayFunctionLibrary::GenericArray_RemoveDuplicates(&TestObject->TestArray, ArrayProperty); }));

		Results.Add(Measure(
			TEXT("GetDistinctUnique"), ElementCount, 0, SampleCount,
			[&]() { TestObject->TestArray = Source; Output.Reset(); },
			[&]() { UDirectiveUtilArrayFunctionLibrary::GenericArray_GetDistinct(&TestObject->TestArray, ArrayProperty, &Output, ArrayProperty); }));

		Results.Add(Measure(
			TEXT("GetMostCommonUnique"), ElementCount, 0, SampleCount,
			[&]() { TestObject->TestArray = Source; MostCommonItem = INDEX_NONE; MostCommonCount = 0; },
			[&]() { UDirectiveUtilArrayFunctionLibrary::GenericArray_GetMostCommon(&TestObject->TestArray, ArrayProperty, &MostCommonItem, &MostCommonCount); }));

		Results.Add(Measure(
			TEXT("RemoveDuplicatesDense"), ElementCount, DenseDistinctCount, SampleCount,
			[&]() { TestObject->TestArray = DenseSource; },
			[&]() { UDirectiveUtilArrayFunctionLibrary::GenericArray_RemoveDuplicates(&TestObject->TestArray, ArrayProperty); }));

		Results.Add(Measure(
			TEXT("GetDistinctDense"), ElementCount, DenseDistinctCount, SampleCount,
			[&]() { TestObject->TestArray = DenseSource; Output.Reset(); },
			[&]() { UDirectiveUtilArrayFunctionLibrary::GenericArray_GetDistinct(&TestObject->TestArray, ArrayProperty, &Output, ArrayProperty); }));

		Results.Add(Measure(
			TEXT("GetMostCommonDense"), ElementCount, DenseDistinctCount, SampleCount,
			[&]() { TestObject->TestArray = DenseSource; MostCommonItem = INDEX_NONE; MostCommonCount = 0; },
			[&]() { UDirectiveUtilArrayFunctionLibrary::GenericArray_GetMostCommon(&TestObject->TestArray, ArrayProperty, &MostCommonItem, &MostCommonCount); }));
	}

	for (const int32 ElementCount : {100, 1000, 10000, 100000})
	{
		const TArray<int32> Source = MakeSequentialIntegers(ElementCount);
		TArray<float> Weights;
		Weights.Reserve(ElementCount);
		for (int32 Index = 0; Index < ElementCount; ++Index)
		{
			Weights.Add(Index % 11 == 0 ? 0.0f : static_cast<float>((Index % 17) + 1));
		}
		TArray<int32> Output;
		FRandomStream RandomStream;

		Results.Add(Measure(
			TEXT("SampleWithoutReplacement"), ElementCount, 16, SampleCount,
			[&]() { TestObject->TestArray = Source; Output.Reset(); RandomStream.Initialize(1337); },
			[&]() { UDirectiveUtilArrayFunctionLibrary::GenericArray_Sample(&TestObject->TestArray, ArrayProperty, 16, false, &RandomStream, &Output, ArrayProperty); }));

		Results.Add(Measure(
			TEXT("SampleWeightedWithoutReplacement"), ElementCount, 16, SampleCount,
			[&]() { TestObject->TestArray = Source; Output.Reset(); RandomStream.Initialize(1337); },
			[&]() { UDirectiveUtilArrayFunctionLibrary::GenericArray_SampleWeighted(&TestObject->TestArray, ArrayProperty, Weights, 16, false, &RandomStream, &Output, ArrayProperty); }));

		Results.Add(Measure(
			TEXT("SampleWeightedWithReplacement"), ElementCount, 256, SampleCount,
			[&]() { TestObject->TestArray = Source; Output.Reset(); RandomStream.Initialize(1337); },
			[&]() { UDirectiveUtilArrayFunctionLibrary::GenericArray_SampleWeighted(&TestObject->TestArray, ArrayProperty, Weights, 256, true, &RandomStream, &Output, ArrayProperty); }));
	}

	{
		constexpr int32 ElementCount = 100000;
		const TArray<int32> Source = MakeSequentialIntegers(ElementCount);
		TArray<float> Weights;
		Weights.Init(1.0f, ElementCount);
		TArray<int32> Output;
		FRandomStream RandomStream;
		int32 PageCount = 0;
		for (const int32 RequestedCount : {1, 16, 24999, 25000, 25001, 50000, 75000, 100000})
		{
			const TCHAR* BenchmarkName = RequestedCount == 75000
				? TEXT("SampleWithoutReplacementDense")
				: TEXT("SampleWithoutReplacementRatio");
			Results.Add(Measure(
				BenchmarkName, ElementCount, RequestedCount, SampleCount,
				[&]() { TestObject->TestArray = Source; Output.Reset(); RandomStream.Initialize(1337); },
				[&]() { UDirectiveUtilArrayFunctionLibrary::GenericArray_Sample(&TestObject->TestArray, ArrayProperty, RequestedCount, false, &RandomStream, &Output, ArrayProperty); }));
		}

		Results.Add(Measure(
			TEXT("SampleWithoutReplacementAlias"), ElementCount, 16, SampleCount,
			[&]() { TestObject->TestArray = Source; RandomStream.Initialize(1337); },
			[&]() { UDirectiveUtilArrayFunctionLibrary::GenericArray_Sample(&TestObject->TestArray, ArrayProperty, 16, false, &RandomStream, &TestObject->TestArray, ArrayProperty); }));

		Results.Add(Measure(
			TEXT("SampleWithReplacement"), ElementCount, 256, SampleCount,
			[&]() { TestObject->TestArray = Source; Output.Reset(); RandomStream.Initialize(1337); },
			[&]() { UDirectiveUtilArrayFunctionLibrary::GenericArray_Sample(&TestObject->TestArray, ArrayProperty, 256, true, &RandomStream, &Output, ArrayProperty); }));

		for (const int32 RequestedCount : {1, 16, 1000, 50000})
		{
			Results.Add(Measure(
				TEXT("SampleWeightedRatio"), ElementCount, RequestedCount, SampleCount,
				[&]() { TestObject->TestArray = Source; Output.Reset(); RandomStream.Initialize(1337); },
				[&]() { UDirectiveUtilArrayFunctionLibrary::GenericArray_SampleWeighted(&TestObject->TestArray, ArrayProperty, Weights, RequestedCount, false, &RandomStream, &Output, ArrayProperty); }));
		}

		for (const int32 PageSize : {1, 128, 4096})
		{
			const int32 PageIndex = PageSize == 128 ? 400 : (ElementCount / PageSize) / 2;
			Results.Add(Measure(
				TEXT("GetPage"), ElementCount, PageSize, SampleCount,
				[&]() { TestObject->TestArray = Source; Output.Reset(); PageCount = 0; },
				[&]() { UDirectiveUtilArrayFunctionLibrary::GenericArray_GetPage(&TestObject->TestArray, ArrayProperty, PageIndex, PageSize, &Output, ArrayProperty, &PageCount); }));
		}

		Results.Add(Measure(
			TEXT("GetPageAlias"), ElementCount, 128, SampleCount,
			[&]() { TestObject->TestArray = Source; PageCount = 0; },
			[&]() { UDirectiveUtilArrayFunctionLibrary::GenericArray_GetPage(&TestObject->TestArray, ArrayProperty, 400, 128, &TestObject->TestArray, ArrayProperty, &PageCount); }));

		Results.Add(Measure(
			TEXT("SliceAliasCorrectness"), ElementCount, 128, SampleCount,
			[&]() { TestObject->TestArray = Source; },
			[&]() { UDirectiveUtilArrayFunctionLibrary::GenericArray_Slice(&TestObject->TestArray, ArrayProperty, 51200, 128, &TestObject->TestArray, ArrayProperty); }));

		int32 ItemToCount = ElementCount - 1;
		int32 OccurrenceCount = 0;
		Results.Add(Measure(
			TEXT("CountOccurrences"), ElementCount, 0, SampleCount,
			[&]() { TestObject->TestArray = Source; OccurrenceCount = 0; },
			[&]() { OccurrenceCount = UDirectiveUtilArrayFunctionLibrary::GenericArray_CountOccurrences(&TestObject->TestArray, ArrayProperty, &ItemToCount); }));

	}

	{
		constexpr int32 ElementCount = 1000000;
		const TArray<int32> Source = MakeSequentialIntegers(ElementCount);
		int32 ItemToCount = ElementCount - 1;
		int32 OccurrenceCount = 0;
		Results.Add(Measure(
			TEXT("CountOccurrences"), ElementCount, 0, SampleCount,
			[&]() { TestObject->TestArray = Source; OccurrenceCount = 0; },
			[&]() { OccurrenceCount = UDirectiveUtilArrayFunctionLibrary::GenericArray_CountOccurrences(&TestObject->TestArray, ArrayProperty, &ItemToCount); }));
	}

	for (const int32 ElementCount : {1000, 100000, 1000000})
	{
		const TArray<int32> Source = MakeSequentialIntegers(ElementCount);
		for (const int32 Shift : {1, ElementCount / 3, ElementCount - 1})
		{
			Results.Add(Measure(
				TEXT("Rotate"), ElementCount, Shift, SampleCount,
				[&]() { TestObject->TestArray = Source; },
				[&]() { UDirectiveUtilArrayFunctionLibrary::GenericArray_Rotate(&TestObject->TestArray, ArrayProperty, Shift); }));
		}
	}

	for (const int32 ElementCount : {100, 1000, 10000})
	{
		const TArray<FString> SourceStrings = MakeNaturalSortStrings(ElementCount);
		const TArray<FName> SourceNames = MakeNaturalSortNames(ElementCount);
		TArray<FString> WorkingStrings;
		TArray<FName> WorkingNames;

		Results.Add(Measure(
			TEXT("NaturalSortString"), ElementCount, 0, SampleCount,
			[&]() { WorkingStrings = SourceStrings; },
			[&]() { UDirectiveUtilArrayFunctionLibrary::NaturalSortStringArray(WorkingStrings); }));

		Results.Add(Measure(
			TEXT("NaturalSortName"), ElementCount, 0, SampleCount,
			[&]() { WorkingNames = SourceNames; },
			[&]() { UDirectiveUtilArrayFunctionLibrary::NaturalSortNameArray(WorkingNames); }));
	}

	for (const int32 ElementCount : {101, 1001, 100001})
	{
		const TArray<int32> ShuffledIndices = MakeShuffledIndices(ElementCount);
		TArray<float> ShuffledFloats;
		ShuffledFloats.Reserve(ElementCount);
		for (const int32 Value : ShuffledIndices)
		{
			ShuffledFloats.Add(static_cast<float>(Value) + 0.25f);
		}
		float MedianResult = 0.0f;
		float PercentileResult = 0.0f;

		Results.Add(Measure(
			TEXT("IntMedian"), ElementCount, 0, SampleCount,
			[]() {},
			[&]() { MedianResult = UDirectiveUtilMathFunctionLibrary::GetIntArrayMedian(ShuffledIndices); }));

		Results.Add(Measure(
			TEXT("FloatMedian"), ElementCount, 0, SampleCount,
			[]() {},
			[&]() { MedianResult = UDirectiveUtilMathFunctionLibrary::GetFloatArrayMedian(ShuffledFloats); }));

		Results.Add(Measure(
			TEXT("FloatPercentile"), ElementCount, 40, SampleCount,
			[]() {},
			[&]() { UDirectiveUtilMathFunctionLibrary::GetFloatArrayPercentile(ShuffledFloats, 40.0f, PercentileResult); }));

		if (ElementCount == 100001)
		{
			const TArray<int32> SortedValues = MakeSequentialIntegers(ElementCount);
			TArray<int32> ReverseValues = SortedValues;
			Algo::Reverse(ReverseValues);
			Results.Add(Measure(
				TEXT("IntMedianSorted"), ElementCount, 0, SampleCount,
				[]() {},
				[&]() { MedianResult = UDirectiveUtilMathFunctionLibrary::GetIntArrayMedian(SortedValues); }));
			Results.Add(Measure(
				TEXT("IntMedianReverse"), ElementCount, 0, SampleCount,
				[]() {},
				[&]() { MedianResult = UDirectiveUtilMathFunctionLibrary::GetIntArrayMedian(ReverseValues); }));
		}
	}

	for (const int32 ElementCount : {1000, 10000, 100000})
	{
		TArray<FVector> PointOutput;
		TArray<FTransform> TransformOutput;
		TArray<FIntPoint> CoordinateOutput;
		TArray<FVector> EaseFromLocations;
		TArray<FVector> EaseToLocations;
		EaseFromLocations.Reserve(ElementCount);
		EaseToLocations.Reserve(ElementCount);
		for (int32 Index = 0; Index < ElementCount; ++Index)
		{
			const FVector Location(Index, Index * 0.5, -Index);
			EaseFromLocations.Add(Location);
			EaseToLocations.Add(Location + FVector(100.0, -50.0, 25.0));
		}

		Results.Add(Measure(
			TEXT("GenerateGridPoints2D"), ElementCount, 0, SampleCount,
			[&]() { PointOutput.Reset(); },
			[&]() { PointOutput = UDirectiveUtilMathFunctionLibrary::GenerateGridPoints2D(
				FVector::ZeroVector, FRotator::ZeroRotator, FIntPoint(ElementCount, 1), FVector2D(100.0, 100.0)); }));
		Results.Add(Measure(
			TEXT("GetRectangularHexGridCoordinates"), ElementCount, 0, SampleCount,
			[&]() { CoordinateOutput.Reset(); },
			[&]() { CoordinateOutput = UDirectiveUtilMathFunctionLibrary::GetRectangularHexGridCoordinates(
				FIntPoint(ElementCount, 1)); }));
		Results.Add(Measure(
			TEXT("GeneratePointsOnCircle"), ElementCount, 0, SampleCount,
			[&]() { PointOutput.Reset(); },
			[&]() { PointOutput = UDirectiveUtilMathFunctionLibrary::GeneratePointsOnCircle(
				FVector::ZeroVector, FRotator::ZeroRotator, 1000.0, ElementCount); }));
		Results.Add(Measure(
			TEXT("GeneratePointsOnSphere"), ElementCount, 0, SampleCount,
			[&]() { PointOutput.Reset(); },
			[&]() { PointOutput = UDirectiveUtilMathFunctionLibrary::GeneratePointsOnSphere(
				FVector::ZeroVector, FRotator::ZeroRotator, 1000.0, ElementCount); }));
		Results.Add(Measure(
			TEXT("GenerateTransformsOnCircle"), ElementCount, 0, SampleCount,
			[&]() { TransformOutput.Reset(); },
			[&]() { TransformOutput = UDirectiveUtilMathFunctionLibrary::GenerateTransformsOnCircle(
				FVector::ZeroVector, FRotator::ZeroRotator, 1000.0, ElementCount); }));
		Results.Add(Measure(
			TEXT("EaseLocationArrays"), ElementCount, 0, SampleCount,
			[&]() { PointOutput.Reset(); },
			[&]() { PointOutput = UDirectiveUtilMathFunctionLibrary::EaseLocationArrays(
			EaseFromLocations, EaseToLocations, 0.5f, EDirectiveUtilEaseType::BackInOut, {}); }));
	}

	{
		constexpr int32 ElementCount = 1000000;
		TArray<FVector> PointOutput;
		Results.Add(Measure(
			TEXT("GenerateGridPoints2D"), ElementCount, 0, SampleCount,
			[&]() { PointOutput.Reset(); },
			[&]() { PointOutput = UDirectiveUtilMathFunctionLibrary::GenerateGridPoints2D(
				FVector::ZeroVector, FRotator::ZeroRotator, FIntPoint(ElementCount, 1), FVector2D(100.0, 100.0)); }));
	}

	for (const int32 Radius : {10, 100, 250})
	{
		TArray<FIntPoint> HexOutput;
		const int32 ElementCount = 1 + 3 * Radius * (Radius + 1);
		Results.Add(Measure(
			TEXT("GetHexesInRange"), ElementCount, Radius, SampleCount,
			[&]() { HexOutput.Reset(); },
			[&]() { HexOutput = UDirectiveUtilMathFunctionLibrary::GetHexesInRange(
				FIntPoint::ZeroValue, Radius); }));
	}

	for (const int32 OperationCount : {1000, 100000})
	{
		double FalloffSum = 0.0;
		Results.Add(Measure(
			TEXT("RangeFalloffDefault"), OperationCount, 1, SampleCount,
			[&]() { FalloffSum = 0.0; },
			[&]()
			{
				for (int32 Index = 0; Index < OperationCount; ++Index)
				{
					FalloffSum += UDirectiveUtilMathFunctionLibrary::RangeFalloff(
						static_cast<float>(Index % 1000), 100.0f, 900.0f);
				}
			}));

		FRandomStream SphereStream;
		FVector SphereSum = FVector::ZeroVector;
		Results.Add(Measure(
			TEXT("RandomPointInSphereStream"), OperationCount, 0, SampleCount,
			[&]() { SphereStream.Initialize(1337); SphereSum = FVector::ZeroVector; },
			[&]()
			{
				for (int32 Index = 0; Index < OperationCount; ++Index)
				{
					SphereSum += UDirectiveUtilMathFunctionLibrary::RandomPointInSphereFromStream(
						SphereStream, 100.0f);
				}
			}));
	}

	for (const int32 CandidateCount : {10, 1000, 10000})
	{
		const TArray<FString> Candidates = MakeStringMatchCandidates(CandidateCount);
		const FString Input = FString::Printf(TEXT("DirectiveUtilityCandidate%05dX"), CandidateCount - 1);
		float Similarity = 0.0f;
		int32 MatchIndex = INDEX_NONE;

		Results.Add(Measure(
			TEXT("FindBestStringMatch"), CandidateCount, Input.Len(), SampleCount,
			[&]() { Similarity = 0.0f; MatchIndex = INDEX_NONE; },
			[&]() { MatchIndex = UDirectiveUtilStringFunctionLibrary::FindBestStringMatch(Input, Candidates, Similarity); }));
	}

	for (const FComparisonResult& Result : ComparisonResults)
	{
		if (!Result.bOutputsMatch)
		{
			AddError(FString::Printf(
				TEXT("RemoveAll comparison mismatch for %s elements=%d pattern=%s"),
				*Result.ElementType,
				Result.ElementCount,
				*Result.Pattern));
		}

		const double Speedup = Result.AfterMedianMilliseconds > 0.0
			? Result.BeforeMedianMilliseconds / Result.AfterMedianMilliseconds
			: 0.0;
		const double TimeReductionPercent = Result.BeforeMedianMilliseconds > 0.0
			? ((Result.BeforeMedianMilliseconds - Result.AfterMedianMilliseconds) / Result.BeforeMedianMilliseconds) * 100.0
			: 0.0;
		AddInfo(FString::Printf(
			TEXT("REMOVE_ALL_PERF type=%s elements=%d pattern=%s matches=%d before=%.6fms after=%.6fms speedup=%.3fx reduction=%.2f%%"),
			*Result.ElementType,
			Result.ElementCount,
			*Result.Pattern,
			Result.MatchCount,
			Result.BeforeMedianMilliseconds,
			Result.AfterMedianMilliseconds,
			Speedup,
			TimeReductionPercent));
	}

	for (const FAppendComparisonResult& Result : AppendComparisonResults)
	{
		if (!Result.bOutputsMatch)
		{
			AddError(FString::Printf(
				TEXT("Append comparison mismatch for %s source=%d scenario=%s"),
				*Result.ElementType,
				Result.SourceCount,
				*Result.Scenario));
		}

		const double Speedup = Result.AfterMedianMilliseconds > 0.0
			? Result.BeforeMedianMilliseconds / Result.AfterMedianMilliseconds
			: 0.0;
		const double TimeReductionPercent = Result.BeforeMedianMilliseconds > 0.0
			? ((Result.BeforeMedianMilliseconds - Result.AfterMedianMilliseconds) / Result.BeforeMedianMilliseconds) * 100.0
			: 0.0;
		AddInfo(FString::Printf(
			TEXT("APPEND_PERF type=%s source=%d initial_target=%d scenario=%s before=%.6fms after=%.6fms speedup=%.3fx reduction=%.2f%%"),
			*Result.ElementType,
			Result.SourceCount,
			Result.InitialTargetCount,
			*Result.Scenario,
			Result.BeforeMedianMilliseconds,
			Result.AfterMedianMilliseconds,
			Speedup,
			TimeReductionPercent));
	}

	for (const FAppendComparisonResult& Result : InsertComparisonResults)
	{
		if (!Result.bOutputsMatch)
		{
			AddError(FString::Printf(
				TEXT("Insert comparison mismatch for %s source=%d scenario=%s"),
				*Result.ElementType,
				Result.SourceCount,
				*Result.Scenario));
		}

		const double Speedup = Result.AfterMedianMilliseconds > 0.0
			? Result.BeforeMedianMilliseconds / Result.AfterMedianMilliseconds
			: 0.0;
		AddInfo(FString::Printf(
			TEXT("INSERT_PERF type=%s source=%d initial_target=%d scenario=%s before=%.6fms after=%.6fms speedup=%.3fx"),
			*Result.ElementType,
			Result.SourceCount,
			Result.InitialTargetCount,
			*Result.Scenario,
			Result.BeforeMedianMilliseconds,
			Result.AfterMedianMilliseconds,
			Speedup));
	}

	for (const FAppendComparisonResult& Result : RemoveIndicesComparisonResults)
	{
		if (!Result.bOutputsMatch)
		{
			AddError(FString::Printf(
				TEXT("Remove At Indices comparison mismatch for %s indices=%d scenario=%s"),
				*Result.ElementType,
				Result.SourceCount,
				*Result.Scenario));
		}

		const double Speedup = Result.AfterMedianMilliseconds > 0.0
			? Result.BeforeMedianMilliseconds / Result.AfterMedianMilliseconds
			: 0.0;
		AddInfo(FString::Printf(
			TEXT("REMOVE_INDICES_PERF type=%s indices=%d initial_target=%d scenario=%s before=%.6fms after=%.6fms speedup=%.3fx"),
			*Result.ElementType,
			Result.SourceCount,
			Result.InitialTargetCount,
			*Result.Scenario,
			Result.BeforeMedianMilliseconds,
			Result.AfterMedianMilliseconds,
			Speedup));
	}

	TMap<FString, double> BaselineMedians;
	FString BaselinePath;
	if (!LoadBaseline(BaselineMedians, BaselinePath))
	{
		AddError(FString::Printf(TEXT("Unable to read performance baseline: %s"), *BaselinePath));
		return false;
	}
	if (!BaselinePath.IsEmpty())
	{
		double MaximumRegressionPercent = 20.0;
		double MinimumGatedMilliseconds = 0.5;
		FParse::Value(
			FCommandLine::Get(),
			TEXT("DirectiveUtilitiesPerfMaxRegressionPercent="),
			MaximumRegressionPercent);
		FParse::Value(
			FCommandLine::Get(),
			TEXT("DirectiveUtilitiesPerfMinGateMilliseconds="),
			MinimumGatedMilliseconds);
		if (!FMath::IsFinite(MaximumRegressionPercent) || MaximumRegressionPercent < 0.0)
		{
			AddError(TEXT("DirectiveUtilitiesPerfMaxRegressionPercent must be finite and non-negative."));
			return false;
		}
		if (!FMath::IsFinite(MinimumGatedMilliseconds) || MinimumGatedMilliseconds < 0.0)
		{
			AddError(TEXT("DirectiveUtilitiesPerfMinGateMilliseconds must be finite and non-negative."));
			return false;
		}

		for (const FResult& Result : Results)
		{
			const double* Baseline = BaselineMedians.Find(Result.GetKey());
			if (!Baseline || *Baseline < MinimumGatedMilliseconds)
			{
				continue;
			}
			const double RegressionPercent = ((Result.MedianMilliseconds - *Baseline) / *Baseline) * 100.0;
			if (RegressionPercent > MaximumRegressionPercent)
			{
				AddError(FString::Printf(
					TEXT("Performance regression for %s elements=%d parameter=%d: %.2f%% exceeds %.2f%%"),
					*Result.Name,
					Result.ElementCount,
					Result.Parameter,
					RegressionPercent,
					MaximumRegressionPercent));
			}
		}
	}

	for (const FResult& Result : Results)
	{
		FString Comparison;
		if (const double* Baseline = BaselineMedians.Find(Result.GetKey()))
		{
			const double Speedup = Result.MedianMilliseconds > 0.0 ? *Baseline / Result.MedianMilliseconds : 0.0;
			const double ChangePercent = *Baseline > 0.0
				? ((*Baseline - Result.MedianMilliseconds) / *Baseline) * 100.0
				: 0.0;
			Comparison = FString::Printf(TEXT(" baseline=%.6fms speedup=%.3fx change=%.2f%%"), *Baseline, Speedup, ChangePercent);
		}
		AddInfo(FString::Printf(
			TEXT("RUNTIME_PERF %s elements=%d parameter=%d median=%.6fms min=%.6fms max=%.6fms samples=%d%s"),
			*Result.Name,
			Result.ElementCount,
			Result.Parameter,
			Result.MedianMilliseconds,
			Result.MinimumMilliseconds,
			Result.MaximumMilliseconds,
			Result.SampleCount,
			*Comparison));
	}

	const FString OutputPath = GetOutputPath();
	IFileManager::Get().MakeDirectory(*FPaths::GetPath(OutputPath), true);
	const FString Csv = BuildCsv(Results, BaselineMedians, BaselinePath);
	TestTrue(
		FString::Printf(TEXT("Performance results saved to %s"), *OutputPath),
		FFileHelper::SaveStringToFile(Csv, *OutputPath));

	const FString ComparisonOutputPath = GetComparisonOutputPath();
	IFileManager::Get().MakeDirectory(*FPaths::GetPath(ComparisonOutputPath), true);
	const FString ComparisonCsv = BuildComparisonCsv(ComparisonResults);
	TestTrue(
		FString::Printf(TEXT("Remove All comparison results saved to %s"), *ComparisonOutputPath),
		FFileHelper::SaveStringToFile(ComparisonCsv, *ComparisonOutputPath));

	const FString AppendComparisonOutputPath = GetAppendComparisonOutputPath();
	IFileManager::Get().MakeDirectory(*FPaths::GetPath(AppendComparisonOutputPath), true);
	const FString AppendComparisonCsv = BuildAppendComparisonCsv(AppendComparisonResults);
	TestTrue(
		FString::Printf(TEXT("Append comparison results saved to %s"), *AppendComparisonOutputPath),
		FFileHelper::SaveStringToFile(AppendComparisonCsv, *AppendComparisonOutputPath));

	const FString InsertComparisonOutputPath = GetInsertComparisonOutputPath();
	IFileManager::Get().MakeDirectory(*FPaths::GetPath(InsertComparisonOutputPath), true);
	TestTrue(
		FString::Printf(TEXT("Insert comparison results saved to %s"), *InsertComparisonOutputPath),
		FFileHelper::SaveStringToFile(
			BuildAppendComparisonCsv(InsertComparisonResults),
			*InsertComparisonOutputPath));

	const FString RemoveIndicesComparisonOutputPath = GetRemoveIndicesComparisonOutputPath();
	IFileManager::Get().MakeDirectory(*FPaths::GetPath(RemoveIndicesComparisonOutputPath), true);
	TestTrue(
		FString::Printf(TEXT("Remove At Indices comparison results saved to %s"), *RemoveIndicesComparisonOutputPath),
		FFileHelper::SaveStringToFile(
			BuildAppendComparisonCsv(RemoveIndicesComparisonResults),
			*RemoveIndicesComparisonOutputPath));
	return !HasAnyErrors();
}
