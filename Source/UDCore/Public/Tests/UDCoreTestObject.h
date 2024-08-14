#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "UDCoreTestObject.generated.h"

UCLASS()
class UUDCoreTestObject : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY()
	TArray<int32> TestArray;
};