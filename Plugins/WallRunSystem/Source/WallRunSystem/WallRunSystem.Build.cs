// Copyright KonekoSupremacy 2026 All Rights Reserved.

using UnrealBuildTool;

public class WallRunSystem : ModuleRules
{
	public WallRunSystem(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"EnhancedInput",
			"GameplayAbilities",
			"GameplayTags",
			"GameplayTasks"
		});
	}
}
