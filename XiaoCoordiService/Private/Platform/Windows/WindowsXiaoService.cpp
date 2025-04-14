/**
  * @author cxx2020@outlook.com
  * @datetime 2023 -9:08 PM
 */
#include "WindowsXiaoService.h"
#include "Platform/Windows/WindowsService.h"
#include "CoordiService.h"

FWindowsCoordiService::FWindowsCoordiService(const FServiceCommandLineOptions& InOptions, const FServiceDesc& InServiceDesc)
	: FGenericService(InOptions, InServiceDesc)
{
	GServiceName = InServiceDesc.ServiceName;
	GServiceDesc = InServiceDesc;
}

FWindowsCoordiService::~FWindowsCoordiService()
{
	
}

bool FWindowsCoordiService::OnInstall()
{
	return FWindowsService::DoInstallService(GServiceDesc);
}

bool FWindowsCoordiService::OnRegister()
{
	XIAO_LOG(Log, TEXT("Register::Begin!"));
	const SERVICE_TABLE_ENTRY DispatchTable[] =
	{
		{const_cast<LPWSTR>(*GServiceName), static_cast<LPSERVICE_MAIN_FUNCTIONW>(FWindowsCoordiService::MainProc) },
		{nullptr, nullptr}
	};

	if(!StartServiceCtrlDispatcher(DispatchTable))
	{
		FWindowsService::ServiceReportEvent(TEXT("StartServiceCtrlDispatcher"));
	}
	XIAO_LOG(Log, TEXT("Register::Finish!"));
	return true;
}

bool FWindowsCoordiService::OnStart()
{
	return FWindowsService::DoStartService(GServiceName);
}

bool FWindowsCoordiService::OnEnable()
{
	return FWindowsService::DoEnableService(GServiceName);
}

bool FWindowsCoordiService::OnDisable()
{
	return FWindowsService::DoDisableService(GServiceName);
}

bool FWindowsCoordiService::OnQuery()
{
	QUERY_SERVICE_CONFIGW Config;
	return FWindowsService::DoQueryService(GServiceName, Config);
}

bool FWindowsCoordiService::OnTick(const float Internal)
{
	return true;
}

bool FWindowsCoordiService::OnPause()
{
	return false;
}

bool FWindowsCoordiService::OnContinue()
{
	return false;
}

bool FWindowsCoordiService::OnStop()
{
	return FWindowsService::DoStopService(GServiceName);
}

bool FWindowsCoordiService::OnDelete()
{
	return FWindowsService::DoDeleteService(GServiceName);
}

void FWindowsCoordiService::MainProc(DWORD DwArgc, LPWSTR* LpszArgv)
{
	if (!FWindowsService::OnDefaultServiceMain(DwArgc, LpszArgv))
	{
		return;
	}

	if(!FCoordiService::OnInitialize(UTF8_TO_TCHAR(LpszArgv)))
	{
		return;
	}
	
	while (!IsEngineExitRequested())
	{
		FCoordiService::OnTick();

		FPlatformProcess::Sleep(FCoordiService::SSleepTime);

		// 服务停止信号
		const DWORD Rtn = WaitForSingleObject(GhSvcStopEvent, 0.01f);
		if (Rtn == WAIT_OBJECT_0 || Rtn == WAIT_ABANDONED)
		{
			XIAO_LOG(Log, TEXT("ServiceInit::SERVICE_STOPPED!"));
			break;
		}
	}

	FCoordiService::OnDeinitialize();
	FWindowsService::ReportSvcStatus(SERVICE_STOPPED, NO_ERROR, 0);
}

void FWindowsCoordiService::ServiceCtrlHandler(const DWORD InDwCtrl)
{
	XIAO_LOG(Log, TEXT("ServiceCtrlHandler::%d!"), InDwCtrl);
	if (Singleton)
	{
		switch (InDwCtrl)
		{
		case SERVICE_CONTROL_STOP:
		case SERVICE_CONTROL_SHUTDOWN:
			{
				FCoordiService::OnDeinitialize();
				XIAO_LOG(Log, TEXT("ServiceCtrlHandler::STOP Or SHUTDOWN"));
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
