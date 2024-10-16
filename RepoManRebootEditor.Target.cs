// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class RepoManRebootEditorTarget : TargetRules
{
	public RepoManRebootEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
        //BuildEnvironment = TargetBuildEnvironment.Unique;
        bOverrideBuildEnvironment = true;
        DefaultBuildSettings = BuildSettingsVersion.V5;
		ExtraModuleNames.Add("RepoManReboot");

        if (Target.Platform == UnrealTargetPlatform.Win64)
            AdditionalCompilerArguments = "/std:c++20";
    }
}
