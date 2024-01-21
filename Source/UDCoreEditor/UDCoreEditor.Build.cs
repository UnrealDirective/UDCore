using UnrealBuildTool;

public class UDCoreEditor : ModuleRules
{
	public UDCoreEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"UnrealEd",
			}
		);

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
				"UDCore",
				"EditorFramework",
				"EditorScriptingUtilities",
			}
		);
	}
}