// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.IO;
using System.Collections.Generic;
using System.Linq;

public class Qt : ModuleRules
{
	public Qt(ReadOnlyTargetRules Target) : base(Target)
	{
		Type = ModuleType.External;

		string QtIncludePath = Path.Combine(ModuleDirectory, "include");
		PublicSystemIncludePaths.Add(QtIncludePath);

		string NamePrefix = "lib";
		string LibExtension = "";	
		string DllExtension = "";
		List<string> PlatformDlls = new();
		string PlatformName = Target.Platform.ToString();
		List<string> QtLibs = new();
		QtLibs.AddRange(new string[] { "Qt6Core", "Qt6Gui", "Qt6Widgets" });

		if (Target.Platform == UnrealTargetPlatform.Win64)
		{
			NamePrefix = "";
			LibExtension = ".lib";
			DllExtension = ".dll";
			PlatformDlls.Add("qwindows.dll");
		}
		else if (Target.IsInPlatformGroup((UnrealPlatformGroup.Unix)))
		{
			LibExtension = ".a";
			DllExtension = ".so";
			QtLibs.AddRange(new string[] { "qglib", "qgtk3" } );
		}
		else if (Target.Platform == UnrealTargetPlatform.Mac)
		{
			LibExtension = ".a";
			DllExtension = ".dylib";
			PlatformDlls.AddRange(new string[] { "qcocoa.dylib", "qminimal.dylib", "qoffscreen.dylib" });
			PlatformName = Target.Architecture == UnrealArch.X64 ? "Mac" : "MacArm64";
		}
		
		foreach (string LibName in QtLibs)
		{
			string QtLibPath = Path.Combine(ModuleDirectory, "lib", PlatformName);
			
			PublicAdditionalLibraries.Add(Path.Combine(QtLibPath, NamePrefix + LibName + LibExtension));
			if (Target.Platform != UnrealTargetPlatform.Mac)
			{
				string TargetFile = Path.Combine("$(TargetOutputDir)", NamePrefix + LibName);
				RuntimeDependencies.Add(TargetFile + DllExtension, Path.Combine(QtLibPath, NamePrefix + LibName + DllExtension));
			}
		}

		if (!Target.IsInPlatformGroup((UnrealPlatformGroup.Unix)))
		{
			foreach (string LibName in PlatformDlls)
			{
				string TargetFolder = "";
				if (Target.Platform != UnrealTargetPlatform.Mac)
				{
					TargetFolder = "$(TargetOutputDir)/platforms";
				}
				else
				{
					if (Target.Configuration == UnrealTargetConfiguration.Shipping)
					{
						TargetFolder = "$(TargetOutputDir)/XiaoTray.app/Contents/plugins/platforms";
					}
					else
					{
						TargetFolder = Path.Combine("$(TargetOutputDir)", "XiaoTray-Mac-" + Target.Configuration.ToString() + ".app", "Contents/plugins/platforms");
					}
				}
				RuntimeDependencies.Add(Path.Combine(TargetFolder, NamePrefix + LibName), Path.Combine(ModuleDirectory, "lib", PlatformName, NamePrefix + LibName));
			}
		}
	}
}
