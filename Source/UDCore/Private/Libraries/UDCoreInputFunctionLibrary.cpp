// © 2024 Unreal Directive. All rights reserved.


#include "Libraries/UDCoreInputFunctionLibrary.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "UDCoreLogChannels.h"
#include "Logging/StructuredLog.h"
#include "GameFramework/Controller.h"
#include "GameFramework/PlayerController.h"
#include "Engine/LocalPlayer.h"

bool UUDCoreInputFunctionLibrary::TryGetEnhancedInputSubsystemFromController(
	AController* PlayerController,
	UEnhancedInputLocalPlayerSubsystem*& EnhancedInput)
{
	if (!PlayerController)
	{
		UE_LOG(LogUDCore, Warning, TEXT("PlayerController is null. Cannot set input mapping contexts."));
		return false;
	}

	const ULocalPlayer* LocalPlayer = Cast<APlayerController>(PlayerController)->GetLocalPlayer();
	if (!LocalPlayer)
	{
		UE_LOG(LogUDCore, Warning, TEXT("LocalPlayer not found. Cannot set input mapping contexts."));
		return false;
	}

	EnhancedInput = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();

	if(!EnhancedInput)
	{
		UE_LOG(LogUDCore, Warning, TEXT("EnhancedInput subsystem not found. Cannot remove input mapping contexts."));
		return false;
	}
	
	return true;
}

EUDSuccessStatus UUDCoreInputFunctionLibrary::AddInputMappingContexts(
	AController* PlayerController,
	const TArray<FUDCoreEnhancedInputContextData>& Contexts,
	const bool bClearPrevious)
{
	if (Contexts.IsEmpty()) { return EUDSuccessStatus::Failure; }
	
	UEnhancedInputLocalPlayerSubsystem* EnhancedInput;
	const bool bEnhancedInputRetrievedFromController = TryGetEnhancedInputSubsystemFromController(PlayerController, EnhancedInput);

	if(!bEnhancedInputRetrievedFromController)
	{
		return EUDSuccessStatus::Failure;
	}
	
	if (bClearPrevious)
	{
		EnhancedInput->ClearAllMappings();
	}

	TArray<int32> FailedIndices;
	for (int32 Index = 0; Index < Contexts.Num(); ++Index)
	{
		const auto& [InputContext, Priority] = Contexts[Index];
		if (const UInputMappingContext* MappingContext = InputContext.LoadSynchronous())
		{
			EnhancedInput->AddMappingContext(MappingContext, Priority);
		}
		else
		{
			FailedIndices.Add(Index);
		}
	}

	if (FailedIndices.Num() > 0)
	{
		FString FailedIndicesStr = FString::JoinBy(FailedIndices, TEXT(", "), [](const int32 Index) { return FString::Printf(TEXT("%d"), Index); });
		UE_LOGFMT(LogUDCore, Warning, "{FailedIndicies} Input Mapping Contexts failed to load and were not added! The failed indexes are [{FailedIndicieIndexes}]", FailedIndices.Num(), FailedIndicesStr);
	}

	UE_LOGFMT(LogUDCore, Verbose, "{MappingCount} Input mapping contexts set successfully.", Contexts.Num() - FailedIndices.Num());
	return EUDSuccessStatus::Success;
}

EUDSuccessStatus UUDCoreInputFunctionLibrary::RemoveInputMappingContexts(
	AController* PlayerController,
	const TArray<TSoftObjectPtr<UInputMappingContext>>& Contexts)
{
	if (Contexts.IsEmpty()) { return EUDSuccessStatus::Failure;; }

	UEnhancedInputLocalPlayerSubsystem* EnhancedInput;
	const bool bEnhancedInputRetrievedFromController = TryGetEnhancedInputSubsystemFromController(PlayerController, EnhancedInput);

	if(!bEnhancedInputRetrievedFromController)
	{
		return EUDSuccessStatus::Failure;
	}
	
	TArray<int32> FailedIndices;
	for (int32 Index = 0; Index < Contexts.Num(); ++Index)
	{
		const TSoftObjectPtr<UInputMappingContext>& Context = Contexts[Index];
		if (const UInputMappingContext* MappingContext = Context.LoadSynchronous())
		{
			EnhancedInput->RemoveMappingContext(MappingContext);
		} else
		{
			FailedIndices.Add(Index);
		}
	}

	if (FailedIndices.Num() > 0)
	{
		FString FailedIndicesStr = FString::JoinBy(FailedIndices, TEXT(", "), [](const int32 Index) { return FString::Printf(TEXT("%d"), Index); });
		UE_LOGFMT(LogUDCore, Warning, "{FailedIndicies} Input Mapping Contexts failed to load and were not removed! The failed indexes are [{FailedIndicieIndexes}]", FailedIndices.Num(), FailedIndicesStr);
	}

	UE_LOGFMT(LogUDCore, Verbose, "{MappingCount} Input mapping contexts removed successfully.", Contexts.Num() - FailedIndices.Num());
	return EUDSuccessStatus::Success;;
}

EUDSuccessStatus UUDCoreInputFunctionLibrary::SwapInputMappingContexts(
 AController* PlayerController,
 const TSoftObjectPtr<UInputMappingContext> PreviousContext,
 const TSoftObjectPtr<UInputMappingContext> NewContext,
 const int32 Priority,
 const bool bUsePreviousPriority)
{
	const UInputMappingContext* LoadedPreviousMappingContext = PreviousContext.LoadSynchronous();
	const UInputMappingContext* LoadedNewMappingContext = NewContext.LoadSynchronous();

	if (!LoadedPreviousMappingContext || !LoadedNewMappingContext)
	{
		UE_LOGFMT(LogUDCore, Warning, "Both the previous and new input mapping contexts must be valid.");
		return EUDSuccessStatus::Failure;
	}

	UEnhancedInputLocalPlayerSubsystem* EnhancedInput;
	if (!TryGetEnhancedInputSubsystemFromController(PlayerController, EnhancedInput))
	{
		return EUDSuccessStatus::Failure;
	}

	if (int32 PreviousPriority; EnhancedInput->HasMappingContext(LoadedPreviousMappingContext, PreviousPriority))
	{
		const int32 TargetPriority = bUsePreviousPriority ? PreviousPriority : Priority;
		EnhancedInput->RemoveMappingContext(LoadedPreviousMappingContext);
		EnhancedInput->AddMappingContext(LoadedNewMappingContext, TargetPriority);
		UE_LOGFMT(LogUDCore, Verbose, "Input mapping contexts {PreviousContext} and {NewContext} swapped successfully at priority {Priority}.", LoadedPreviousMappingContext->GetName(), LoadedNewMappingContext->GetName(), TargetPriority);
	}
	else
	{
		EnhancedInput->AddMappingContext(LoadedNewMappingContext, Priority);
		UE_LOGFMT(LogUDCore, Warning, "Previous input mapping context {PreviousContext} not found. New context {NewContext} added at priority {BackupPriority}.", LoadedPreviousMappingContext->GetName(), LoadedNewMappingContext->GetName(), Priority);
	}

	return EUDSuccessStatus::Success;
}