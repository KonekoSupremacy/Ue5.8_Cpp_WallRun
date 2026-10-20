// Copyright: Elixir Studios

using UnrealBuildTool;
using System.Collections.Generic;

public class WallRunTarget : TargetRules
{
	public WallRunTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.V7;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_8;
		ExtraModuleNames.Add("WallRun");
	}
}
