// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class RepoManRebootTarget : TargetRules
{
	public RepoManRebootTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
        bOverrideBuildEnvironment = true;
        DefaultBuildSettings = BuildSettingsVersion.V5;
        IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
        ExtraModuleNames.Add("RepoManReboot");

        if (Target.Platform == UnrealTargetPlatform.Win64)
            AdditionalCompilerArguments = "/std:c++20";
    }
}
