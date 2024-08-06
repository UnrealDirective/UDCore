#pragma once

#include "CoreMinimal.h"
#include "UDCoreTypes.generated.h"

/**
 * Provides a list of success types.
 */
UENUM(BlueprintType)
enum class EUDSuccessStatus : uint8
{
	Success,
	Failure,
};