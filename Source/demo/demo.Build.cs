// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class demo : ModuleRules
{
	public demo(ReadOnlyTargetRules Target) : base(Target)
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
			"demo",
			"demo/Variant_Platforming",
			"demo/Variant_Platforming/Animation",
			"demo/Variant_Combat",
			"demo/Variant_Combat/AI",
			"demo/Variant_Combat/Animation",
			"demo/Variant_Combat/Gameplay",
			"demo/Variant_Combat/Interfaces",
			"demo/Variant_Combat/UI",
			"demo/Variant_SideScrolling",
			"demo/Variant_SideScrolling/AI",
			"demo/Variant_SideScrolling/Gameplay",
			"demo/Variant_SideScrolling/Interfaces",
			"demo/Variant_SideScrolling/UI"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
