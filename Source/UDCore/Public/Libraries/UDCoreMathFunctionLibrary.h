// Copyright Unreal Directive. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "UDCoreMathFunctionLibrary.generated.h"

/**
 * UUDCoreMathFunctionLibrary
 *
 * Contains math functions for the UDCore plugin.
 */
UCLASS()
class UDCORE_API UUDCoreMathFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	
	/**
	* Returns a perlin noise value between -1 and 1 at the given position.
	* @note This exposes the built-in PerlinNoise2D function to blueprints.
	* @param Position - The position to get the noise value for.
	* @returns The noise value at the given position.
	*/
	UFUNCTION(BlueprintPure, Category = "UDCore|Math|Random")
	static float PerlinNoise2D(FVector2D Position);

	/**
	* Returns a perlin noise value between -1 and 1 at the given position.
	* @note This exposes the built-in PerlinNoise3D function to blueprints.
	* @param Position - The position to get the noise value for.
	* @returns The noise value at the given position.
	*/
	UFUNCTION(BlueprintPure, Category = "UDCore|Math|Random")
	static float PerlinNoise3D(const FVector& Position);
};
