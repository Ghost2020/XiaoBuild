// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.IO;

public class Redis : ModuleRules
{
	public Redis(ReadOnlyTargetRules Target) : base(Target)
	{
		Type = ModuleType.External;

		PublicDependencyModuleNames.Add("Protobuf");
		PublicDependencyModuleNames.Add("Json");
		PrivateDependencyModuleNames.Add("OpenSSL");

		PublicSystemIncludePaths.Add(Path.Combine(ModuleDirectory, "redis-cpp/include"));
		PublicSystemIncludePaths.Add(Path.Combine(ModuleDirectory, "redis-cpp/include/sw/redis++/cxx17"));
		PublicSystemIncludePaths.Add(Path.Combine(ModuleDirectory, "redis-cpp/include/sw/redis++/tls"));

		bool UseRedisCplusCplus = false;

		var libraryPath = Path.Combine(ModuleDirectory, "redis-cpp/lib");

		if (Target.Platform == UnrealTargetPlatform.Win64)
		{
			libraryPath = Path.Combine(libraryPath, "Win64");
			PublicAdditionalLibraries.Add(Path.Combine(libraryPath, "hiredis.lib"));
			PublicAdditionalLibraries.Add(Path.Combine(libraryPath, "hiredis_ssl.lib"));
			if (UseRedisCplusCplus)
			{
				PublicAdditionalLibraries.Add(Path.Combine(libraryPath, "redis.lib"));
			}
			PublicAdditionalLibraries.Add(Path.Combine(libraryPath, "WS2_32.Lib"));
			PublicAdditionalLibraries.Add(Path.Combine(libraryPath, "NetAPI32.Lib"));
			PublicAdditionalLibraries.Add(Path.Combine(libraryPath, "Crypt32.lib"));
		}

		if (Target.IsInPlatformGroup(UnrealPlatformGroup.Unix) || Target.Platform == UnrealTargetPlatform.Mac)
		{
			if (Target.Platform == UnrealTargetPlatform.Linux)
			{
				libraryPath = Path.Combine(libraryPath, "Linux");
			}
			else if (Target.Platform == UnrealTargetPlatform.LinuxArm64)
			{
				libraryPath = Path.Combine(libraryPath, "LinuxArm64");
			}
			else if(Target.Platform == UnrealTargetPlatform.Mac)
			{
				libraryPath = Path.Combine(libraryPath, Target.Architecture == UnrealArch.X64 ? "Mac" : "MacArm64");
				PublicAdditionalLibraries.Add(Path.Combine(libraryPath, "libhiredis_ssl.a"));
			}

			PublicAdditionalLibraries.Add(Path.Combine(libraryPath, "libhiredis.a"));
			if (UseRedisCplusCplus)
			{
				PublicAdditionalLibraries.Add(Path.Combine(libraryPath, "libredis++.a"));
			}
		}
	}
}
