/**
  * @author cxx2020@outlook.com
  * @date 10:24 PM
 */

using UnrealBuildTool;
using System.IO;

public class XiaoAgentService : ModuleRules
{
	public XiaoAgentService(ReadOnlyTargetRules Target) : base(Target)
	{
		CppStandard = CppStandardVersion.Latest;
		CStandard = CStandardVersion.Latest;
		OptimizeCode = CodeOptimization.InShippingBuildsOnly;

		bEnableExceptions = true;

		PublicIncludePaths.AddRange(new string []
		{
			"Runtime/Launch/Public",
			Path.Combine(ModuleDirectory, "Private/Platform"),
			Path.Combine(ModuleDirectory, "../Xiao"),
			Path.Combine(ModuleDirectory, "../Xiao/Platform")
		});

		PrivateIncludePaths.Add("Runtime/Launch/Private");

		PrivateDefinitions.Add("USE_XIAO_AGENT=1");

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Json",
				"RSA",
				"Json",
				"JsonUtilities",
				"XiaoCommon"
			}
		);

		PrivateDependencyModuleNames.AddRange(
			new string[] 
			{
				"Core",
				"Projects",
				"Protobuf",
				"Redis"
			}
		);
		
		if (Target.Platform == UnrealTargetPlatform.Win64)
		{
			PublicSystemLibraries.Add("Pdh.lib");
			PublicSystemLibraries.Add("dxgi.lib");
		}
	}
}
