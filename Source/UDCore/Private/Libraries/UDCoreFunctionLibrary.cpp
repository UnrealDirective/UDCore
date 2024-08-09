// Fill out your copyright notice in the Description page of Project Settings.


#include "Libraries/UDCoreFunctionLibrary.h"
#include "Algo/Sort.h"

void UUDCoreFunctionLibrary::GetChildClasses(const UClass* BaseClass, const bool bRecursive, TArray<UClass*>& DerivedClasses)
{
	GetDerivedClasses(BaseClass, DerivedClasses, bRecursive);
}

bool UUDCoreFunctionLibrary::ContainsLetters(const FString& String)
{
	for (const TCHAR& Char : String)
	{
		if (FChar::IsAlpha(Char))
		{
			return true;
		}
	}
	return false;
}

bool UUDCoreFunctionLibrary::ContainsNumbers(const FString& String)
{
	for (const TCHAR& Char : String)
	{
		if (FChar::IsDigit(Char))
		{
			return true;
		}
	}
	return false;
}

bool UUDCoreFunctionLibrary::ContainsSpaces(const FString& String)
{
	for (const TCHAR& Char : String)
	{
		if (FChar::IsWhitespace(Char))
		{
			return true;
		}
	}
	return false;
}

bool UUDCoreFunctionLibrary::ContainsSpecialCharacters(const FString& String)
{
	for (const TCHAR& Char : String)
	{
		if (FChar::IsPunct(Char))
		{
			return true;
		}
	}
	return false;
}

FString UUDCoreFunctionLibrary::FilterCharacters(
	const FString& String,
	const bool bLetters,
	const bool bNumbers,
	const bool bSpecialCharacters,
	const bool bSpaces)
{
	FString NewString;
	NewString.Reserve(String.Len());
	
	for (const TCHAR& Char : String)
	{
		if (bLetters && FChar::IsAlpha(Char)) continue;
		if (bNumbers && FChar::IsDigit(Char)) continue;
		if (bSpecialCharacters && FChar::IsPunct(Char)) continue;
		if (bSpaces && FChar::IsWhitespace(Char)) continue;
		NewString.AppendChar(Char);
	}
	return NewString;
}

TArray<FString> UUDCoreFunctionLibrary::SortStringArray(TArray<FString> StringArray)
{
	Algo::Sort(StringArray);
	return StringArray;
}

bool UUDCoreFunctionLibrary::IsNotEmpty(const FText& Text)
{
	return !Text.IsEmpty();
}
