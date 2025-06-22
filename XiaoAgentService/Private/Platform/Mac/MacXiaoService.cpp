/**
  * @author cxx2020@outlook.com
  * @datetime 2023 -10:17 PM
 */
#include "MacXiaoService.h"
#include "../../AgentService.h"
#include "Misc/Paths.h"
#include "HAL/PlatformFilemanager.h"


static const FString SAgentServicePath = TEXT("/Library/LaunchDaemons/com.XiaoBuild.XiaoAgent.plist");
static const FString SAgentEnvPath = TEXT("/Library/LaunchDaemons/com.XiaoBuild.XiaoEnv.plist");


FMacBuildAgentService::FMacBuildAgentService(const FServiceCommandLineOptions& InOptions, const FServiceDesc& InServiceDesc)
	: FGenericService(InOptions, InServiceDesc)
{
	
}

bool FMacBuildAgentService::OnInstall()
{
	static const FString XmlAgentContent = TEXT(
		"<?xml version=\"1.0\" encoding = \"UTF-8\"?>\n"
		"<plist version = \"1.0\">\n"
		"\t<dict>\n"
		"\t\t<key>Label</key>\n"
		"\t\t<string>com.XiaoBuild.AgentService</string>\n"
		"\n"
		"\t\t<key>ProgramArguments</key>\n"
		"\t\t<array>\n"
		"\t\t\t<string>/Applications/XiaoApp.app/Contents/UE/Engine/Binaries/Mac/XiaoAgentService</string>\n"
		"\t\t\t<string></string>\n"
		"\t\t</array>\n"
		"\n"
		"\t\t<key>RunAtLoad</key>\n"
		"\t\t<true/>\n"
		"\n"
		"\t\t<key>KeepAlive</key>\n"
		"\t\t<true/>\n"
		"\n"
		"\t\t<key>StandardErrorPath</key>\n"
		"\t\t<string>/Library/Logs/XiaoAgentService.err</string>\n"
		"\n"
		"\t\t<key>StandardOutPath</key>\n"
		"\t\t<string>/Library/Logs/XiaoAgentService.log</string>\n"
		"\t</dict>\n"
		"</plist>"
	);

	static const FString XmlEnvContent = TEXT(
		"<?xml version="1.0" encoding="UTF-8"?>\n"
		"<!DOCTYPE plist PUBLIC \"-//Apple//DTD PLIST 1.0//EN\""
 		"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">\n"
		"<plist version=\"1.0\">\n"
		"<dict>\n"
    		"\t<key>Label</key>\n"
    		"\t<string>com.XiaoBuild.XiaoEnv</string>\n"
		"\n"
    		"\t<key>ProgramArguments</key>\n"
    		"\t<array>\n"
        		"\t\t<string>launchctl</string>\n"
       			"\t\t<string>setenv</string>\n"
        		"\t\t<string>XIAO_HOME</string>\n"
        		"\t\t<string>/Applications/XiaoApp.app/Contents/UE/Engine/Binaries/Mac</string>\n"
    		"\t</array>\n"
		"\n"
    		"\t<key>RunAtLoad</key>\n"
    		"\t<true/>\n"
		"</dict>\n"
		"</plist>\n"
	);

	return FFileHelper::SaveStringToFile(XmlAgentContent, *SAgentServicePath) && FFileHelper::SaveStringToFile(XmlEnvContent, *SAgentEnvPath);
}

bool FMacBuildAgentService::OnStart()
{
	if (FAgentService::OnInitialize(TEXT("")))
	{
		double DeltaTime = 0.0f;
		double LastTime = FPlatformTime::Seconds();

		while (!IsEngineExitRequested())
		{
			const float FloatDelta = static_cast<float>(DeltaTime);
			FAgentService::OnTick(FloatDelta);
			FTSTicker::GetCoreTicker().Tick(FloatDelta);

			FPlatformProcess::Sleep(FMath::Max<float>(0.0f, SIdleFrameTime - (FPlatformTime::Seconds() - LastTime)));

			const double CurrentTime = FPlatformTime::Seconds();
			DeltaTime = CurrentTime - LastTime;
			LastTime = CurrentTime;
		}

		FAgentService::OnDeinitialize();
		return true;
	}
	return false;
}

bool FMacBuildAgentService::OnStop()
{
	FAgentService::OnDeinitialize();
	return true;
}

bool FMacBuildAgentService::OnDelete()
{
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	if(FPaths::FileExists(SAgentServicePath))
	{
		PlatformFile.DeleteFile(*SAgentServicePath);
	}
	if(FPaths::FileExists(SAgentEnvPath))
	{
		PlatformFile.DeleteFile(*SAgentEnvPath);
	}
	return true;
}