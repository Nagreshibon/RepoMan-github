// Copyright Epic Games, Inc. All Rights Reserved.
using System.IO;
using UnrealBuildTool;

public class RepoManRebootEditor : ModuleRules
{
	private string PluginsPath
	{
		get { return Path.GetFullPath(Path.Combine(ModuleDirectory, "../../Plugins/")); }
	}

	//protected void AddSPUD() {
	//	// Linker
	//	PrivateDependencyModuleNames.AddRange(new string[] { "SPUD" });
	//	// Headers
	//	PublicIncludePaths.Add(Path.Combine( PluginsPath, "SPUD", "Source", "SPUD", "Public"));
	//}

	public RepoManRebootEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        //CppStandard = CppStandardVersion.Cpp20; 

        PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "HeadMountedDisplay", "NavigationSystem", "AIModule", "GameplayTasks", "SystemsTemplatePlugin", "CinematicCamera", "CableComponent", "UMG", "Slate", "SlateCore", "RepoManReboot" });


        if (Target.bBuildEditor == true)
		{
			PublicDependencyModuleNames.AddRange(new string[] { "DesktopPlatform", "Blutility" });
		}

		//if(Target.bBuildEditor == false)
		//{
		//	PublicDependencyModuleNames.Remove("Blutility");
		//}

  //      AddSPUD();
	}
}