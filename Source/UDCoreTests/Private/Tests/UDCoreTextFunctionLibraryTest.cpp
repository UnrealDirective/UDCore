#include "Libraries/UDCoreTextFunctionLibrary.h"
#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FUDCoreTextFunctionLibraryTest, "UDCore.TextFunctionLibraryTests", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FUDCoreTextFunctionLibraryTest::RunTest(const FString& Parameters)
{
	// Test IsNotEmpty with non-empty text
	const FText NonEmptyText = FText::FromString(TEXT("Hello"));
	TestTrue("IsNotEmpty should return true for non-empty text", UUDCoreTextFunctionLibrary::IsNotEmpty(NonEmptyText));

	// Test IsNotEmpty with empty text
	const FText EmptyText = FText::GetEmpty();
	TestFalse("IsNotEmpty should return false for empty text", UUDCoreTextFunctionLibrary::IsNotEmpty(EmptyText));

	return true;
}