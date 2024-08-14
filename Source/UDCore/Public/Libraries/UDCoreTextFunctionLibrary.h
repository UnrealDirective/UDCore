// © 2024 Unreal Directive. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "UDCoreTextFunctionLibrary.generated.h"

/**
 * UDCoreTextFunctionLibrary
 * A collection of helpful text utility functions that improve the usability of text in Blueprints.
 */
UCLASS()
class UDCORE_API UUDCoreTextFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	/**
	 * Returns true if the provided text is not empty.
	 * @param Text - The text to check.
	 */
	UFUNCTION(BlueprintPure, Category = "UDCore|Text" )
	static bool IsNotEmpty(const FText& Text);
};
