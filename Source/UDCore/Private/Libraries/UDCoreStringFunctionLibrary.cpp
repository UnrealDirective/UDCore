// © 2024 Unreal Directive. All rights reserved.


#include "Libraries/UDCoreStringFunctionLibrary.h"

bool UUDCoreStringFunctionLibrary::ContainsLetters(const FString& String)
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

bool UUDCoreStringFunctionLibrary::ContainsNumbers(const FString& String)
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

bool UUDCoreStringFunctionLibrary::ContainsSpaces(const FString& String)
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

bool UUDCoreStringFunctionLibrary::ContainsSpecialCharacters(const FString& String)
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

FString UUDCoreStringFunctionLibrary::FilterCharacters(
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

TArray<FString> UUDCoreStringFunctionLibrary::SortStringArray(TArray<FString> StringArray)
{
	Algo::Sort(StringArray);
	return StringArray;
}
