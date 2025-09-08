// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class RuntimeIslandGenerator : ModuleRules
{
	public RuntimeIslandGenerator(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PrivateIncludePaths.AddRange(
			new[]
			{
				"RuntimeIslandGenerator/"
			}
		);


		PublicDependencyModuleNames.AddRange(
			new[]
			{
				"Core"
			}
		);


		PrivateDependencyModuleNames.AddRange(
			new[]
			{
				"CoreUObject", "Engine", "Slate", "SlateCore",
				"ProceduralMeshComponent"
			}
		);
	}
}