#pragma once

#include "CoreMinimal.h"
#include "UDCoreInputTypes.generated.h"

class UInputMappingContext;

// A structure for the input context
USTRUCT(BlueprintType)
struct FUDCoreEnhancedInputContextData
{
	GENERATED_BODY()

	/** The input context to be used. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	TSoftObjectPtr<UInputMappingContext> InputContext;

	/** The priority of the input context. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	int32 Priority;

	FUDCoreEnhancedInputContextData()
		: Priority(0)
	{
	}
};