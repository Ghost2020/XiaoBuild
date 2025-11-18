// Copyright Xiao Studio, Inc. All Rights Reserved.

using UnrealBuildTool;
using EpicGames.Core;
using System.IO;

public class XiaoApp : ModuleRules
{
	public XiaoApp(ReadOnlyTargetRules target) : base(target)
	{
		CppStandard = CppStandardVersion.Latest;
		CStandard = CStandardVersion.Latest;
		OptimizeCode = CodeOptimization.InShippingBuildsOnly;

		PublicDefinitions.Add("XIAO_USE_BOOST");

		bEnableExceptions = true;

		PublicIncludePaths.AddRange(
			new string[]
			{
				"Runtime/Launch/Resources",
				"Runtime/Launch/Public",
				"Programs/XiaoBuild/Xiao"
			}
		);
		PrivateIncludePaths.AddRange(
			new string[]
			{
				"Runtime/Launch/Private",
				"Programs/XiaoBuild/XiaoApp/Private",
				"Programs/XiaoBuild/XiaoApp/Private/App/Slate"
			}
		);

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Projects",
				"RSA",
				"Json",
				"TraceInsights",
				"Sockets",
				"XiaoCommon"
			}
		);

		if(Target.Platform == UnrealTargetPlatform.LinuxArm64 || Target.Platform == UnrealTargetPlatform.Mac)
		{
			PrivateDefinitions.Add("USE_LZ4=1");
		}
		else
		{
			PrivateDefinitions.Add("USE_LZ4=0");
			PublicDependencyModuleNames.Add("zstd");
		}

		PrivateDependencyModuleNames.AddRange(
			new string[] {
				"AppFramework",
				"ApplicationCore",
				"Slate",
				"SlateCore",
				"StandaloneRenderer",
				"DesktopPlatform",
				"SourceCodeAccess",
				"InputCore",
				"Protobuf",
				"Redis",
				"Boost",
				"ToolWidgets",
				"InterchangeCore",
				"MessageLog",
				// "ImGui",
				"Networking",
				"ImageWrapper",
				"OodleDataCompression",
			}
		);

		if (Target.Platform == UnrealTargetPlatform.Win64)
		{
			PrivateDependencyModuleNames.Add("VisualStudioSourceCodeAccess");
		}
		else if (Target.Platform.IsInGroup(UnrealPlatformGroup.Unix))
		{
			PrivateDependencyModuleNames.Add("UnixCommonStartup"); 
			PrivateDependencyModuleNames.Add("VisualStudioCodeSourceCodeAccess");
		}
		else if (Target.Platform == UnrealTargetPlatform.Mac)
		{
			PrivateDependencyModuleNames.Add("XCodeSourceCodeAccess");
		}
	}
}
