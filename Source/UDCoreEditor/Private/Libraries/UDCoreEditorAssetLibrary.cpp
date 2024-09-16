// © 2024 Unreal Directive. All rights reserved.


#include "Libraries/UDCoreEditorAssetLibrary.h"
#include "Subsystems/EditorAssetSubsystem.h"
#include "Algo/Transform.h"

TArray<FAssetData> UUDCoreEditorAssetLibrary::GetAssetDataListFromDirectory(
	const FString& DirectoryPath,
	const bool bRecursive)
{
	TArray<FAssetData> AssetDataList;

	if (UEditorAssetSubsystem* EditorAssetSubsystem = GEditor->GetEditorSubsystem<UEditorAssetSubsystem>())
	{
		const TArray<FString> AssetPaths = EditorAssetSubsystem->ListAssets(DirectoryPath, bRecursive, false);

		Algo::Transform(AssetPaths, AssetDataList, [EditorAssetSubsystem](const FString& AssetPath) {
			return EditorAssetSubsystem->FindAssetData(AssetPath);
		});

		AssetDataList.RemoveAll([](const FAssetData& AssetData) {
			return !AssetData.IsValid();
		});
	}
	
	return AssetDataList;
}

TMap<FUDAssetKey, FUDDuplicateAssetData> UUDCoreEditorAssetLibrary::FindDuplicateAssets(
	const TArray<FString>& DirectoryPaths,
	const bool bRecursive)
{
	TArray<FAssetData> CombinedAssetDataList;

	// Retrieve asset data lists for all directories
	for (const FString& DirectoryPath : DirectoryPaths)
	{
		const TArray<FAssetData> AssetDataList = GetAssetDataListFromDirectory(DirectoryPath, bRecursive);
		CombinedAssetDataList.Append(AssetDataList);
	}

	// Map to group assets by their names and classes
	TMap<FUDAssetKey, FUDDuplicateAssetData> DuplicateAssetsMap;
	for (const FAssetData& AssetData : CombinedAssetDataList)
	{
		if(!AssetData.IsValid()) continue;
		
		const FString AssetName = AssetData.AssetName.ToString();
		const FString AssetClass = AssetData.AssetClassPath.ToString();
		const FString AssetPath = AssetData.GetSoftObjectPath().ToString();
		const FUDAssetKey AssetKey(AssetName, AssetClass);
		
		if (FUDDuplicateAssetData* ExistingData = DuplicateAssetsMap.Find(AssetKey))
		{
			ExistingData->DuplicateAssetPaths.Add(AssetPath);
		}
		else
		{
			FUDDuplicateAssetData DuplicateAssetData;
			DuplicateAssetData.AssetName = AssetName;
			DuplicateAssetData.AssetClass = AssetClass;
			DuplicateAssetData.DuplicateAssetPaths.Add(AssetPath);
			DuplicateAssetsMap.Emplace(AssetKey, MoveTemp(DuplicateAssetData));
		}
	}

	// Filter out entries that do not have duplicates
	TArray<FUDAssetKey> KeysToRemove;
	for (const auto& Pair : DuplicateAssetsMap)
	{
		if (Pair.Value.DuplicateAssetPaths.Num() <= 1)
		{
			KeysToRemove.Add(Pair.Key);
		}
	}

	for (const FUDAssetKey& Key : KeysToRemove)
	{
		DuplicateAssetsMap.Remove(Key);
	}

	return DuplicateAssetsMap;
}
