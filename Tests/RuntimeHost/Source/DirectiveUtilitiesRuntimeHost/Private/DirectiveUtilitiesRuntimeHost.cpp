// Copyright (c) 2026 Unreal Directive. Licensed under the MIT License.

#include "DirectiveUtilitiesRuntimeAppendBenchmark.h"
#include "DirectiveUtilitiesRuntimeShippingSmoke.h"

#include "HAL/PlatformMisc.h"
#include "Misc/CommandLine.h"
#include "Misc/CoreDelegates.h"
#include "Misc/Parse.h"
#include "Modules/ModuleManager.h"

class FDirectiveUtilitiesRuntimeHostModule final : public FDefaultGameModuleImpl
{
public:
	virtual void StartupModule() override
	{
		FDefaultGameModuleImpl::StartupModule();

		FString AppendOutputPath;
		FString SmokeOutputPath;
		const bool bRunAppendBenchmark = FParse::Value(
			FCommandLine::Get(), TEXT("DirectiveUtilitiesAppendShippingBenchmarkOutput="), AppendOutputPath);
		const bool bRunShippingSmoke = FParse::Value(
			FCommandLine::Get(), TEXT("DirectiveUtilitiesShippingSmokeOutput="), SmokeOutputPath);
		if (bRunAppendBenchmark || bRunShippingSmoke)
		{
			FCoreDelegates::OnFEngineLoopInitComplete.AddLambda([
				bRunAppendBenchmark, bRunShippingSmoke, AppendOutputPath, SmokeOutputPath]()
			{
				const bool bAppendPassed = !bRunAppendBenchmark
					|| RunDirectiveUtilitiesRuntimeAppendBenchmark(AppendOutputPath);
				const bool bSmokePassed = !bRunShippingSmoke
					|| RunDirectiveUtilitiesRuntimeShippingSmoke(SmokeOutputPath);
				FPlatformMisc::RequestExitWithStatus(false, bAppendPassed && bSmokePassed ? 0 : 1);
			});
		}
	}
};

IMPLEMENT_PRIMARY_GAME_MODULE(FDirectiveUtilitiesRuntimeHostModule, DirectiveUtilitiesRuntimeHost, "DirectiveUtilitiesRuntimeHost");
