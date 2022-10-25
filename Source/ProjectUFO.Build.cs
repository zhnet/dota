// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class ProjectUFO : ModuleRules
{
	public ProjectUFO(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "HeadMountedDisplay", "Networking", "Sockets", "ApexDestruction"});
	}
}
