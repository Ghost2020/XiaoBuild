// Copyright Epic Games, Inc. All Rights Reserved.

using System;
using UnrealBuildBase;
using UnrealBuildTool;

[SupportedPlatforms(UnrealPlatformClass.Desktop)]
[SupportedPlatforms("LinuxArm64")]
public class XiaoSchedulerTarget : TargetRules
{
	public XiaoSchedulerTarget(TargetInfo Target) : base(Target)
	{
		LaunchModuleName = "XiaoScheduler";
		bUsePCHFiles = false; // if include Core must be false
		UbaAgentTarget.CommonUbaSettings(this, Target);
		SolutionDirectory = "Programs/XiaoBuild";
		bWarningsAsErrors = false;
		bForceEnableExceptions = true;
		bUseStaticCRT = false;
	}
}
