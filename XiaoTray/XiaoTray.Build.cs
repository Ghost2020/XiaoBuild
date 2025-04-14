// Copyright Epic Games, Inc. All Rights Reserved.

using System.IO;
using UnrealBuildBase;
using UnrealBuildTool;

public class XiaoTray : ModuleRules
{
	public XiaoTray(ReadOnlyTargetRules Target) : base(Target)
	{
		CppStandard = CppStandardVersion.Latest;
		CStandard = CStandardVersion.Latest;
		OptimizeCode = CodeOptimization.InShippingBuildsOnly;

		if (Target.Platform.IsInGroup(UnrealPlatformGroup.Unix))
		{
			PrivateDependencyModuleNames.AddRange(new string[]
			{
				"HarfBuzz",
				"UElibPNG",
				"ICU"
			});
		}

		PrivateDependencyModuleNames.AddRange(new string[] {
			"Projects",
			"Redis",
			"XiaoCommon",
			"Boost",
			"RSA",
			"Qt"
		});

		PublicDefinitions.Add("XIAO_USE_BOOST");
		PrivateDefinitions.Add("USE_LZ4=1");

		PublicIncludePaths.AddRange(
			new string[]
			{
				"Runtime/Launch/Public",
				Path.Combine(ModuleDirectory, "Private/Platform"),
				"Programs/XiaoBuild/Xiao",
			}
		);

		PrivateIncludePaths.Add("Runtime/Launch/Private");

		if (Target.Platform.IsInGroup(UnrealPlatformGroup.Unix))
		{
			PrivateDependencyModuleNames.Add("UnixCommonStartup");
		}
	}
}
