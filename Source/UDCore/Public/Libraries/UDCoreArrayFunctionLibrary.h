// © 2024 Unreal Directive. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "UDCoreArrayFunctionLibrary.generated.h"

/**
 * UUDCoreArrayFunctionLibrary
 * A collection of array utility functions that improve the usability of arrays in Blueprints.
 */
UCLASS()
class UDCORE_API UUDCoreArrayFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	/**
	* Returns the next index in the array.
	* If the next index is greater than the last array index and bLoop is enabled, the index will loop back to the start of the array.
	* Otherwise, the last index will be returned.
	 * @param TargetArray - The array to get the next index for.
	 * @param Index - The current index.
	 * @param bLoop - If true, the index will loop back to the beginning of the array.
	 * @returns The next index in the array. If at the end and bLoop is false, the last index is returned.
	 */
	UFUNCTION(BlueprintPure, CustomThunk, meta=(DisplayName = "Next Index", CompactNodeTitle = "NEXT INDEX", ArrayParm = "TargetArray", BlueprintThreadSafe), Category="UDCore|Array")
	static int32 Array_NextIndex(const TArray<int32>& TargetArray, const int32 Index, const bool bLoop);

	/**
	 * Returns the previous index in the array.
	 * If the previous index is less than 0 and bLoop is enabled, the index will loop back to the end of the array.
	 * Otherwise, 0 will be returned.
	 * @param TargetArray - The array to get the previous index for.
	 * @param Index - The current index.
	 * @param bLoop - If true, the index will loop back to the end of the array.
	 * @returns The previous index in the array. If at the beginning and bLoop is false, the first index is returned.
	 */
	UFUNCTION(BlueprintPure, CustomThunk, meta=(DisplayName = "Previous Index", CompactNodeTitle = "PREV INDEX", ArrayParm = "TargetArray", BlueprintThreadSafe), Category="UDCore|Array")
	static int32 Array_PreviousIndex(const TArray<int32>& TargetArray, const int32 Index, const bool bLoop);

	
	/*~
	 * Native functions that will be called by the below custom thunk layers, which read off the property address and call the appropriate native handler.
	 * Based off UKismetArrayLibrary implementation
	 ~*/ 

	static int32 GenericArray_NextIndex(const void* TargetArray, const FArrayProperty* ArrayProperty, int32 Index, bool bLoop);
	static int32 GenericArray_PreviousIndex(const void* TargetArray, const FArrayProperty* ArrayProperty, int32 Index, bool bLoop);

	/*~
	 * Custom thunk layers that read off the property address and call the appropriate native handler.
	 * Based off UKismetArrayLibrary implementation
	 ~*/ 

	DECLARE_FUNCTION(execArray_NextIndex)
	{
		Stack.MostRecentProperty = nullptr;
		Stack.StepCompiledIn<FArrayProperty>(nullptr);
		const void* ArrayAddr = Stack.MostRecentPropertyAddress;
		const FArrayProperty* ArrayProperty = CastField<FArrayProperty>(Stack.MostRecentProperty);
		if (!ArrayProperty)
		{
			Stack.bArrayContextFailed = true;
			return;
		}
		P_GET_PROPERTY(FIntProperty, Index);
		P_GET_UBOOL(bLoop);
		P_FINISH;
		P_NATIVE_BEGIN;

		*static_cast<int32*>(RESULT_PARAM) = GenericArray_NextIndex(ArrayAddr, ArrayProperty, Index, bLoop);
		P_NATIVE_END;
	}

	DECLARE_FUNCTION(execArray_PreviousIndex)
	{
		Stack.MostRecentProperty = nullptr;
		Stack.StepCompiledIn<FArrayProperty>(nullptr);
		const void* ArrayAddr = Stack.MostRecentPropertyAddress;
		const FArrayProperty* ArrayProperty = CastField<FArrayProperty>(Stack.MostRecentProperty);
		if (!ArrayProperty)
		{
			Stack.bArrayContextFailed = true;
			return;
		}
		P_GET_PROPERTY(FIntProperty, Index);
		P_GET_UBOOL(bLoop);
		P_FINISH;
		P_NATIVE_BEGIN;

		*static_cast<int32*>(RESULT_PARAM) = GenericArray_PreviousIndex(ArrayAddr, ArrayProperty, Index, bLoop);
		P_NATIVE_END;
	}
};
