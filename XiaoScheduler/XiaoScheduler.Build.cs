// Copyright Epic Games, Inc. All Rights Reserved.

using System.IO;
using UnrealBuildBase;
using UnrealBuildTool;

[SupportedPlatforms(UnrealPlatformClass.Desktop)]
public class XiaoScheduler : ModuleRules
{
	public XiaoScheduler(ReadOnlyTargetRules Target) : base(Target)
	{
		PrivatePCHHeaderFile = "../Core/Public/UbaCorePch.h";

		CppStandard = CppStandardVersion.Cpp20;
		CStandard = CStandardVersion.Latest;
		OptimizeCode = CodeOptimization.InShippingBuildsOnly;

		UnsafeTypeCastWarningLevel = WarningLevel.Error;
		StaticAnalyzerDisabledCheckers.Clear();

		PublicDefinitions.Add("XIAO_USE_BOOST");

		PrivateDependencyModuleNames.AddRange(new string[] {
			"Core",
			"Json",
			"RSA",
			"Protobuf",
			"UbaCore",
			"UbaCommon",
			"UbaVersion",
			"UbaMimalloc",
			"Redis",
			"Boost",
			"XiaoCommon"
		});

		PublicIncludePaths.AddRange(new string[] {
			"Programs/XiaoBuild/Xiao",
			"Programs/XiaoBuild/XiaoCommon/Public"
		});
	}
}
