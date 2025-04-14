/**
  * @author cxx2020@outlook.com
  * @datetime 2023 -9:08 PM
 */

#include "WindowsXiaoService.h"
#include "Platform/Windows/WindowsService.h"
#include "WindowsAgentStatsMonitor.h"
#include "XiaoShare.h"
#include "AgentService.h"
#include "Containers/Ticker.h"


FWindowsBuildAgentService::FWindowsBuildAgentService(const FServiceCommandLineOptions& InOptions, const FServiceDesc& InServiceDesc)
	: FGenericService(InOptions, InServiceDesc)
{
	GServiceName = InServiceDesc.ServiceName;
	GServiceDesc = InServiceDesc;
}

FWindowsBuildAgentService::~FWindowsBuildAgentService()
{
	
}

bool FWindowsBuildAgentService::OnInstall()
{
	return FWindowsService::DoInstallService(GServiceDesc);
}

bool FWindowsBuildAgentService::OnRegister()
{
	XIAO_LOG(Log, TEXT("Register::Begin!"));
	const SERVICE_TABLE_ENTRY DispatchTable[] =
	{
		{const_cast<LPWSTR>(*GServiceName), static_cast<LPSERVICE_MAIN_FUNCTIONW>(FWindowsBuildAgentService::MainProc) },
		{nullptr, nullptr}
	};

	if(!StartServiceCtrlDispatcher(DispatchTable))
	{
		FWindowsService::ServiceReportEvent(TEXT("StartServiceCtrlDispatcher"));
	}
	XIAO_LOG(Log, TEXT("Register::Finish!"));
	return true;
}

bool FWindowsBuildAgentService::OnStart()
{
	return FWindowsService::DoStartService(GServiceName);
}

bool FWindowsBuildAgentService::OnEnable()
{
	return FWindowsService::DoEnableService(GServiceName);
}

bool FWindowsBuildAgentService::OnDisable()
{
	return FWindowsService::DoDisableService(GServiceName);
}

bool FWindowsBuildAgentService::OnQuery()
{
	QUERY_SERVICE_CONFIGW Config;
	return FWindowsService::DoQueryService(GServiceName, Config);
}

bool FWindowsBuildAgentService::OnTick(const float Internal)
{
	return true;
}

bool FWindowsBuildAgentService::OnPause()
{
	return false;
}

bool FWindowsBuildAgentService::OnContinue()
{
	return false;
}

bool FWindowsBuildAgentService::OnStop()
{
	return FWindowsService::DoStopService(GServiceName);
}

bool FWindowsBuildAgentService::OnDelete()
{
	return FWindowsService::DoDeleteService(GServiceName);
}

void FWindowsBuildAgentService::MainProc(DWORD DwArgc, LPWSTR* LpszArgv)
{
	if (!FWindowsService::OnDefaultServiceMain(DwArgc, LpszArgv))
	{
		return;
	}

	if (!FAgentService::OnInitialize(UTF8_TO_TCHAR(LpszArgv)))
	{
		XIAO_LOG(Error, TEXT("OnInitialize::failed!"));
		FWindowsService::ReportSvcStatus(SERVICE_STOPPED, ERROR_BAD_ENVIRONMENT, 0);
		return;
	}
	
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
		
		// 服务停止信号
		const DWORD Rtn = WaitForSingleObject(GhSvcStopEvent, DeltaTime);
		if (Rtn == WAIT_OBJECT_0 || Rtn == WAIT_ABANDONED)
		{
			XIAO_LOG(Log, TEXT("ServiceInit::SERVICE_STOPPED!"));
			break;
		}
	}
	
	FAgentService::OnDeinitialize();
	FWindowsService::ReportSvcStatus(SERVICE_STOPPED, NO_ERROR, 0);
}

void FWindowsBuildAgentService::ServiceCtrlHandler(const DWORD InDwCtrl)
{
	XIAO_LOG(Log, TEXT("ServiceCtrlHandler::%d!"), InDwCtrl);
	if (Singleton)
	{
		switch (InDwCtrl)
		{
		case SERVICE_CONTROL_STOP:
		case SERVICE_CONTROL_SHUTDOWN:
		{
			FAgentService::OnDeinitialize();
			XIAO_LOG(Log, TEXT("ServiceCtrlHandler::STOP Or Shutdown"));
			FWindowsService::ReportSvcStatus(SERVICE_STOP_PENDING, NO_ERROR, 0);
			SetEvent(GhSvcStopEvent);
			break;
		}
		case SERVICE_CONTROL_PAUSE:
		{
			XIAO_LOG(Log, TEXT("ServiceCtrlHandler::Pause!"));
			Singleton->OnPause();
			break;
		}
		case SERVICE_CONTROL_CONTINUE:
		{
			XIAO_LOG(Log, TEXT("ServiceCtrlHandler::Continue!"));
			Singleton->OnContinue();
			break;
		}
		case SERVICE_CONTROL_INTERROGATE:
		{
			XIAO_LOG(Log, TEXT("ServiceCtrlHandler::Interrograte!"));
			break;
		}
		case SERVICE_CONTROL_PARAMCHANGE:
		{
			XIAO_LOG(Log, TEXT("ServiceCtrlHandler::Param Change!"));
			break;
		}
		case SERVICE_CONTROL_DEVICEEVENT:
		{
			XIAO_LOG(Log, TEXT("ServiceCtrlHandler::Device Event!"));
			break;
		}
		case SERVICE_CONTROL_NETBINDADD:
		{
			XIAO_LOG(Log, TEXT("ServiceCtrlHandler::Net Bind Add!"));
			break;
		}
		case SERVICE_CONTROL_NETBINDREMOVE:
		{
			XIAO_LOG(Log, TEXT("ServiceCtrlHandler::Net Bind Remove!"));
			break;
		}
		case SERVICE_CONTROL_NETBINDENABLE:
		{
			XIAO_LOG(Log, TEXT("ServiceCtrlHandler::Net Bind Enable!"));
			break;
		}
		case SERVICE_CONTROL_NETBINDDISABLE:
		{
			XIAO_LOG(Log, TEXT("ServiceCtrlHandler::Net Bind Disable!"));
			break;
		}
		default: break;
		}
	}
}
