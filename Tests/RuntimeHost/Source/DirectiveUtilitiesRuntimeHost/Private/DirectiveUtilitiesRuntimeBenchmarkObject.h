// Copyright (c) 2026 Unreal Directive. Licensed under the MIT License.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "DirectiveUtilitiesRuntimeBenchmarkObject.generated.h"

UCLASS()
class UDirectiveUtilitiesRuntimeBenchmarkObject : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY()
	TArray<bool> BoolValues;

	UPROPERTY()
	TArray<int32> IntegerValues;

	UPROPERTY()
	TArray<float> FloatValues;

	UPROPERTY()
	TArray<FVector> VectorValues;

	UPROPERTY()
	TArray<FString> StringValues;

	UPROPERTY()
	TArray<TObjectPtr<UObject>> ObjectValues;
};
