// Copyright (c) 2026 Unreal Directive. Licensed under the MIT License.


#include "Libraries/DirectiveUtilSaveGameFunctionLibrary.h"
#include "Libraries/DirectiveUtilStringFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/SaveGame.h"
#include "HAL/FileManager.h"
#include "Misc/Paths.h"
#include "PlatformFeatures.h"
#include "SaveGameSystem.h"

#include <ctime>

namespace
{
	FString GetSaveGamesDirectory()
	{
		return FPaths::ProjectSavedDir() / TEXT("SaveGames");
	}

	FString GetSaveSlotFilePath(const FString& SlotName)
	{
		return GetSaveGamesDirectory() / (SlotName + TEXT(".sav"));
	}

	bool IsValidSaveSlotName(const FString& SlotName)
	{
		return UDirectiveUtilStringFunctionLibrary::IsValidFileName(SlotName);
	}

	ISaveGameSystem* GetSaveGameSystem()
	{
		return IPlatformFeaturesModule::Get().GetSaveGameSystem();
	}

	FDateTime ConvertUtcFileTimeToLocal(const FDateTime& UtcTimestamp)
	{
		const int64 UnixSeconds = UtcTimestamp.ToUnixTimestamp();
		const int32 Milliseconds = UtcTimestamp.GetMillisecond();
		const time_t Time = static_cast<time_t>(UnixSeconds);
		tm LocalTm;
#if PLATFORM_WINDOWS
		if (localtime_s(&LocalTm, &Time) != 0)
		{
			return UtcTimestamp;
		}
#else
		if (localtime_r(&Time, &LocalTm) == nullptr)
		{
			return UtcTimestamp;
		}
#endif
		return FDateTime(
			LocalTm.tm_year + 1900,
			LocalTm.tm_mon + 1,
			LocalTm.tm_mday,
			LocalTm.tm_hour,
			LocalTm.tm_min,
			LocalTm.tm_sec,
			Milliseconds);
	}

}

TArray<FString> UDirectiveUtilSaveGameFunctionLibrary::GetAllSaveSlotNames()
{
	TArray<FString> SlotNames;
	if (ISaveGameSystem* SaveSystem = GetSaveGameSystem())
	{
		if (SaveSystem->GetSaveGameNames(SlotNames, 0))
		{
			return SlotNames;
		}
	}

	TArray<FString> Files;
	IFileManager::Get().FindFiles(Files, *(GetSaveGamesDirectory() / TEXT("*.sav")), true, false);

	SlotNames.Reserve(Files.Num());
	for (const FString& File : Files)
	{
		SlotNames.Add(FPaths::GetBaseFilename(File));
	}
	return SlotNames;
}

bool UDirectiveUtilSaveGameFunctionLibrary::GetSaveSlotTimestamp(const FString& SlotName, FDateTime& OutTimestamp)
{
	OutTimestamp = FDateTime();
	if (!IsValidSaveSlotName(SlotName))
	{
		return false;
	}

	if (ISaveGameSystem* SaveSystem = GetSaveGameSystem())
	{
		if (!SaveSystem->DoesSaveGameExist(*SlotName, 0))
		{
			return false;
		}
	}

	const FDateTime Timestamp = IFileManager::Get().GetTimeStamp(*GetSaveSlotFilePath(SlotName));
	if (Timestamp == FDateTime::MinValue())
	{
		return false;
	}

	OutTimestamp = ConvertUtcFileTimeToLocal(Timestamp);
	return true;
}

bool UDirectiveUtilSaveGameFunctionLibrary::SaveGameToBytes(USaveGame* SaveGameObject, TArray<uint8>& OutBytes)
{
	OutBytes.Reset();
	if (!SaveGameObject)
	{
		return false;
	}
	return UGameplayStatics::SaveGameToMemory(SaveGameObject, OutBytes);
}

USaveGame* UDirectiveUtilSaveGameFunctionLibrary::LoadGameFromBytes(const TArray<uint8>& SaveData)
{
	if (SaveData.Num() == 0)
	{
		return nullptr;
	}
	return UGameplayStatics::LoadGameFromMemory(SaveData);
}

bool UDirectiveUtilSaveGameFunctionLibrary::DoesSaveSlotExist(const FString& SlotName, const int32 UserIndex)
{
	if (!IsValidSaveSlotName(SlotName))
	{
		return false;
	}
	return UGameplayStatics::DoesSaveGameExist(SlotName, UserIndex);
}

bool UDirectiveUtilSaveGameFunctionLibrary::DeleteSaveSlot(const FString& SlotName, const int32 UserIndex)
{
	if (!IsValidSaveSlotName(SlotName))
	{
		return false;
	}
	return UGameplayStatics::DeleteGameInSlot(SlotName, UserIndex);
}

bool UDirectiveUtilSaveGameFunctionLibrary::RenameSaveSlot(const FString& OldSlotName, const FString& NewSlotName, const int32 UserIndex)
{
	if (!IsValidSaveSlotName(OldSlotName) || !IsValidSaveSlotName(NewSlotName)
		|| OldSlotName.Equals(NewSlotName, ESearchCase::CaseSensitive))
	{
		return false;
	}

	const bool bCaseOnlyRename = OldSlotName.Equals(NewSlotName, ESearchCase::IgnoreCase);
	if (bCaseOnlyRename)
	{
		TArray<uint8> SaveData;
		if (!UGameplayStatics::LoadDataFromSlot(SaveData, OldSlotName, UserIndex) || SaveData.Num() == 0)
		{
			return false;
		}
		if (!UGameplayStatics::SaveDataToSlot(SaveData, NewSlotName, UserIndex))
		{
			return false;
		}
		// Slot-name case sensitivity belongs to the active platform backend. Deleting
		// the old spelling here can delete the newly written slot on a case-insensitive
		// filesystem, so treat both spellings as the same logical slot and rewrite it.
		return true;
	}

	if (!UGameplayStatics::DoesSaveGameExist(OldSlotName, UserIndex) || UGameplayStatics::DoesSaveGameExist(NewSlotName, UserIndex))
	{
		return false;
	}

	TArray<uint8> SaveData;
	if (!UGameplayStatics::LoadDataFromSlot(SaveData, OldSlotName, UserIndex))
	{
		return false;
	}
	if (!UGameplayStatics::SaveDataToSlot(SaveData, NewSlotName, UserIndex))
	{
		return false;
	}
	// If this delete fails the new copy is kept alongside the original, so the save is never lost.
	return UGameplayStatics::DeleteGameInSlot(OldSlotName, UserIndex);
}
