using UnrealBuildTool;
using System.Collections.Generic;

public class ZeldaDiabloEditorTarget : TargetRules
{
	public ZeldaDiabloEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.Latest;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		ExtraModuleNames.Add("ZeldaDiablo");
	}
}

