// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Evera : ModuleRules
{
	public Evera(ReadOnlyTargetRules Target) : base(Target)
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
			"Evera",
			"Evera/Variant_Platforming",
			"Evera/Variant_Platforming/Animation",
			"Evera/Variant_Combat",
			"Evera/Variant_Combat/AI",
			"Evera/Variant_Combat/Animation",
			"Evera/Variant_Combat/Gameplay",
			"Evera/Variant_Combat/Interfaces",
			"Evera/Variant_Combat/UI",
			"Evera/Variant_SideScrolling",
			"Evera/Variant_SideScrolling/AI",
			"Evera/Variant_SideScrolling/Gameplay",
			"Evera/Variant_SideScrolling/Interfaces",
			"Evera/Variant_SideScrolling/UI"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
