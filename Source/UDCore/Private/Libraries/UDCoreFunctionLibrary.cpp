﻿// Fill out your copyright notice in the Description page of Project Settings.


#include "Libraries/UDCoreFunctionLibrary.h"
#include "HAL/PlatformApplicationMisc.h"
#include "Misc/ConfigCacheIni.h"

void UUDCoreFunctionLibrary::GetChildClasses(const UClass* BaseClass, const bool bRecursive, TArray<UClass*>& DerivedClasses)
{
	GetDerivedClasses(BaseClass, DerivedClasses, bRecursive);
}

void UUDCoreFunctionLibrary::CopyTextToClipboard(const FText& Text)
{
	const FString ClipboardText = Text.ToString();
	FPlatformApplicationMisc::ClipboardCopy(*ClipboardText);
}

void UUDCoreFunctionLibrary::CopyStringToClipboard(const FString& String)
{
	FPlatformApplicationMisc::ClipboardCopy(*String);
}

FText UUDCoreFunctionLibrary::GetTextFromClipboard()
{
	FString ClipboardText;
	FPlatformApplicationMisc::ClipboardPaste(ClipboardText);
	return FText::FromString(ClipboardText);
}

FString UUDCoreFunctionLibrary::GetStringFromClipboard()
{
	FString ClipboardText;
	FPlatformApplicationMisc::ClipboardPaste(ClipboardText);
	return ClipboardText;
}

void UUDCoreFunctionLibrary::ClearClipboard()
{
	FPlatformApplicationMisc::ClipboardCopy(TEXT(""));
}

FString UUDCoreFunctionLibrary::GetProjectVersion()
{
	FString ProjectVersion;
	GConfig->GetString(
		TEXT("/Script/EngineSettings.GeneralProjectSettings"),
		TEXT("ProjectVersion"),
		ProjectVersion,
		GGameIni);
	return ProjectVersion;
}
