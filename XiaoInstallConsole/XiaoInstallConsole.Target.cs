// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

[SupportedPlatforms(UnrealPlatformClass.Desktop)]
[SupportedPlatforms("LinuxArm64")]
public class XiaoInstallConsoleTarget : TargetRules
{
	public XiaoInstallConsoleTarget(TargetInfo target) : base(target)
	{
		Type = TargetType.Program;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		LinkType = TargetLinkType.Monolithic;
		SolutionDirectory = "Programs/XiaoBuild";
		LaunchModuleName = "XiaoInstallConsole";
		bUseLoggingInShipping = true;
		
		bUsesSlate = false;

		UndecoratedConfiguration = UnrealTargetConfiguration.Shipping;

		// Lean and mean
		bBuildDeveloperTools = false;

		// Currently this app is not linking against the engine, so we'll compile out references from Core to the rest of the engine
		bCompileAgainstEngine = false;
		bCompileAgainstCoreUObject = false;
		bCompileAgainstApplicationCore = false;
		bCompileICU = false;
		bHasExports = false;

		bCompileWithPluginSupport = false;

		bIsBuildingConsoleApplication = true;

		if (target.Platform.IsInGroup(UnrealPlatformGroup.Windows))
		{
			AdditionalLinkerArguments = "/MANIFESTUAC:NO";
			WindowsPlatform.ManifestFile = "Programs/XiaoBuild/XiaoInstallConsole/XiaoInstallConsole.manifest";
		}
	}
}
