using UnrealBuildTool;

public class UDCoreTests : ModuleRules
{
    public UDCoreTests(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

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
                "UDCore",
                "AutomationTest"
            }
        );
        
        if (Target.bBuildEditor)
        {
            PrivateDependencyModuleNames.Add("UDCoreEditor");
        }
    }
}