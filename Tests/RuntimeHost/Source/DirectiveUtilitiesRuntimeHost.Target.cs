using UnrealBuildTool;

public class DirectiveUtilitiesRuntimeHostTarget : TargetRules
{
	public DirectiveUtilitiesRuntimeHostTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.Latest;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		ExtraModuleNames.Add("DirectiveUtilitiesRuntimeHost");
		if (Configuration != UnrealTargetConfiguration.Shipping)
		{
			bBuildDeveloperTools = true;
			bForceCompileDevelopmentAutomationTests = true;
			ExtraModuleNames.Add("DirectiveUtilitiesRuntimeHostTests");
		}
	}
}
