// © 2024 Unreal Directive. All rights reserved.


#include "Libraries/UDCoreTextFunctionLibrary.h"

bool UUDCoreTextFunctionLibrary::IsNotEmpty(const FText& Text)
{
	return !Text.IsEmpty();
}
