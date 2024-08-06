// © 2024 Unreal Directive. All rights reserved.


#include "Libraries/UDCoreInputFunctionLibrary.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "UDCoreLogChannels.h"
#include "Logging/StructuredLog.h"

EUDSuccessStatus UUDCoreInputFunctionLibrary::AddInputMappingContexts(
	AController* PlayerController,
	const TArray<FUDCoreEnhancedInputContextData>& Contexts,
	const bool bClearPrevious)
{
	if (Contexts.IsEmpty()) { return EUDSuccessStatus::Failure; }

	if (!PlayerController)
	{
		UE_LOG(LogUDCore, Warning, TEXT("PlayerController is null. Cannot set input mapping contexts."));
		return EUDSuccessStatus::Failure;
	}
	
	const ULocalPlayer* LocalPlayer = Cast<APlayerController>(PlayerController)->GetLocalPlayer();
	if (!LocalPlayer)
	{
		UE_LOG(LogUDCore, Warning, TEXT("LocalPlayer not found. Cannot set input mapping contexts."));
		return EUDSuccessStatus::Failure;
	}

	UEnhancedInputLocalPlayerSubsystem* EnhancedInput = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();

	if(!EnhancedInput)
	{
		UE_LOGFMT(LogUDCore, Warning, "EnhancedInput subsystem not found. Cannot set input mapping contexts.");
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
		FString FailedIndicesStr;
		for (const int32 Index : FailedIndices)
		{
			if (Index > 1) { FailedIndicesStr += ", "; }
			FailedIndicesStr += FString::Printf(TEXT("%d"), Index);
		}
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

	if (!PlayerController)
	{
		UE_LOG(LogUDCore, Warning, TEXT("PlayerController is null. Cannot set input mapping contexts."));
		return EUDSuccessStatus::Failure;;
	}

	const ULocalPlayer* LocalPlayer = Cast<APlayerController>(PlayerController)->GetLocalPlayer();
	if (!LocalPlayer)
	{
		UE_LOG(LogUDCore, Warning, TEXT("LocalPlayer not found. Cannot set input mapping contexts."));
		return EUDSuccessStatus::Failure;;
	}

	UEnhancedInputLocalPlayerSubsystem* EnhancedInput = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();

	if(!EnhancedInput)
	{
		UE_LOG(LogUDCore, Warning, TEXT("EnhancedInput subsystem not found. Cannot remove input mapping contexts."));
		return EUDSuccessStatus::Failure;;
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
		FString FailedIndicesStr;
		for (const int32 Index : FailedIndices)
		{
			if (Index > 1) { FailedIndicesStr += ", "; }
			FailedIndicesStr += FString::Printf(TEXT("%d"), Index);
			
		}
		UE_LOGFMT(LogUDCore, Warning, "{FailedIndicies} Input Mapping Contexts failed to load and were not removed! The failed indexes are [{FailedIndicieIndexes}]", FailedIndices.Num(), FailedIndicesStr);
	}

	UE_LOGFMT(LogUDCore, Verbose, "{MappingCount} Input mapping contexts removed successfully.", Contexts.Num() - FailedIndices.Num());
	return EUDSuccessStatus::Success;;
}