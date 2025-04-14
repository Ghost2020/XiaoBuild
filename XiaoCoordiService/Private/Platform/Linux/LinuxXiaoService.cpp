/**
  * @author cxx2020@outlook.com
  * @datetime 2023 -9:08 PM
 */
#include "LinuxXiaoService.h"
#include "CoordiService.h"
#include "Misc/Paths.h"
#include "HAL/PlatformFileManager.h"

static const FString SCoordiServicePath = TEXT("/etc/systemd/system/XiaoCoordi.service");

FLinuxCoordiService::FLinuxCoordiService(const FServiceCommandLineOptions& InOptions, const FServiceDesc& InServiceDesc)
	: FGenericService(InOptions, InServiceDesc)
{
}

bool FLinuxCoordiService::OnInstall()
{
	static const FString ServiceContent = FString::Printf(TEXT(
"[Unit]\n"
"Description=%s\n"
"After=network.target\n"
"\n"
"[Service]\n"
"ExecStart=/usr/bin/%s\n"
"Restart=always\n"
"User=root\n"
"WorkingDirectory=/usr/bin\n"
"\n"
"[Install]\n"
"WantedBy=multi-user.target\n"
	), *Desc.Description, *Desc.DisplayName);
	return FFileHelper::SaveStringToFile(ServiceContent, *SCoordiServicePath);
}

bool FLinuxCoordiService::OnStart()
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

bool FLinuxCoordiService::OnEnable()
{
	return system("sudo systemctl enable XiaoCoordiService") == 0;
}

bool FLinuxCoordiService::OnDisable()
{
	return system("sudo systemctl disable XiaoCoordiService") == 0;
}

bool FLinuxCoordiService::OnStop()
{
	return system("sudo systemctl stop XiaoCoordiService") == 0;
}

bool FLinuxCoordiService::OnDelete()
{
	if (FPaths::FileExists(SCoordiServicePath))
	{
		IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
		return PlatformFile.DeleteFile(*SCoordiServicePath);
	}
	return true;
}