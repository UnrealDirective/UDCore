// Copyright (c) 2026 Unreal Directive. Licensed under the MIT License.

#include "DirectiveUtilitiesRuntimeAppendBenchmark.h"

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

		FString OutputPath;
		if (FParse::Value(
			FCommandLine::Get(),
			TEXT("DirectiveUtilitiesAppendShippingBenchmarkOutput="),
			OutputPath))
		{
			FCoreDelegates::OnFEngineLoopInitComplete.AddLambda([OutputPath]()
			{
				RunDirectiveUtilitiesRuntimeAppendBenchmark(OutputPath);
				FPlatformMisc::RequestExit(false);
			});
		}
	}
};

IMPLEMENT_PRIMARY_GAME_MODULE(FDirectiveUtilitiesRuntimeHostModule, DirectiveUtilitiesRuntimeHost, "DirectiveUtilitiesRuntimeHost");
