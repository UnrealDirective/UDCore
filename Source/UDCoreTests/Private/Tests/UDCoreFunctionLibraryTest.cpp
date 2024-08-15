#include "Libraries/UDCoreFunctionLibrary.h"
#include "Tests/AutomationCommon.h"
#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FUDCoreFunctionLibraryTest, "UDCore.FunctionLibraryTests", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FUDCoreFunctionLibraryTest::RunTest(const FString& Parameters)
{
	// Test CopyTextToClipboard and GetTextFromClipboard
	const FText TestText = FText::FromString(TEXT("Hello, Clipboard!"));
	UUDCoreFunctionLibrary::CopyTextToClipboard(TestText);
	const FText ClipboardText = UUDCoreFunctionLibrary::GetTextFromClipboard();
	TestEqual("GetTextFromClipboard should return the copied text", ClipboardText.ToString(), TestText.ToString());

	ADD_LATENT_AUTOMATION_COMMAND(FEngineWaitLatentCommand(1.0f));

	// Test CopyStringToClipboard and GetStringFromClipboard
	const FString TestString = TEXT("Hello, Clipboard!");
	UUDCoreFunctionLibrary::CopyStringToClipboard(TestString);
	const FString ClipboardString = UUDCoreFunctionLibrary::GetStringFromClipboard();
	TestEqual("GetStringFromClipboard should return the copied string", ClipboardString, TestString);

	ADD_LATENT_AUTOMATION_COMMAND(FEngineWaitLatentCommand(1.0f));

	// Test ClearClipboard
	UUDCoreFunctionLibrary::ClearClipboard();
	const FString ClearedClipboardString = UUDCoreFunctionLibrary::GetStringFromClipboard();
	TestEqual("GetStringFromClipboard should return an empty string after clearing the clipboard", ClearedClipboardString, TEXT(""));

	// Test GetProjectVersion
	const FString ProjectVersion = UUDCoreFunctionLibrary::GetProjectVersion();
	TestNotEqual("GetProjectVersion should return a non-empty string", ProjectVersion, FString(""));

	return true;
}