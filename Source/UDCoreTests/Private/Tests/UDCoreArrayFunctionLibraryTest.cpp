#include "Libraries/UDCoreArrayFunctionLibrary.h"
#include "Tests/UDCoreTestObject.h"
#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FUDCoreArrayFunctionLibraryTest, "UDCore.ArrayFunctionLibraryTests", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FUDCoreArrayFunctionLibraryTest::RunTest(const FString& Parameters)
{
	// Create a test object
	UUDCoreTestObject* TestObject = NewObject<UUDCoreTestObject>();

	// Initialize the array
	TestObject->TestArray = {1, 2, 3, 4, 5};

	// Get the ArrayProperty
	FArrayProperty* ArrayProperty = FindFProperty<FArrayProperty>(UUDCoreTestObject::StaticClass(), GET_MEMBER_NAME_CHECKED(UUDCoreTestObject, TestArray));

	
	// Test generic array
	const int32 NextIndex = UUDCoreArrayFunctionLibrary::GenericArray_NextIndex(&TestObject->TestArray, ArrayProperty, 2, false);
	const int32 PreviousIndex = UUDCoreArrayFunctionLibrary::GenericArray_PreviousIndex(&TestObject->TestArray, ArrayProperty, 2, false);

	TestEqual("Array_NextIndex should return the next index in the array", NextIndex, 3);
	TestEqual("Array_PreviousIndex should return the previous index in the array", PreviousIndex, 1);

	// Test looped array
	const int32 NextIndexLooped = UUDCoreArrayFunctionLibrary::GenericArray_NextIndex(&TestObject->TestArray, ArrayProperty, 4, true);
	const int32 PreviousIndexLooped = UUDCoreArrayFunctionLibrary::GenericArray_PreviousIndex(&TestObject->TestArray, ArrayProperty, 0, true);

	TestEqual("Array_NextIndex should return the first index when looping", NextIndexLooped, 0);
	TestEqual("Array_PreviousIndex should return the last index when looping", PreviousIndexLooped, 4);

	// Test non-looped array
	const int32 NextIndexNonLooped = UUDCoreArrayFunctionLibrary::GenericArray_NextIndex(&TestObject->TestArray, ArrayProperty, 4, false);
	const int32 PreviousIndexNonLooped = UUDCoreArrayFunctionLibrary::GenericArray_PreviousIndex(&TestObject->TestArray, ArrayProperty, 0, false);

	TestEqual("Array_NextIndex should return the last index when not looping", NextIndexNonLooped, 4);
	TestEqual("Array_PreviousIndex should return the first index when not looping", PreviousIndexNonLooped, 0);

	// Test out of bounds
	const int32 NextIndexOutOfBounds = UUDCoreArrayFunctionLibrary::GenericArray_NextIndex(&TestObject->TestArray, ArrayProperty, 5, false);
	const int32 PreviousIndexOutOfBounds = UUDCoreArrayFunctionLibrary::GenericArray_PreviousIndex(&TestObject->TestArray, ArrayProperty, -1, false);

	TestEqual("Array_NextIndex should return the last index when out of bounds", NextIndexOutOfBounds, 4);
	TestEqual("Array_PreviousIndex should return the first index when out of bounds", PreviousIndexOutOfBounds, 0);

	return true;
}