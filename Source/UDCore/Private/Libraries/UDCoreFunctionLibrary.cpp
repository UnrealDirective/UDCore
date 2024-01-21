// Fill out your copyright notice in the Description page of Project Settings.


#include "Libraries/UDCoreFunctionLibrary.h"

void UUDCoreFunctionLibrary::GetChildClasses(const UClass* BaseClass, const bool bRecursive, TArray<UClass*>& DerivedClasses)
{
	GetDerivedClasses(BaseClass, DerivedClasses, bRecursive);
}

bool UUDCoreFunctionLibrary::ContainsLetters(FString String)
{
	for (int i = 0; i < String.Len(); i++)
	{
		if (isalpha(String[i]))
		{
			return true;
		}
	}
	return false;
}

bool UUDCoreFunctionLibrary::ContainsNumbers(FString String)
{
	for (int i = 0; i < String.Len(); i++)
	{
		if (isdigit(String[i]))
		{
			return true;
		}
	}
	return false;
}

bool UUDCoreFunctionLibrary::ContainsSpaces(FString String)
{
	for (int i = 0; i < String.Len(); i++)
	{
		if (isspace(String[i]))
		{
			return true;
		}
	}
	return false;
}

bool UUDCoreFunctionLibrary::ContainsSpecialCharacters(FString String)
{
	for (int i = 0; i < String.Len(); i++)
	{
		if (ispunct(String[i]))
		{
			return true;
		}
	}
	return false;
}

FString UUDCoreFunctionLibrary::FilterCharacters(
	FString String,
	const bool bLetters,
	const bool bNumbers,
	const bool bSpecialCharacters,
	const bool bSpaces)
{
	FString NewString;
	for (int i = 0; i < String.Len(); i++)
	{
		if (bLetters && isalpha(String[i])) continue;
		if (bNumbers && isdigit(String[i])) continue;
		if (bSpecialCharacters && ispunct(String[i])) continue;
		if (bSpaces && isspace(String[i])) continue;
		NewString.AppendChar(String[i]);
	}
	return NewString;
}

TArray<FString> UUDCoreFunctionLibrary::SortStringArray(TArray<FString> StringArray)
{
	StringArray.Sort();
	return StringArray;
}

bool UUDCoreFunctionLibrary::IsNotEmpty(const FText& Text)
{
	return !Text.IsEmpty();
}