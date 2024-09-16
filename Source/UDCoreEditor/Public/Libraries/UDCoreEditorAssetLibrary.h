// © 2024 Unreal Directive. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "EditorAssetLibrary.h"
#include "Types/UDCoreEditorAssetTypes.h"
#include "UDCoreEditorAssetLibrary.generated.h"

/**
 * UUDCoreEditorAssetLibrary
 *
 * An enhanced version of the Editor Asset Library.
 */
UCLASS()
class UDCOREEDITOR_API UUDCoreEditorAssetLibrary : public UEditorAssetLibrary
{
	GENERATED_BODY()

public:

	/**
	 * Retrieve a list of asset data for the given directory.
	 * @param DirectoryPath	Directory path of the asset we want the list from. (e.g., /Game/MyFolder or /MyPluginName/MyFolder)
	 * @param bRecursive The search will be recursive and will look in subfolders. Defaults to true.
	 * @return TArray<FAssetData> List of asset data.
	 */
	UFUNCTION(BlueprintCallable, Category = "UDCore | Editor Scripting | Asset")
	static TArray<FAssetData> GetAssetDataListFromDirectory(const FString& DirectoryPath, bool bRecursive = true);

	/**
	 * Find and return a list of duplicate assets within the given directories.
	 * The criteria for duplication is based on the asset name and class.
	 * @param DirectoryPaths List of directory paths to search for duplicate assets.
	 * (e.g., /Game/MyFolder or /MyPluginName/MyFolder)
	 * @param bRecursive The search will be recursive and will look in subfolders. Defaults to true.
	 * @return TMap<FUDAssetKey, FUDDuplicateAssetData> Mapped list of duplicate assets.
	 */
	UFUNCTION(BlueprintCallable, Category = "UDCore | Editor Scripting | Asset")
	static TMap<FUDAssetKey, FUDDuplicateAssetData> FindDuplicateAssets(
		const TArray<FString>& DirectoryPaths,
		bool bRecursive = true);
};
