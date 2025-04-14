/**
  * @author cxx2020@outlook.com
  * @date 10:24 PM
 */

using UnrealBuildTool;

[SupportedPlatforms(UnrealPlatformClass.Desktop)]
[SupportedPlatforms("LinuxArm64")]
public class XiaoCoordiServiceTarget : TargetRules
{
	public XiaoCoordiServiceTarget(TargetInfo target) : base(target)
	{
		Type = TargetType.Program;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		LinkType = TargetLinkType.Monolithic;
		SolutionDirectory = "Programs/XiaoBuild";
		LaunchModuleName = "XiaoCoordiService";
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

		bIsBuildingConsoleApplication = true;
	}
}
