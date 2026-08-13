// Copyright (c) 2026 Unreal Directive. Licensed under the MIT License.

#if WITH_EDITOR

#include "AssetRegistry/AssetData.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Editor.h"
#include "Libraries/DirectiveUtilEditorAssetLibrary.h"
#include "Engine/World.h"
#include "Misc/AutomationTest.h"
#include "Misc/Guid.h"
#include "ObjectTools.h"
#include "Subsystems/EditorAssetSubsystem.h"
#include "Tests/DirectiveUtilTestObject.h"
#include "UObject/ObjectRedirector.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDirectiveUtilEditorAssetLibraryTest, "DirectiveUtilities.EditorAssetLibraryTests", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDirectiveUtilEditorAssetLibraryTest::RunTest(const FString& Parameters)
{
	EDirectiveUtilSuccessStatus Status = EDirectiveUtilSuccessStatus::Success;

	const FString DefaultName = UDirectiveUtilEditorAssetLibrary::GetDefaultAssetNameForClass(nullptr, Status);
	TestEqual("GetDefaultAssetNameForClass should fail for a null class", Status, EDirectiveUtilSuccessStatus::Failure);
	TestTrue("GetDefaultAssetNameForClass should return an empty name for a null class", DefaultName.IsEmpty());

	Status = EDirectiveUtilSuccessStatus::Success;
	const TArray<FAssetData> NoAssets = UDirectiveUtilEditorAssetLibrary::GetAssetsByClass(nullptr, TEXT("/Game"), false, true, Status);
	TestEqual("GetAssetsByClass should fail for a null class", Status, EDirectiveUtilSuccessStatus::Failure);
	TestEqual("GetAssetsByClass should return no assets for a null class", NoAssets.Num(), 0);

	Status = EDirectiveUtilSuccessStatus::Success;
	const TArray<FString> NoDependencies = UDirectiveUtilEditorAssetLibrary::GetAssetDependencies(FAssetData(), false, Status);
	TestEqual("GetAssetDependencies should fail for an invalid asset", Status, EDirectiveUtilSuccessStatus::Failure);
	TestEqual("GetAssetDependencies should return nothing for an invalid asset", NoDependencies.Num(), 0);

	Status = EDirectiveUtilSuccessStatus::Success;
	const TArray<FString> NoReferencers = UDirectiveUtilEditorAssetLibrary::GetAssetReferencers(FAssetData(), false, Status);
	TestEqual("GetAssetReferencers should fail for an invalid asset", Status, EDirectiveUtilSuccessStatus::Failure);
	TestEqual("GetAssetReferencers should return nothing for an invalid asset", NoReferencers.Num(), 0);

	Status = EDirectiveUtilSuccessStatus::Failure;
	UDirectiveUtilEditorAssetLibrary::GetAssetsByClass(UWorld::StaticClass(), TEXT("/Game"), false, true, Status);
	TestEqual("GetAssetsByClass should succeed for a valid class query", Status, EDirectiveUtilSuccessStatus::Success);

	const TMap<FDirectiveUtilAssetKey, FDirectiveUtilDuplicateAssetData> DuplicateAssets =
		UDirectiveUtilEditorAssetLibrary::FindDuplicateAssets({TEXT("/Engine/BasicShapes"), TEXT("/Engine/BasicShapes")}, false);
	for (const TPair<FDirectiveUtilAssetKey, FDirectiveUtilDuplicateAssetData>& Pair : DuplicateAssets)
	{
		TSet<FString> UniquePaths;
		for (const FString& AssetPath : Pair.Value.DuplicateAssetPaths)
		{
			UniquePaths.Add(AssetPath);
		}
		TestEqual("FindDuplicateAssets should return each asset path once", UniquePaths.Num(), Pair.Value.DuplicateAssetPaths.Num());
	}

	int32 RedirectorsProcessed = INDEX_NONE;
	TestEqual("FixUpRedirectorsInPaths succeeds when no redirectors match",
		UDirectiveUtilEditorAssetLibrary::FixUpRedirectorsInPaths(
			{TEXT("/Game/DirectiveUtilitiesTests/NoRedirectors")}, RedirectorsProcessed),
		EDirectiveUtilSuccessStatus::Success);
	TestEqual("No redirectors are reported for an empty path", RedirectorsProcessed, 0);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDirectiveUtilFixUpRedirectorsTest, "DirectiveUtilities.FixUpRedirectorsTests", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDirectiveUtilFixUpRedirectorsTest::RunTest(const FString& Parameters)
{
	UEditorAssetSubsystem* EditorAssetSubsystem = GEditor
		? GEditor->GetEditorSubsystem<UEditorAssetSubsystem>()
		: nullptr;
	if (!EditorAssetSubsystem)
	{
		AddError(TEXT("Editor asset subsystem unavailable for the redirector test."));
		return false;
	}

	const FString RootPath = FString::Printf(
		TEXT("/Game/DirectiveUtilitiesTests/Redirectors_%s"),
		*FGuid::NewGuid().ToString(EGuidFormats::Digits));
	const FString OriginalName = TEXT("OriginalAsset");
	const FString OriginalPackageName = RootPath / OriginalName;
	UPackage* Package = CreatePackage(*OriginalPackageName);
	UDirectiveUtilTestObject* Asset = NewObject<UDirectiveUtilTestObject>(
		Package, *OriginalName, RF_Public | RF_Standalone);
	FAssetRegistryModule::AssetCreated(Asset);

	const FString RenamedAssetName = TEXT("RenamedAsset");
	ObjectTools::FPackageGroupName PackageGroupName;
	PackageGroupName.PackageName = RootPath / RenamedAssetName;
	PackageGroupName.ObjectName = RenamedAssetName;
	TSet<UPackage*> RefusedPackages;
	FText RenameError;
	if (!ObjectTools::RenameSingleObject(
		Asset, PackageGroupName, RefusedPackages, RenameError, nullptr, true))
	{
		EditorAssetSubsystem->DeleteDirectory(RootPath);
		AddError(FString::Printf(TEXT("Failed to rename the redirector test asset: %s"), *RenameError.ToString()));
		return false;
	}

	const FString OriginalObjectPath = OriginalPackageName + TEXT(".") + OriginalName;
	UObjectRedirector* Redirector = FindObject<UObjectRedirector>(nullptr, *OriginalObjectPath);
	if (!Redirector)
	{
		EditorAssetSubsystem->DeleteDirectory(RootPath);
		AddError(TEXT("Renaming the test asset did not create a redirector."));
		return false;
	}
	TestNotNull("Renaming an asset creates a redirector", Redirector);

	AddExpectedMessagePlain(
		TEXT("Redirector fix-up requires an interactive editor session."),
		ELogVerbosity::Warning,
		EAutomationExpectedMessageFlags::Contains,
		1);
	int32 RedirectorsProcessed = INDEX_NONE;
	const EDirectiveUtilSuccessStatus Status = UDirectiveUtilEditorAssetLibrary::FixUpRedirectorsInPaths(
		{RootPath}, RedirectorsProcessed);
	TestEqual("Redirector fix-up fails closed in unattended runs", Status, EDirectiveUtilSuccessStatus::Failure);
	TestEqual("No redirector is reported as processed after a guarded run", RedirectorsProcessed, 0);
	TestTrue("Guarded fix-up leaves the redirector intact", EditorAssetSubsystem->DoesAssetExist(OriginalObjectPath));
	TestTrue("Renamed asset remains available", EditorAssetSubsystem->DoesAssetExist(
		RootPath / RenamedAssetName + TEXT(".") + RenamedAssetName));

	TestTrue("Redirector test assets are removed", EditorAssetSubsystem->DeleteDirectory(RootPath));
	return true;
}

#endif
