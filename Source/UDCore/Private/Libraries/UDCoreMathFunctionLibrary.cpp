#include "Libraries/UDCoreMathFunctionLibrary.h"

float UUDCoreMathFunctionLibrary::PerlinNoise2D(const FVector2D Position)
{
	return FMath::PerlinNoise2D(Position);
}

float UUDCoreMathFunctionLibrary::PerlinNoise3D(const FVector& Position)
{
	return FMath::PerlinNoise3D(Position);
}
