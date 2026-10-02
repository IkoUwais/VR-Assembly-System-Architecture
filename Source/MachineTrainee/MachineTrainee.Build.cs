// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;

public class MachineTrainee : ModuleRules {
	
	public MachineTrainee(ReadOnlyTargetRules Target) : base(Target) {
		
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput", "HeadMountedDisplay", "DeveloperSettings", "NetCore", "GameplayTags" });

		PrivateDependencyModuleNames.AddRange(new string[] {  });

	}
	
}