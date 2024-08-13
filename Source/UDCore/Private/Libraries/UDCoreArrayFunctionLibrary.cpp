// © 2024 Unreal Directive. All rights reserved.


#include "Libraries/UDCoreArrayFunctionLibrary.h"

int32 UUDCoreArrayFunctionLibrary::Array_NextIndex(const TArray<int32>& TargetArray, const int32 Index, const bool bLoop)
{
	// We should never hit this function, as the custom thunk layer should be calling the native handler directly.
	check(0);
	return 0;
}

int32 UUDCoreArrayFunctionLibrary::Array_PreviousIndex(
	const TArray<int32>& TargetArray,
	const int32 Index,
	const bool bLoop)
{
	// We should never hit this function, as the custom thunk layer should be calling the native handler directly.
	check(0);
	return 0;
}

int32 UUDCoreArrayFunctionLibrary::GenericArray_NextIndex(
	const void* TargetArray,
	const FArrayProperty* ArrayProperty,
	const int32 Index,
	const bool bLoop)
{
	if (!TargetArray)
	{
		return INDEX_NONE;
	}

	const FScriptArrayHelper ArrayHelper(ArrayProperty, TargetArray);
	const int32 NextIndex = Index + 1;

	if(ArrayHelper.Num() == 0)
	{
		return INDEX_NONE;
	}

	if(NextIndex < 0)
	{
		return 0;
	}

	if (NextIndex <= ArrayHelper.Num() - 1)
	{
		return NextIndex;
	}

	if (bLoop)
	{
		return 0;
	}
	
	return Index;
}

int32 UUDCoreArrayFunctionLibrary::GenericArray_PreviousIndex(
	const void* TargetArray,
	const FArrayProperty* ArrayProperty,
	const int32 Index,
	const bool bLoop)
{
	if (!TargetArray)
	{
		return INDEX_NONE;
	}

	const FScriptArrayHelper ArrayHelper(ArrayProperty, TargetArray);
	const int32 PreviousIndex = Index - 1;

	if(ArrayHelper.Num() == 0)
	{
		return INDEX_NONE;
	}
	
	if (PreviousIndex >= 0)
	{
		return PreviousIndex;
	}
	
	if (bLoop)
	{
		return ArrayHelper.Num() - 1;
	}
	
	return Index;
}
