// Copyright Epic Games, Inc. All Rights Reserved.

using System;
using UnrealBuildBase;
using UnrealBuildTool;

[SupportedPlatforms(UnrealPlatformClass.Desktop)]
[SupportedPlatforms("LinuxArm64")]
public class XiaoTrayTarget : TargetRules
{
	public XiaoTrayTarget(TargetInfo Target) : base(Target)
	{
		LaunchModuleName = "XiaoTray";

		Type = TargetType.Program;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		LinkType = TargetLinkType.Monolithic;
		bDeterministic = true;
		bWarningsAsErrors = false;

		UndecoratedConfiguration = UnrealTargetConfiguration.Shipping;
		bHasExports = false;

		// Lean and mean
		bBuildDeveloperTools = false;

		// Editor-only is enabled for desktop platforms to run unit tests that depend on editor-only data
		// It's disabled in test and shipping configs to make profiling similar to the game
		bBuildWithEditorOnlyData = false;

		// Currently this app is not linking against the engine, so we'll compile out references from Core to the rest of the engine
		bCompileAgainstEngine = false;
		bCompileAgainstCoreUObject = false;
		bCompileAgainstApplicationCore = false;

		// This app is a console application, not a Windows app (sets entry point to main(), instead of WinMain())
		bIsBuildingConsoleApplication = false;

		WindowsPlatform.TargetWindowsVersion = 0x0A00;
		bUseStaticCRT = false;

		SolutionDirectory = "Programs/XiaoBuild";

		bForceEnableExceptions = true;
		bUseLoggingInShipping = true;

		bUsePCHFiles = false; // if include Core must be false

		bCompileICU = true;
		bCompileFreeType = true;
		bUsesSlate = false;
	}
}
