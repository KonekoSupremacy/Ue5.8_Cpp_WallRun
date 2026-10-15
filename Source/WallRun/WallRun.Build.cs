// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class WallRun : ModuleRules
{
	public WallRun(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"AIModule",
			"StateTreeModule",
			"GameplayStateTreeModule",
			"UMG",
			"Slate"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });

		PublicIncludePaths.AddRange(new string[] {
			"WallRun",
			"WallRun/Variant_Platforming",
			"WallRun/Variant_Platforming/Animation",
			"WallRun/Variant_Combat",
			"WallRun/Variant_Combat/AI",
			"WallRun/Variant_Combat/Animation",
			"WallRun/Variant_Combat/Gameplay",
			"WallRun/Variant_Combat/Interfaces",
			"WallRun/Variant_Combat/UI",
			"WallRun/Variant_SideScrolling",
			"WallRun/Variant_SideScrolling/AI",
			"WallRun/Variant_SideScrolling/Gameplay",
			"WallRun/Variant_SideScrolling/Interfaces",
			"WallRun/Variant_SideScrolling/UI"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
