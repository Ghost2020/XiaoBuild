/**
  * @author cxx2020@outlook.com
  * @date 10:24 PM
 */

using UnrealBuildTool;
using System.IO;


[SupportedPlatforms(UnrealPlatformClass.Desktop)]
public class XiaoCommon : ModuleRules
{
	public XiaoCommon(ReadOnlyTargetRules target) : base(target)
	{
		CppStandard = CppStandardVersion.Latest;
		CStandard = CStandardVersion.Latest;
		OptimizeCode = CodeOptimization.InShippingBuildsOnly;

		PublicIncludePaths.AddRange(new string []
		{
			
		});

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Protobuf"
			}
		);

		PrivateDependencyModuleNames.AddRange(
			new string[] 
			{
				
			}
		);
	}
}
