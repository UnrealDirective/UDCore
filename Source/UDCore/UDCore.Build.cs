// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class UDCore : ModuleRules
{
	public UDCore(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;


		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
			}
		);


		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
				"AIModule",
				"EnhancedInput",
				"ApplicationCore"
			}
		);


		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
			}
		);
	}
}