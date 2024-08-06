// © 2024 Unreal Directive. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Types/UDCoreInputTypes.h"
#include "Types/UDCoreTypes.h"
#include "UDCoreInputFunctionLibrary.generated.h"

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
};
