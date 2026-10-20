// Copyright: Elixir Studios

using UnrealBuildTool;
using System.Collections.Generic;

public class WallRunEditorTarget : TargetRules
{
	public WallRunEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.V7;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_8;
		ExtraModuleNames.Add("WallRun");
	}
}
