/**
  * @author cxx2020@outlook.com
  * @date 10:24 PM
 */


using System.IO;
using UnrealBuildBase;
using UnrealBuildTool;

[SupportedPlatforms(UnrealPlatformClass.Desktop)]
[SupportedPlatforms("LinuxArm64")]
public class XiaoAgentServiceTarget : TargetRules
{
	public XiaoAgentServiceTarget(TargetInfo target) : base(target)
	{
		Type = TargetType.Program;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		LinkType = TargetLinkType.Monolithic;
		SolutionDirectory = "Programs/XiaoBuild";
		LaunchModuleName = "XiaoAgentService";
		bUseLoggingInShipping = true;
		
		bUsesSlate = false;

		UndecoratedConfiguration = UnrealTargetConfiguration.Shipping;

		// Lean and mean
		bBuildDeveloperTools = false; 

		// Editor-only is enabled for desktop platforms to run unit tests that depend on editor-only data
		// It's disabled in test and shipping configs to make profiling similar to the game
		bool bDebugOrDevelopment = target.Configuration == UnrealTargetConfiguration.Debug || target.Configuration == UnrealTargetConfiguration.Development;
		bBuildWithEditorOnlyData = target.Platform.IsInGroup(UnrealPlatformGroup.Desktop) && bDebugOrDevelopment;

		// Currently this app is not linking against the engine, so we'll compile out references from Core to the rest of the engine
		bCompileAgainstEngine = false;
		bCompileAgainstCoreUObject = false;
		bCompileAgainstApplicationCore = false;
		bCompileICU = false;

		bIsBuildingConsoleApplication = true;
	}
}
