// © 2024 Unreal Directive. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Types/UDCoreInputTypes.h"
#include "Types/UDCoreTypes.h"
#include "UDCoreInputFunctionLibrary.generated.h"

class UEnhancedInputLocalPlayerSubsystem;
/**
 * UUDCoreInputFunctionLibrary
 * A function library that adds functionality for working with input in Unreal Engine.
 */
UCLASS()
class UDCORE_API UUDCoreInputFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
	/**
	* Apply multiple Input Mapping Contexts.
	* @param PlayerController The player controller to add the contexts to. Will attempt to get the LocalPlayer from the controller.
	* @param Contexts The contexts to apply.
	* @param bClearPrevious Whether to clear all previous contexts before applying the new ones.
	* @returns Returns Success if the contexts were successfully applied.
	*/
	UFUNCTION(BlueprintCallable, Category = "UDCore|Input", meta=(ExpandEnumAsExecs="ReturnValue", DefaultToSelf="PlayerController"))
	static EUDSuccessStatus AddInputMappingContexts(
		AController* PlayerController,
		const TArray<FUDCoreEnhancedInputContextData>& Contexts,
		bool bClearPrevious);
	
	/**
	 * Remove multiple Input Mapping Contexts.
	 * @param PlayerController The player controller to remove the contexts from. Will attempt to get the LocalPlayer from the controller.
	 * @param Contexts The contexts to remove.
	 * @returns Returns Success if the contexts were successfully removed.
	 */
	UFUNCTION(BlueprintCallable, Category = "UDCore|Input", meta=(ExpandEnumAsExecs="ReturnValue", DefaultToSelf="PlayerController"))
	static EUDSuccessStatus RemoveInputMappingContexts(
		AController* PlayerController,
		const TArray<TSoftObjectPtr<UInputMappingContext>>& Contexts);
		
	/** 
	* Swap a designated Input Mapping Context with a new one.
	* If the previous context is found, it will be removed and the new context will be added.
	* If the previous context is not found, the new context will be added at the specified priority.
	* @param PlayerController The player controller to swap the contexts on. Will attempt to get the LocalPlayer from the controller.
	* @param PreviousContext The context to swap out.
	* @param NewContext The context to swap in.
	* @param Priority The priority to set the new context to.
	* @param bUsePreviousPriority Whether to use the previous context's priority when adding the new context.
	* @returns Returns Success if the contexts were successfully swapped.
    */
    UFUNCTION(BlueprintCallable, Category = "UDCore|Input", meta=(ExpandEnumAsExecs="ReturnValue", DefaultToSelf="PlayerController"))
	static EUDSuccessStatus SwapInputMappingContexts(
		AController* PlayerController,
		TSoftObjectPtr<UInputMappingContext> PreviousContext,
		TSoftObjectPtr<UInputMappingContext> NewContext,
		int32 Priority,
		bool bUsePreviousPriority);

protected:
	/**
	 * Attempt to get the Enhanced Input Subsystem from the provided controller.
	 * @param PlayerController - The player controller to get the subsystem from.
	 * @param EnhancedInput - The Enhanced Input Subsystem to return
	 * @return True if the subsystem was found.
	 */
	static bool TryGetEnhancedInputSubsystemFromController(
		AController* PlayerController,
		UEnhancedInputLocalPlayerSubsystem*& EnhancedInput);
};
