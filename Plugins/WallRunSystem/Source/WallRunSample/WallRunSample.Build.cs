// Copyright KonekoSupremacy 2026 All Rights Reserved.

using UnrealBuildTool;

public class WallRunSample : ModuleRules
{
	public WallRunSample(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"GameplayAbilities",
			"GameplayTags",
			"GameplayTasks",
			"WallRunSystem"
		});
	}
}
