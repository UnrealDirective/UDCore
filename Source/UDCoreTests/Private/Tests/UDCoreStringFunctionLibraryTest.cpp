#include "Libraries/UDCoreStringFunctionLibrary.h"
#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FUDCoreStringFunctionLibraryTest, "UDCore.StringFunctionLibraryTests", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FUDCoreStringFunctionLibraryTest::RunTest(const FString& Parameters)
{
    // Test ContainsLetters
    TestTrue("ContainsLetters should return true for 'Hello'", UUDCoreStringFunctionLibrary::ContainsLetters(TEXT("Hello")));
    TestFalse("ContainsLetters should return false for '1234'", UUDCoreStringFunctionLibrary::ContainsLetters(TEXT("1234")));

    // Test ContainsNumbers
    TestTrue("ContainsNumbers should return true for '1234'", UUDCoreStringFunctionLibrary::ContainsNumbers(TEXT("1234")));
    TestFalse("ContainsNumbers should return false for 'Hello'", UUDCoreStringFunctionLibrary::ContainsNumbers(TEXT("Hello")));

    // Test ContainsSpaces
    TestTrue("ContainsSpaces should return true for 'Hello World'", UUDCoreStringFunctionLibrary::ContainsSpaces(TEXT("Hello World")));
    TestFalse("ContainsSpaces should return false for 'HelloWorld'", UUDCoreStringFunctionLibrary::ContainsSpaces(TEXT("HelloWorld")));

    // Test ContainsSpecialCharacters
    TestTrue("ContainsSpecialCharacters should return true for 'Hello!'", UUDCoreStringFunctionLibrary::ContainsSpecialCharacters(TEXT("Hello!")));
    TestFalse("ContainsSpecialCharacters should return false for 'Hello'", UUDCoreStringFunctionLibrary::ContainsSpecialCharacters(TEXT("Hello")));

    // Test FilterCharacters
    const FString FilteredString = UUDCoreStringFunctionLibrary::FilterCharacters(TEXT("Hello123! "), true, true, true, true);
    TestEqual("FilterCharacters should return an empty string", FilteredString, TEXT(""));

    // Test GetSortedStringArray
    const TArray<FString> UnsortedArray = { TEXT("Banana"), TEXT("Apple"), TEXT("Cherry") };
    const TArray<FString> SortedArray = UUDCoreStringFunctionLibrary::GetSortedStringArray(UnsortedArray);
    TestEqual("GetSortedStringArray should return a sorted array", SortedArray, TArray<FString>({ TEXT("Apple"), TEXT("Banana"), TEXT("Cherry") }));

    return true;
}