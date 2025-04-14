/**
  * @author cxx2020@outlook.com
  * @datetime 2023 -10:17 PM
 */
#include "LinuxXiaoService.h"
#include "AgentService.h"
#include "Misc/Paths.h"
#include "HAL/PlatformFileManager.h"

static const FString SAgentServicePath = TEXT("/etc/systemd/system/XiaoAgent.service");

FLinuxBuildAgentService::FLinuxBuildAgentService(const FServiceCommandLineOptions& InOptions, const FServiceDesc& InServiceDesc)
	: FGenericService(InOptions, InServiceDesc)
{
	
}

bool FLinuxBuildAgentService::OnInstall()
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
	return FFileHelper::SaveStringToFile(ServiceContent, *SAgentServicePath);
}

bool FLinuxBuildAgentService::OnStart()
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

bool FLinuxBuildAgentService::OnEnable()
{
	return system("sudo systemctl enable XiaoAgentService") == 0;
}

bool FLinuxBuildAgentService::OnDisable()
{
	return system("sudo systemctl disable XiaoAgentService") == 0;
}

bool FLinuxBuildAgentService::OnStop()
{
	return system("sudo systemctl stop XiaoAgentService") == 0;
}

bool FLinuxBuildAgentService::OnDelete()
{
	if (FPaths::FileExists(SAgentServicePath))
	{
		IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
		return PlatformFile.DeleteFile(*SAgentServicePath);
	}
	return true;
}
