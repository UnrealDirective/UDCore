// © 2024 Unreal Directive. All rights reserved.


#include "Libraries/UDCoreTextFunctionLibrary.h"

#include "Windows/WindowsPlatformApplicationMisc.h"


bool UUDCoreTextFunctionLibrary::IsNotEmpty(const FText& Text)
{
	return !Text.IsEmpty();
}
