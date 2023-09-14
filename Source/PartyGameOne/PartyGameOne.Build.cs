// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class PartyGameOne : ModuleRules
{
	public PartyGameOne(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "HeadMountedDisplay", "InputCore", "EnhancedInput", "WebSockets" });

	}
}
