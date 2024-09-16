#pragma once

#include "CoreMinimal.h"
#include "UDCoreEditorAssetTypes.generated.h"

USTRUCT(BlueprintType)
struct FUDAssetKey
{
    GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	FString AssetName;

	UPROPERTY(BlueprintReadOnly)
	FString AssetClass;

	FUDAssetKey(const FString& InAssetName, const FString& InAssetClass)
		: AssetName(InAssetName), AssetClass(InAssetClass) {}

	bool operator==(const FUDAssetKey& Other) const
	{
		return AssetName == Other.AssetName && AssetClass == Other.AssetClass;
	}

	friend uint32 GetTypeHash(const FUDAssetKey& Key)
	{
		return HashCombine(GetTypeHash(Key.AssetName), GetTypeHash(Key.AssetClass));
	}

	FUDAssetKey(): AssetName(FString()), AssetClass(FString()) {}
};

USTRUCT(BlueprintType)
struct FUDDuplicateAssetData
{
    GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	FString AssetName;

	UPROPERTY(BlueprintReadOnly)
	FString AssetClass;

	UPROPERTY(BlueprintReadOnly)
	TArray<FString> DuplicateAssetPaths;

	FUDDuplicateAssetData()
	{
		AssetName = FString();
		AssetClass = FString();
		DuplicateAssetPaths = TArray<FString>();
	}
};