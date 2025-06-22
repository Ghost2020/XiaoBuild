/**
  * @author cxx2020@outlook.com
  * @datetime 2023 -9:08 PM
 */
#include "MacXiaoService.h"
#include "CoordiService.h"
#include "Misc/Paths.h"
#include "HAL/PlatformFilemanager.h"

static const FString SCoordiServicePath = TEXT("/Library/LaunchDaemons/com.XiaoBuild.XiaoCoordi.plist");

FMacCoordiService::FMacCoordiService(const FServiceCommandLineOptions& InOptions, const FServiceDesc& InServiceDesc)
	: FGenericService(InOptions, InServiceDesc)
{
}

FMacCoordiService::~FMacCoordiService()
{
	
}

bool FMacCoordiService::OnInstall()
{
	static FString XmlContent = TEXT(
		"<?xml version=\"1.0\" encoding = \"UTF-8\"?>\n"
		"<plist version = \"1.0\">\n"
		"\t<dict>\n"
		"\t\t<key>Label</key>\n"
		"\t\t<string>com.XiaoBuild.CoordiService</string>\n"
		"\n"
		"\t\t<key>ProgramArguments</key>\n"
		"\t\t<array>\n"
		"\t\t\t<string>/Applications/XiaoApp.app/Contents/UE/Engine/Binaries/Mac/XiaoCoordiService</string>\n"
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
		"\t\t<string>/Library/Logs/XiaoCoordiService.err</string>\n"
		"\n"
		"\t\t<key>StandardOutPath</key>\n"
		"\t\t<string>/Library/Logs/XiaoCoordiService.log</string>\n"
		"\t</dict>\n"
		"</plist>"
	);

	return FFileHelper::SaveStringToFile(XmlContent, *SCoordiServicePath);
}

bool FMacCoordiService::OnStart()
{
	if (!FCoordiService::OnInitialize(TEXT("")))
	{
		return false;
	}

	while (!IsEngineExitRequested())
	{
		FCoordiService::OnTick();

		FPlatformProcess::Sleep(FCoordiService::SSleepTime);
	}

	FCoordiService::OnDeinitialize();
	
	return true;
}

bool FMacCoordiService::OnStop()
{
	FCoordiService::OnDeinitialize();
	return true;
}

bool FMacCoordiService::OnDelete()
{
	if (FPaths::FileExists(SCoordiServicePath))
	{
		IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
		return PlatformFile.DeleteFile(*SCoordiServicePath);
	}
	return true;
}
