// Copyright Unreal Directive. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
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

	//-----------------------------
	// Basic Functions
	//-----------------------------
	
	/**
	 * Returns a list of actor classes that are derived from the given base class.
	 * This exposes the built-in GetDerivedClasses function to blueprints.
	 * @param BaseClass The base class to get the derived classes for.
	 * @param bRecursive Whether to include derived classes of derived classes.
	 * @param DerivedClasses The list of derived classes.
	 */
	UFUNCTION(BlueprintPure, Category = "Unreal Directive Toolkit|Utility")
	static void GetChildClasses(const UClass* BaseClass, bool bRecursive, TArray<UClass*>& DerivedClasses);

	//-----------------------------
	// String Manipulation Functions
	//-----------------------------

	/**
	 * Detect if the provided string contains any letters.
	 * @param String - The string to check.
	 * @returns True if the string contains letters.
	 */
	UFUNCTION(BlueprintPure, Category = "Unreal Directive Toolkit|String" )
	static bool ContainsLetters(FString String);

	/**
	 * Detect if the provided string contains any numbers.
	 * @param String - The string to check.
	 * @returns True if the string contains numbers.
	 */
	UFUNCTION(BlueprintPure, Category = "Unreal Directive Toolkit|String" )
	static bool ContainsNumbers(FString String);
	
	/**
	 * Detect if the provided string contains any spaces.
	 * @param String - The string to check.
	 * @returns True if the string contains spaces.
	 */
	UFUNCTION(BlueprintPure, Category = "Unreal Directive Toolkit|String" )
	static bool ContainsSpaces(FString String);
	
	/**
	 * Detect if the provided string contains any special characters.
	 * @param String - The string to check.
	 * @returns True if the string contains special characters.
	 */
	UFUNCTION(BlueprintPure, Category = "Unreal Directive Toolkit|String" )
	static bool ContainsSpecialCharacters(FString String);

	/**
	 * Filter out characters types from the string.
	 * @param String - The string to filter.
	 * @param bLetters - If true, filter out letters.
	 * @param bNumbers - If true, filter out numbers.
	 * @param bSpecialCharacters - If true, filter out special characters.
	 * @param bSpaces - If true, filter out spaces.
	 */
	UFUNCTION(BlueprintPure, Category = "Unreal Directive Toolkit|String" )
	static FString FilterCharacters(FString String, bool bLetters, bool bNumbers, bool bSpecialCharacters, bool bSpaces);
	
	/** 
	* Sort a string array alphabetically.
	* @param StringArray - The string array to sort.
    */
    UFUNCTION(BlueprintPure, Category = "Unreal Directive Toolkit|String" )
	static TArray<FString> SortStringArray(TArray<FString> StringArray);

	//-----------------------------
	// Text Manipulation Functions
	//-----------------------------

	/**
	 * Check if the provided text is not empty.
	 * @param Text - The text to check.
	 */
	UFUNCTION(BlueprintPure, Category = "Unreal Directive Toolkit|Text" )
	static bool IsNotEmpty(const FText& Text);
};
