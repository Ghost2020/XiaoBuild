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
				// "Networking",
				"Protobuf",
				"Redis"
			}
		);
		
		if (Target.Platform == UnrealTargetPlatform.Win64)
		{
			PublicSystemLibraries.Add("Pdh.lib");
		}
	}
}
