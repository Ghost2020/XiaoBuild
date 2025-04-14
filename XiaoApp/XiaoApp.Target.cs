// Copyright Xiao Studio, Inc. All Rights Reserved.

using UnrealBuildTool;

[SupportedPlatforms(UnrealPlatformClass.Desktop)]
[SupportedPlatforms("LinuxArm64")]
public class XiaoAppTarget : TargetRules
{
	public XiaoAppTarget(TargetInfo target) : base(target)
	{
		bCompileAgainstApplicationCore = true;
		bWarningsAsErrors = false;

		Type = TargetType.Program;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		LinkType = TargetLinkType.Monolithic;
		SolutionDirectory = "Programs/XiaoBuild";
		LaunchModuleName = "XiaoApp";

		UndecoratedConfiguration = UnrealTargetConfiguration.Shipping;

		bUseLoggingInShipping = true;
		bCompileAgainstCoreUObject = true;
		bForceBuildTargetPlatforms = true;
		bCompileICU = true;
		// bCompileWithPluginSupport = true;
		// bIncludePluginsForTargetPlatforms = true;
		// Make sure to get all code in SlateEditorStyle compiled in
		bBuildDeveloperTools = true;

		bCompileAgainstEngine = false;
		bCompilePython = false;
		bBuildRequiresCookedData = false;
		bBuildWithEditorOnlyData = false;
		bCompileCEF3 = false;

		bHasExports = false;

		bIsBuildingConsoleApplication = false;
		// WindowsPlatform.ApplicationIcon = "";
	}
}
