/**
  * @author cxx2020@outlook.com
  * @date 10:24 PM
 */

using UnrealBuildTool;
using System.IO;

public class XiaoInstallConsole : ModuleRules
{
	public XiaoInstallConsole(ReadOnlyTargetRules target) : base(target)
	{
		CppStandard = CppStandardVersion.Latest;
		CStandard = CStandardVersion.Latest;
		OptimizeCode = CodeOptimization.InShippingBuildsOnly;

		bEnableExceptions = true;

		PublicDefinitions.Add("XIAO_USE_BOOST");
		PublicDefinitions.Add("USE_LZ4");

		PublicIncludePaths.AddRange(new string[]
		{
			"Runtime/Launch/Public",
			Path.Combine(ModuleDirectory, "../Xiao")
		});

		PrivateIncludePaths.Add("Runtime/Launch/Private");      // For LaunchEngineLoop.cpp include

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"zstd"
			}
		);

		PrivateDependencyModuleNames.AddRange(
			new string[] 
			{
				"Core",
				"Projects",
				"RSA",
				"Json",
				"Protobuf",
				"Boost",
				"XmlParser",
				"XiaoCommon",
				"Json",
				"Redis"
			}
		);
	}
}
