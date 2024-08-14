// Copyright Unreal Directive. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "UDCoreFunctionLibrary.generated.h"

/**
 * UUDCoreFunctionLibrary
 *
 * The primary function library for the UDCore plugin.
 */
UCLASS()
class UDCORE_API UUDCoreFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
	/**
	 * Returns a list of actor classes that are derived from the given base class.
	 * This exposes the built-in GetDerivedClasses function to blueprints.
	 * @param BaseClass The base class to get the derived classes for.
	 * @param bRecursive Whether to include derived classes of derived classes.
	 * @param DerivedClasses The list of derived classes.
	 */
	UFUNCTION(BlueprintPure, Category = "UDCore|Utility")
	static void GetChildClasses(const UClass* BaseClass, bool bRecursive, TArray<UClass*>& DerivedClasses);

	/**
	 * Copy the provided text to the clipboard.
	 * @param Text - The text to copy to the clipboard.
	 */
	UFUNCTION(BlueprintCallable, Category = "UDCore|Clipboard" )
	static void CopyTextToClipboard(const FText& Text);

	/**
	 * Copy the provided string to the clipboard.
	 * @param String - The string to copy to the clipboard.
	 */
	UFUNCTION(BlueprintCallable, Category = "UDCore|Clipboard" )
	static void CopyStringToClipboard(const FString& String);

	/**
	 * Get the content from the clipboard as FText.
	 * @returns The text from the clipboard.
	 */
	UFUNCTION(BlueprintPure, Category = "UDCore|Clipboard" )
	static FText GetTextFromClipboard();

	/**
	 * Get the content from the clipboard as an FString.
	 * @returns The content from the clipboard as a string.
	 */
	UFUNCTION(BlueprintPure, Category = "UDCore|Clipboard" )
	static FString GetStringFromClipboard();

	/**
	 * Clear the clipboard.
	 */
	UFUNCTION(BlueprintCallable, Category = "UDCore|Clipboard")
	static void ClearClipboard();

	/**
	 * Get the project version as a string.
	 * @returns The project version as a string.
	 */
	UFUNCTION(BlueprintPure, Category = "UDCore|Utility")
	static FString GetProjectVersion();
	
};
