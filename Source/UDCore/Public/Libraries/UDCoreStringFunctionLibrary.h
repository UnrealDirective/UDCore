// © 2024 Unreal Directive. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Types/UDCoreTypes.h"
#include "UDCoreStringFunctionLibrary.generated.h"

/**
 * UUDStringFunctionLibrary
 * A collection of helpful string utility functions that improve the usability of strings in Blueprints.
 */
UCLASS()
class UDCORE_API UUDCoreStringFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	/**
	 * Detect if the provided string contains any letters.
	 * @param String - The string to check.
	 * @returns True if the string contains letters.
	 */
	UFUNCTION(BlueprintPure, Category = "UDCore|String" )
	static bool ContainsLetters(const FString& String);

	/**
	 * Detect if the provided string contains any numbers.
	 * @param String - The string to check.
	 * @returns True if the string contains numbers.
	 */
	UFUNCTION(BlueprintPure, Category = "UDCore|String" )
	static bool ContainsNumbers(const FString& String);
	
	/**
	 * Detect if the provided string contains any spaces.
	 * @param String - The string to check.
	 * @returns True if the string contains spaces.
	 */
	UFUNCTION(BlueprintPure, Category = "UDCore|String" )
	static bool ContainsSpaces(const FString& String);
	
	/**
	 * Detect if the provided string contains any special characters.
	 * @param String - The string to check.
	 * @returns True if the string contains special characters.
	 */
	UFUNCTION(BlueprintPure, Category = "UDCore|String" )
	static bool ContainsSpecialCharacters(const FString& String);

	/**
	 * Filter out characters types from the string.
	 * @param String - The string to filter.
	 * @param bLetters - If true, filter out letters.
	 * @param bNumbers - If true, filter out numbers.
	 * @param bSpecialCharacters - If true, filter out special characters.
	 * @param bSpaces - If true, filter out spaces.
	 */
	UFUNCTION(BlueprintPure, Category = "UDCore|String" )
	static FString FilterCharacters(const FString& String, const bool bLetters, const bool bNumbers, const bool bSpecialCharacters, const bool bSpaces);
	
	/** 
	* Sort a string array alphabetically.
	* @param StringArray - The string array to sort.
	*/
	UFUNCTION(BlueprintPure, Category = "UDCore|String", meta = (DeprecatedFunction, DeprecationMessage = "Use GetSortStringArray instead."))
	static TArray<FString> SortStringArray(TArray<FString> StringArray);

	/** 
	* Returns a sorted copy of the provided string array.
	* @param StringArray - The array of strings to sort.
	* @returns A sorted copy of the provided string array.
	*/
	UFUNCTION(BlueprintCallable, Category = "UDCore|String")
	static TArray<FString> GetSortedStringArray(TArray<FString> StringArray);
};
