/**
  * @author cxx2020@outlook.com
  * @date 10:24 PM
 */

using UnrealBuildTool;
using System.IO;

public class XiaoCoordiService : ModuleRules
{
	public XiaoCoordiService(ReadOnlyTargetRules target) : base(target)
	{
		CppStandard = CppStandardVersion.Latest;
		CStandard = CStandardVersion.Latest;
		OptimizeCode = CodeOptimization.InShippingBuildsOnly;

		bEnableExceptions = true;

		PublicDefinitions.Add("XIAO_USE_BOOST");
		PublicDefinitions.Add("USE_LZ4");

		PublicIncludePaths.AddRange(new string []
		{
			"Runtime/Launch/Public",
			Path.Combine(ModuleDirectory, "Private/Platform"),
			Path.Combine(ModuleDirectory, "../Xiao")
		});
		PrivateIncludePaths.Add("Runtime/Launch/Private");

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Json",
				"RSA",
				"Networking",
				"Sockets",
				"XiaoCommon",
				"zstd"
			}
		);

		PrivateDependencyModuleNames.AddRange(
			new string[] 
			{
				"Core",
				"Protobuf",
				"Projects",
				"Redis",
				"Boost"
			}
		);

		if (Target.Platform == UnrealTargetPlatform.Win64)
		{
			PublicSystemLibraries.Add("Pdh.lib");
		}
	}
}
