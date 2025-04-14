/**
  * @author cxx2020@outlook.com
  * @datetime 2023 -11:33 PM
 */

#pragma once

#include "XiaoShare.h"
#include "Xiao/XiaoShareField.h"
#include "Misc/CommandLine.h"
#include "Windows/AllowWindowsPlatformTypes.h"
#include <accctrl.h>
#include <aclapi.h>
#include <winsvc.h>
#include <winerror.h>
#include <winnt.h>
#define SVC_ERROR ((DWORD)0xC0020001L)

static FString GServiceName;
static FServiceDesc GServiceDesc;
static SERVICE_STATUS GSvcStatus;
static SERVICE_STATUS_HANDLE GSvcStatusHandle;
static HANDLE GhSvcStopEvent = nullptr;

using namespace XiaoAppName::AppControl;

class FWindowsService
{
public:
	// 注册服务
	static bool DoInstallService(const FServiceDesc& ServiceDesc)
	{
		TCHAR SzPath[MAX_PATH];
        if(!GetModuleFileName(nullptr, SzPath, MAX_PATH))
        {
        	XIAO_LOG(Error, TEXT("Cannot install service (%d) \n"), GetLastError());
        	return false;
        }
       
        // Get a handle to the SCM database.
        const SC_HANDLE SchSCManager = OpenSCManager(nullptr, nullptr, SC_MANAGER_ALL_ACCESS);
        if(nullptr == SchSCManager)
        {
        	XIAO_LOG(Error, TEXT("OpenSCManager failed (%d) \n"), GetLastError());
        	return false;
        }

		/*LPCWSTR Deps = NULL;
		if (ServiceDesc.ServiceName == XiaoAppName::SBuildAgentService)
		{
			Deps = L"XiaoLicenseService\0XiaoCoordiService\0";
		}
		else if (ServiceDesc.ServiceName == XiaoAppName::SBuildLicenseService)
		{
			Deps = L"XiaoLicenseService\0";
		}*/
		
        // Create the service
        const SC_HANDLE SchService = CreateService(
			SchSCManager,
			*ServiceDesc.ServiceName,
			*ServiceDesc.DisplayName,
			SERVICE_ALL_ACCESS,
			ServiceDesc.ServiceType,
			ServiceDesc.StartType,
			SERVICE_ERROR_NORMAL,	// # TODO
			SzPath,
			ServiceDesc.LoadOrderGroup.IsEmpty() ? nullptr : *ServiceDesc.LoadOrderGroup,
			nullptr,
			nullptr,
			ServiceDesc.User.IsEmpty() ? nullptr : *ServiceDesc.User,
			ServiceDesc.Password.IsEmpty() ? nullptr : *ServiceDesc.Password
        );
        if(nullptr == SchService)
        {
        	XIAO_LOG(Error, TEXT("CreateService failed (%d) \n"), GetLastError());
        	CloseServiceHandle(SchSCManager);
        	return false;
        }
        
        XIAO_LOG(Log, TEXT("Service installed syccessfuly \n"));
       
		// 服务描述
		{
			SERVICE_DESCRIPTION Desc;
			Desc.lpDescription = const_cast<LPWSTR>(*ServiceDesc.Description);
			if (!ChangeServiceConfig2(SchService, SERVICE_CONFIG_DESCRIPTION, &Desc))
			{
				XIAO_LOG(Error, TEXT("ChangeServiceConfig2 failed\n"));
			}
			else
			{
				XIAO_LOG(Log, TEXT("Service description updated successfully.\n"));
			}
		}

		// 服务异常处理
		{
			SC_ACTION Actions[2];
			SERVICE_FAILURE_ACTIONS FailureActions;
			ZeroMemory(&FailureActions, sizeof(Actions));

			Actions[0].Type = SC_ACTION_RESTART;
			Actions[0].Delay = 3000;

			Actions[1].Type = SC_ACTION_RUN_COMMAND;
			Actions[1].Delay = 50000;

			FailureActions.dwResetPeriod = 86400;
			if (XiaoAppName::SBuildCoordiManager != ServiceDesc.ServiceName)
			{
				const FString Command = FString::Printf(TEXT("XiaoBuildManager.exe -failure=%s"), *ServiceDesc.ServiceName);
				FailureActions.lpCommand = const_cast<LPWSTR>(*Command);
			}
			FailureActions.cActions = 2;
			FailureActions.lpsaActions = Actions;

			if (!ChangeServiceConfig2(SchService, SERVICE_CONFIG_FAILURE_ACTIONS, &FailureActions))
			{
				XIAO_LOG(Error, TEXT("ChangeServiceConfig2 failed\n"));
			}
			else
			{
				XIAO_LOG(Log, TEXT("Service description updated successfully.\n"));
			}
		}
       
        Close(SchSCManager, SchService);
        return true;
	}
	
	// 启动服务
	static void DoInitService()
	{
		// Create an event. The Control handler function, SvcCtrlHandler,
		// signals this event when it receives the stop control code.
		GhSvcStopEvent = CreateEvent(nullptr, true, false, nullptr);
		if(GhSvcStopEvent == nullptr)
		{
			ReportSvcStatus(SERVICE_STOPPED, NO_ERROR, 0);
			return;
		}

		// Report running status when initialization is complete
		ReportSvcStatus(SERVICE_RUNNING, NO_ERROR, 0);

		// #TODO: Perform work until service stops
		while(true)
		{
			// Check whether to stop the service.
			WaitForSingleObject(GhSvcStopEvent, INFINITE);
			ReportSvcStatus(SERVICE_STOPPED, NO_ERROR, 0);
			return;
		}	
	}
	
	// 开始服务
	static bool DoStartService(const FString& InServiceName)
	{
		SC_HANDLE SchSCManager, SchService;
		if(!GetServiceContext(InServiceName, SchSCManager, SchService))
		{
			return false;
		}
	
		SERVICE_STATUS_PROCESS Status;
		DWORD BytesNeeded;
		// Save the tick count and initial checkpoint.
		DWORD DwStartTickCount = 0;
		DWORD DwWaitTime;
		DWORD DwOldCheckPoint;
		DWORD DwBytesNeeded;
		// Check the status in case the service is not stopped. 
		if (!QueryServiceStatusEx( SchService, SC_STATUS_PROCESS_INFO, reinterpret_cast<LPBYTE>(&Status), sizeof(SERVICE_STATUS_PROCESS), &BytesNeeded ) )
		{
			XIAO_LOG(Error, TEXT("QueryServiceStatusEx failed (%d)\n"), GetLastError());
			goto cleanup; 
		}
	
		if(Status.dwCurrentState != SERVICE_STOPPED && Status.dwCurrentState != SERVICE_STOP_PENDING)
		{
			XIAO_LOG(Error, TEXT("Cannot start the service because it is already running\n"));
			goto cleanup;
		}
	
		DwStartTickCount = GetTickCount();
		DwOldCheckPoint = Status.dwCheckPoint;
	
		// Wait for the service to stop before attempting to start it.
		while (Status.dwCurrentState == SERVICE_STOP_PENDING)
		{
			// Do not wait longer than the wait hint. A good interval is 
			// one-tenth of the wait hint but not less than 1 second 
			// and not more than 10 seconds. 
			DwWaitTime = Status.dwWaitHint / 10;
			if( DwWaitTime < 1000 )
			{
				DwWaitTime = 1000;
			}
			else if ( DwWaitTime > 10000 )
			{
				DwWaitTime = 10000;
			}
			Sleep( DwWaitTime );
			
			// Check the status until the service is no longer stop pending. 
			if (!QueryServiceStatusEx(SchService, SC_STATUS_PROCESS_INFO, reinterpret_cast<LPBYTE>(&Status), sizeof(SERVICE_STATUS_PROCESS), &DwBytesNeeded))
			{
				XIAO_LOG(Error, TEXT("QueryServiceStatusEx failed (%d)\n"), GetLastError());
				goto cleanup;
			}
	
			if (Status.dwCheckPoint > DwOldCheckPoint)
			{
				// Continue to wait and check.
				DwStartTickCount = GetTickCount();
				DwOldCheckPoint = Status.dwCheckPoint;
			}
			else
			{
				if(GetTickCount()-DwStartTickCount > Status.dwWaitHint)
				{
					XIAO_LOG(Error, TEXT("Timeout waiting for service to stop\n"));
					goto cleanup;
				}
			}
		}
	
		// Attempt to start the service.
		if (!StartService(SchService, 0,nullptr) )
		{
			XIAO_LOG(Error, TEXT("StartService failed (%d)\n"), GetLastError());
			goto cleanup;
		}
		XIAO_LOG(Log, TEXT("Service start pending...\n"));
	
		if (!QueryServiceStatusEx(SchService, SC_STATUS_PROCESS_INFO, reinterpret_cast<LPBYTE>(&Status), sizeof(SERVICE_STATUS_PROCESS), &DwBytesNeeded))
		{
			XIAO_LOG(Error, TEXT("QueryServiceStatusEx failed (%d)\n"), GetLastError());
			goto cleanup;
		}
	
		// Save the tick count and initial checkpoint.
		DwStartTickCount = GetTickCount();
		DwOldCheckPoint = Status.dwCheckPoint;
	
		while(Status.dwCurrentState == SERVICE_START_PENDING)
		{
			// Do not wait longer than the wait hint. A good interval is 
			// one-tenth the wait hint, but no less than 1 second and no 
			// more than 10 seconds.
			DwWaitTime = Status.dwWaitHint / 10;
			if( DwWaitTime < 1000 )
			{
				DwWaitTime = 1000;
			}
			else if (DwWaitTime > 10000)
			{
				DwWaitTime = 10000;
			}
	
			Sleep(DwWaitTime);
			// Check the status again. 
			if (!QueryServiceStatusEx(SchService, SC_STATUS_PROCESS_INFO, reinterpret_cast<LPBYTE>(&Status),sizeof(SERVICE_STATUS_PROCESS), &DwBytesNeeded))
			{
				XIAO_LOG(Error, TEXT("QueryServiceStatusEx failed (%d)\n"), GetLastError());
				break;
			}
			if (Status.dwCheckPoint > DwOldCheckPoint )
			{
				// Continue to wait and check.
				DwStartTickCount = GetTickCount();
				DwOldCheckPoint = Status.dwCheckPoint;
			}
			else
			{
				if(GetTickCount()-DwStartTickCount > Status.dwWaitHint)
				{
					// No progress made within the wait hint.
					break;
				}
			}
		}
	
		// Determine whether the service is running.
		if (Status.dwCurrentState == SERVICE_RUNNING) 
		{
			XIAO_LOG(Log, TEXT("Service started successfully.\n")); 
		}
		else 
		{ 
			XIAO_LOG(Error, TEXT("Service not started. \n"));
			XIAO_LOG(Error, TEXT(" Current State: %d\n"), Status.dwCurrentState); 
			XIAO_LOG(Error, TEXT(" Exit Code: %d\n"), Status.dwWin32ExitCode); 
			XIAO_LOG(Error, TEXT(" Check Point: %d\n"), Status.dwCheckPoint); 
			XIAO_LOG(Error, TEXT(" Wait Hint: %d\n"), Status.dwWaitHint); 
		}
	
	cleanup:
		Close(SchSCManager, SchService);
		return true;	
	}
	
	// 启用服务
	static bool DoEnableService(const FString& InServiceName)
	{
		SC_HANDLE SchSCManager, SchService;
		if(!GetServiceContext(InServiceName, SchSCManager, SchService))
		{
			return false;
		}

		// Change the service start type.
		if (!ChangeServiceConfig(SchService,SERVICE_NO_CHANGE,SERVICE_DEMAND_START, SERVICE_NO_CHANGE,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr))
		{
			XIAO_LOG(Error, TEXT("ChangeServiceConfig failed (%d)\n"), GetLastError()); 
		}
		else
		{
			XIAO_LOG(Log, TEXT("Service enabled successfully.\n"));
		}

		Close(SchSCManager, SchService);
		return true;	
	}
	
	// 禁用服务
	static bool DoDisableService(const FString& InServiceName)
	{
		SC_HANDLE SchSCManager, SchService;
		if(!GetServiceContext(InServiceName, SchSCManager, SchService))
		{
			return false;
		}

		// Change the service start type.
		if (!ChangeServiceConfig(SchService,SERVICE_NO_CHANGE,SERVICE_DISABLED,SERVICE_NO_CHANGE,  nullptr,nullptr,nullptr, nullptr,nullptr, nullptr,nullptr) )
		{
			XIAO_LOG(Error, TEXT("ChangeServiceConfig failed (%d)\n"), GetLastError()); 
		}
		else
		{
			XIAO_LOG(Log, TEXT("Service disabled successfully.\n"));
		}

		Close(SchSCManager, SchService);
		return true;	
	}
	
	// 更新服务
	static bool DoUpdateService(const FString& InServiceName, const FString& InDesc)
	{
		SC_HANDLE SchSCManager, SchService;
		if(!GetServiceContext(InServiceName, SchSCManager, SchService))
		{
			return false;
		}
	
		// Get the current security descriptor.
		EXPLICIT_ACCESS Ea;
		SECURITY_DESCRIPTOR Sd;
		PSECURITY_DESCRIPTOR PSD = nullptr;
		PACL Pacl = nullptr;
		PACL PNewAcl = nullptr;
		BOOL BDaclPresent = false;
		BOOL BDaclDefaulted = false;
		DWORD DwBytesNeeded = 0;
		DWORD DwError;
		if (!QueryServiceObjectSecurity(SchService, DACL_SECURITY_INFORMATION, &PSD,0, &DwBytesNeeded))
		{
			if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
			{
				const DWORD DwSize = DwBytesNeeded;
				PSD = (PSECURITY_DESCRIPTOR)HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY, DwSize);
				if (PSD == nullptr)
				{
					// Note: HeapAlloc does not support GetLastError.
					XIAO_LOG(Error, TEXT("HeapAlloc failed\n"));
					goto dacl_cleanup;
				}
	
				if (!QueryServiceObjectSecurity(SchService, DACL_SECURITY_INFORMATION, PSD, DwSize, &DwBytesNeeded))
				{
					XIAO_LOG(Error, TEXT("QueryServiceObjectSecurity failed (%d)\n"), GetLastError());
					goto dacl_cleanup;
				}
			}
			else 
			{
				XIAO_LOG(Error, TEXT("QueryServiceObjectSecurity failed (%d)\n"), GetLastError());
				goto dacl_cleanup;
			}
		}
	
		// Get the DACL.
		if (!GetSecurityDescriptorDacl(PSD, &BDaclPresent, &Pacl,&BDaclDefaulted))
		{
			XIAO_LOG(Error, TEXT("GetSecurityDescriptorDacl failed(%d)\n"), GetLastError());
			goto dacl_cleanup;
		}

		// Build the ACE.
		BuildExplicitAccessWithName(&Ea, const_cast<LPWSTR>(L"GUEST"), SERVICE_START | SERVICE_STOP | READ_CONTROL | DELETE, SET_ACCESS, NO_INHERITANCE);
		DwError = SetEntriesInAcl(1, &Ea, Pacl, &PNewAcl);
		if (DwError != ERROR_SUCCESS)
		{
			XIAO_LOG(Error, TEXT("SetEntriesInAcl failed(%d)\n"), DwError);
			goto dacl_cleanup;
		}
		
		// Initialize a new security descriptor.
		if (!InitializeSecurityDescriptor(&Sd, SECURITY_DESCRIPTOR_REVISION))
		{
			XIAO_LOG(Error, TEXT("InitializeSecurityDescriptor failed(%d)\n"), GetLastError());
			goto dacl_cleanup;
		}
		
		// Set the new DACL in the security descriptor.
		if (!SetSecurityDescriptorDacl(&Sd, true, PNewAcl, false))
		{
			XIAO_LOG(Error, TEXT("SetSecurityDescriptorDacl failed(%d)\n"), GetLastError());
			goto dacl_cleanup;
		}
		
		// Set the new DACL for the service object.
		if (!SetServiceObjectSecurity(SchService, DACL_SECURITY_INFORMATION, &Sd))
		{
			XIAO_LOG(Error, TEXT("SetServiceObjectSecurity failed(%d)\n"), GetLastError());
			goto dacl_cleanup;
		}
	
		XIAO_LOG(Log, TEXT("Service DACL updated successfully\n"));
	
		// Change the service description.
		SERVICE_DESCRIPTION Desc;
		Desc.lpDescription = const_cast<LPWSTR>(*InDesc);
		if( !ChangeServiceConfig2(SchService, SERVICE_CONFIG_DESCRIPTION, &Desc) )
		{
			XIAO_LOG(Error, TEXT("ChangeServiceConfig2 failed\n"));
		}
		else
		{
			XIAO_LOG(Log, TEXT("Service description updated successfully.\n"));
		}

	dacl_cleanup:
		Close(SchSCManager, SchService);
		if(nullptr != PNewAcl)
		{
			LocalFree((HLOCAL)PNewAcl);
		}
		if(nullptr != PSD)
		{
			HeapFree(GetProcessHeap(), 0, (LPVOID)PSD);
		}
		return true;
	}
	
	// 查询服务
	static bool DoQueryService(const FString& InServiceName, QUERY_SERVICE_CONFIGW& OutConfig)
	{
		SC_HANDLE SchSCManager, SchService;
		if(!GetServiceContext(InServiceName, SchSCManager, SchService))
		{
			return false;
		}
	
		LPQUERY_SERVICE_CONFIG Lpsc = nullptr;
		LPSERVICE_DESCRIPTION Lpsd = nullptr;
		DWORD DwBytesNeeded, DwError;
		DWORD CbBufSize = 0;
		if(!QueryServiceConfig(SchService, nullptr, 0, &DwBytesNeeded))
		{
			DwError = GetLastError();
			if( ERROR_INSUFFICIENT_BUFFER == DwError )
			{
				CbBufSize = DwBytesNeeded;
				Lpsc = static_cast<LPQUERY_SERVICE_CONFIG>(LocalAlloc(LMEM_FIXED, CbBufSize));
			}
			else
			{
				XIAO_LOG(Error, TEXT("QueryServiceConfig failed (%d)"), DwError);
				goto cleanup; 
			}
		}
		if(!QueryServiceConfig(SchService, Lpsc,  CbBufSize, &DwBytesNeeded) ) 
		{
			XIAO_LOG(Error, TEXT("QueryServiceConfig failed (%d)"), GetLastError());
			goto cleanup;
		}
		if(!QueryServiceConfig2(SchService, SERVICE_CONFIG_DESCRIPTION, nullptr, 0, &DwBytesNeeded))
		{
			DwError = GetLastError();
			if( ERROR_INSUFFICIENT_BUFFER == DwError )
			{
				CbBufSize = DwBytesNeeded;
				Lpsd = static_cast<LPSERVICE_DESCRIPTION>(LocalAlloc(LMEM_FIXED, CbBufSize));
			}
			else
			{
				XIAO_LOG(Error, TEXT("QueryServiceConfig2 failed (%d)"), DwError);
				goto cleanup;
			}
		}
		if (!QueryServiceConfig2(SchService, SERVICE_CONFIG_DESCRIPTION, reinterpret_cast<LPBYTE>(Lpsd), CbBufSize, &DwBytesNeeded) ) 
		{
			XIAO_LOG(Error, TEXT("QueryServiceConfig2 failed (%d)"), GetLastError());
			goto cleanup;
		}
	
		XIAO_LOG(Log, TEXT("%s configuration: \n"), *InServiceName);
		XIAO_LOG(Log, TEXT("Type: 0x%x\n"), Lpsc->dwServiceType);
		XIAO_LOG(Log, TEXT("Start Type: 0x%x\n"), Lpsc->dwStartType);
		XIAO_LOG(Log, TEXT("Error Control: 0x%x\n"), Lpsc->dwErrorControl);
		XIAO_LOG(Log, TEXT("Binary path: %s\n"), Lpsc->lpBinaryPathName);
		XIAO_LOG(Log, TEXT("Account: %s\n"), Lpsc->lpServiceStartName);
		if(Lpsd->lpDescription != nullptr && lstrcmp(Lpsd->lpDescription, TEXT("")) != 0)
		{
			XIAO_LOG(Log, TEXT("Description: %s\n"), Lpsd->lpDescription);
		}
		if(Lpsc->lpLoadOrderGroup != nullptr && lstrcmp(Lpsc->lpLoadOrderGroup, TEXT("")) != 0)
		{
			XIAO_LOG(Log, TEXT("Load order group: %s\n"), Lpsc->lpLoadOrderGroup);
		}
		if(Lpsc->dwTagId != 0)
		{
			XIAO_LOG(Log, TEXT("Tag ID: %d\n"), Lpsc->dwTagId);
		}
		if(Lpsc->lpDependencies != nullptr && lstrcmp(Lpsc->lpDependencies, TEXT("")) != 0)
		{
			XIAO_LOG(Log, TEXT("Dependencies: %s\n"), Lpsc->lpDependencies);
		}
		
		OutConfig = *Lpsc;
	
		LocalFree(Lpsc); 
		LocalFree(Lpsd);
	
cleanup:
		Close(SchSCManager, SchService);
		return true;	
	}
	
	// 停止服务
	static bool DoStopService(const FString& InServiceName)
	{
		SC_HANDLE SchSCManager, SchService;
		if(!GetServiceContext(InServiceName, SchSCManager, SchService))
		{
			return false;
		}
	
		// Make sure the service is not already stopped.
		SERVICE_STATUS_PROCESS Ssp;
		DWORD DwBytesNeeded;
		constexpr DWORD DwTimeout = 30000; // 30-second time-out
		const DWORD DwStartTime = GetTickCount();
		if (!QueryServiceStatusEx(SchService, SC_STATUS_PROCESS_INFO, reinterpret_cast<LPBYTE>(&Ssp), sizeof(SERVICE_STATUS_PROCESS), &DwBytesNeeded))
		{
			// #1072::指定的服务已标记为要删除
			XIAO_LOG(Error, TEXT("QueryServiceStatusEx failed (%d)\n"), GetLastError()); 
			goto clean_up;
		}
	
		if (Ssp.dwCurrentState == SERVICE_STOPPED)
		{
			XIAO_LOG(Error, TEXT("Service is already stopped.\n"));
			goto clean_up;
		}

		while(Ssp.dwCurrentState == SERVICE_STOP_PENDING)
		{
			XIAO_LOG(Log, TEXT("Service stop pending...\n"));
			// Do not wait longer than the wait hint. A good interval is 
			// one-tenth of the wait hint but not less than 1 second 
			// and not more than 10 seconds. 
			DWORD DwWaitTime = Ssp.dwWaitHint / 10;
			if( DwWaitTime < 1000 )
			{
				DwWaitTime = 1000;
			}
			else if ( DwWaitTime > 10000 )
			{
				DwWaitTime = 10000;
			}
			Sleep( DwWaitTime );
	
			if (!QueryServiceStatusEx( SchService, SC_STATUS_PROCESS_INFO, reinterpret_cast<LPBYTE>(&Ssp), sizeof(SERVICE_STATUS_PROCESS),&DwBytesNeeded))
			{
				XIAO_LOG(Error, TEXT("QueryServiceStatusEx failed (%d)\n"), GetLastError()); 
				goto clean_up;
			}
			
			if (Ssp.dwCurrentState == SERVICE_STOPPED)
			{
				XIAO_LOG(Log, TEXT("Service stopped successfully.\n"));
				goto clean_up;
			}
			if ( GetTickCount() - DwStartTime > DwTimeout )
			{
				XIAO_LOG(Error, TEXT("Service stop timed out.\n"));
				goto clean_up;
			}
		}

		// If the service is running, dependencies must be stopped first.
		StopDependentServices(SchSCManager, SchService);
		
		// Send a stop code to the service.
		if (!ControlService( SchService, SERVICE_CONTROL_STOP, reinterpret_cast<LPSERVICE_STATUS>(&Ssp) ) )
		{
			XIAO_LOG(Error, TEXT("ControlService failed (%d)\n"), GetLastError() );
			goto clean_up;
		}
		
		// Wait for the service to stop.
		while (Ssp.dwCurrentState != SERVICE_STOPPED )
		{
			Sleep(Ssp.dwWaitHint);
			if (!QueryServiceStatusEx( SchService, SC_STATUS_PROCESS_INFO, reinterpret_cast<LPBYTE>(&Ssp), sizeof(SERVICE_STATUS_PROCESS), &DwBytesNeeded))
			{
				XIAO_LOG(Error, TEXT("QueryServiceStatusEx failed (%d)\n"), GetLastError() );
				goto clean_up;
			}
			if (Ssp.dwCurrentState == SERVICE_STOPPED )
			{
				break;
			}
	
			if ((GetTickCount() - DwStartTime) > DwTimeout)
			{
				XIAO_LOG(Error, TEXT("Wait timed out\n"));
				goto clean_up;
			}
		}
		XIAO_LOG(Log, TEXT("Service stopped successfully\n"));

	clean_up:
		Close(SchSCManager, SchService);
		return true;
	}
	
	// 删除服务
	static bool DoDeleteService(const FString& InServiceName)
	{
		SC_HANDLE SchSCManager, SchService;
		if(!GetServiceContext(InServiceName, SchSCManager, SchService))
		{
			return false;
		}

		if (!DeleteService(SchService) ) 
		{
			XIAO_LOG(Error, TEXT("DeleteService failed (%d)\n"), GetLastError()); 
		}
		else
		{
			XIAO_LOG(Log, TEXT("Service deleted successfully\n"));
		}

		Close(SchSCManager, SchService);
		return true;
	}
	
	// Entry point for the service
	static bool OnDefaultServiceMain(DWORD DwArgc, LPWSTR *LpszArgv)
	{
		// Register the handler function for the service
		XIAO_LOG(Log, TEXT("ServiceCtrlHandler::OnServiceMain Start!"));
		GSvcStatusHandle = RegisterServiceCtrlHandler(*GServiceName, ServiceCtrlHandler);
		if(!GSvcStatusHandle)
		{ 
			XIAO_LOG(Error, TEXT("ServiceCtrlHandler::OnServiceMain::RegisterServiceCtrlHandler Failed!"));
			ServiceReportEvent(TEXT("RegisterServiceCtrlHandler")); 
			return false; 
		} 

		// These SERVICE_STATUS members remain as set here
		GSvcStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
		GSvcStatus.dwServiceSpecificExitCode = 0; 

		// Report initial status to the SCM
		ReportSvcStatus( SERVICE_START_PENDING, NO_ERROR, 3000 );

		// Perform service-specific initialization and work.
		DefaultServiceInit( DwArgc, LpszArgv );
		XIAO_LOG(Log, TEXT("ServiceCtrlHandler::OnServiceMain Finish!"));

		return true;
	}
	
	// Logs messages to the event log
	static void ServiceReportEvent(const FString InSzFunction)
	{
		if(const HANDLE HEventSource = RegisterEventSource(nullptr, *GServiceName); nullptr != HEventSource )
		{
			LPCTSTR LpszStrings[2];
			TCHAR Buffer[80];
			StringCchPrintf(Buffer, 80, TEXT("%s failed with %d"), *InSzFunction, GetLastError());
			LpszStrings[0] = *GServiceName;
			LpszStrings[1] = Buffer;
			ReportEventW(HEventSource, EVENTLOG_ERROR_TYPE, 0, SVC_ERROR, nullptr, 2, 0, LpszStrings, nullptr);
			DeregisterEventSource(HEventSource);
		}
	}

	// 设置当前服务的状态并报告给SCM
	static void ReportSvcStatus(const DWORD DwCurrentState, const DWORD DwWin32ExitCode, const DWORD DwWaitHint)
	{
		// Fill in the SERVICE_STATUS structure.
		XIAO_LOG(Log, TEXT("ReportSvcStatus::CurrentState::%d ExitCode::%d WaitHint::%d!"), DwCurrentState, DwWin32ExitCode, DwWaitHint);

		GSvcStatus.dwCurrentState = DwCurrentState;
		GSvcStatus.dwWin32ExitCode = DwWin32ExitCode;
		GSvcStatus.dwWaitHint = DwWaitHint;

		if (DwCurrentState == SERVICE_START_PENDING)
		{
			GSvcStatus.dwControlsAccepted = 0;
		}
		else
		{
			GSvcStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;
		}

		if ( (DwCurrentState == SERVICE_RUNNING) || (DwCurrentState == SERVICE_STOPPED) )
		{
			GSvcStatus.dwCheckPoint = 0;
		}
		else
		{
			static DWORD GDwCheckPoint = 1;
			GSvcStatus.dwCheckPoint = GDwCheckPoint++;
		}

		// Report the status of the service to the SCM.
		SetServiceStatus( GSvcStatusHandle, &GSvcStatus );
	}
	
	// The defualt service code
	static void DefaultServiceInit(DWORD DwArgc, LPWSTR *LpszArgv)
	{
		// TO_DO: Declare and set any required variables.
		// Be sure to periodically call ReportSvcStatus() with 
		// SERVICE_START_PENDING. If initialization fails, call
		// ReportSvcStatus with SERVICE_STOPPED.
		// Create an event. The control handler function, SvcCtrlHandler,
		// signals this event when it receives the stop control code.
		XIAO_LOG(Log, TEXT("ServiceInit::Begin!"));
		GhSvcStopEvent = CreateEvent(nullptr, true, false, nullptr); 
		if (GhSvcStopEvent == nullptr)
		{
			XIAO_LOG(Error, TEXT("ServiceInit::CreateEvent::Failred!"));
			ReportSvcStatus( SERVICE_STOPPED, GetLastError(), 0);
			return;
		}

		// Report running status when initialization is complete.
		ReportSvcStatus( SERVICE_RUNNING, NO_ERROR, 0);
		XIAO_LOG(Log, TEXT("ServiceInit::CreateEvent::Success!"));
		return;
	}
	
	// 获取服务上下文环境
	static bool GetServiceContext(const FString& InServiceName, SC_HANDLE& OutSchSCManager, SC_HANDLE& OutSchService)
	{
		// Get a handle to the SCM database. 
		OutSchSCManager= OpenSCManager( nullptr, nullptr,SC_MANAGER_ALL_ACCESS); 
		if (nullptr == OutSchSCManager) 
		{
			XIAO_LOG(Error, TEXT("OpenSCManager failed (%d)\n"), GetLastError());
			return false;
		}

		// Get a handle to the service.
		OutSchService = OpenService( OutSchSCManager, *InServiceName, DELETE);
		// need delete access
		if (OutSchService == nullptr)
		{ 
			XIAO_LOG(Error, TEXT("OpenService failed (%d)\n"), GetLastError()); 
			CloseServiceHandle(OutSchSCManager);
			return false;
		}

		return true;
	}
	
	// 停止依赖此服务的服务
	static bool StopDependentServices(const SC_HANDLE& InSchSCManager, const SC_HANDLE& InSchService)
	{
		DWORD DwBytesNeeded;
		DWORD DwCount;
		LPENUM_SERVICE_STATUS LpDependencies = nullptr;
		SERVICE_STATUS_PROCESS Ssp;
	
		const DWORD DwStartTime = GetTickCount();
	
		// Pass a zero-length buffer to get the required buffer size.
		if (EnumDependentServices(InSchService, SERVICE_ACTIVE, LpDependencies, 0, &DwBytesNeeded, &DwCount ) ) 
		{
			// If the Enum call succeeds, then there are no dependent
			// services, so do nothing.
			return true;
		} 
		
		if (GetLastError() != ERROR_MORE_DATA)
		{
			return false;
		}
		// Allocate a buffer for the dependencies.
		LpDependencies = static_cast<LPENUM_SERVICE_STATUS>(HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, DwBytesNeeded));
		if (!LpDependencies)
		{
			return false;
		}
	
		__try
		{
			// Enumerate the dependencies.
			if ( !EnumDependentServices(InSchService, SERVICE_ACTIVE, LpDependencies, DwBytesNeeded, &DwBytesNeeded, &DwCount ) )
			{
				return false;
			}
			for ( DWORD i = 0; i < DwCount; i++ ) 
			{
				const ENUM_SERVICE_STATUS Ess = *(LpDependencies + i);
				// Open the service.
				const SC_HANDLE HDepService = OpenService(InSchSCManager, Ess.lpServiceName, SERVICE_STOP | SERVICE_QUERY_STATUS);
				if ( !HDepService )
				{
					return false;
				}
				__try
				{
					// Send a stop code.
					if (!ControlService(HDepService, SERVICE_CONTROL_STOP, reinterpret_cast<LPSERVICE_STATUS>(&Ssp)))
					{
						return false;
					}
					// Wait for the service to stop.
					while ( Ssp.dwCurrentState != SERVICE_STOPPED ) 
					{
						Sleep( Ssp.dwWaitHint );
						if ( !QueryServiceStatusEx( HDepService, SC_STATUS_PROCESS_INFO, reinterpret_cast<LPBYTE>(&Ssp), sizeof(SERVICE_STATUS_PROCESS), &DwBytesNeeded ) )
						{
							return false;
						}
						if ( Ssp.dwCurrentState == SERVICE_STOPPED )
						{
							break;
						}
						constexpr DWORD DwTimeout = 30000;
						if ( GetTickCount() - DwStartTime > DwTimeout )
						{
							return false;
						}
					}
				} 
				__finally 
				{
					// Always release the service handle.
					CloseServiceHandle( HDepService );
				}
			}
		}
		__finally 
		{
			// Always free the enumeration buffer.
			HeapFree( GetProcessHeap(), 0, LpDependencies );
		}
	
		return true;
	}

	static void WINAPI ServiceCtrlHandler(const DWORD InDwCtrl)
	{
		XIAO_LOG(Log, TEXT("ServiceCtrlHandler::%d!"), InDwCtrl);
		switch (InDwCtrl)
		{
		case SERVICE_CONTROL_STOP:
		case SERVICE_CONTROL_SHUTDOWN:
		{
			XIAO_LOG(Log, TEXT("ServiceCtrlHandler::STOP Or Shutdown"));
			ReportSvcStatus(SERVICE_STOP_PENDING, NO_ERROR, 0);
			SetEvent(GhSvcStopEvent);
			break;
		}
		case SERVICE_CONTROL_PAUSE:
		{
			XIAO_LOG(Log, TEXT("ServiceCtrlHandler::Pause!"));
			break;
		}
		case SERVICE_CONTROL_CONTINUE:
		{
			XIAO_LOG(Log, TEXT("ServiceCtrlHandler::Continue!"));
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
		default: break;
		}
	}
	
	static void Close(const SC_HANDLE& InSchSCManager, const SC_HANDLE& InSchService)
	{
		CloseServiceHandle(InSchService); 
		CloseServiceHandle(InSchSCManager);
	}

	static bool ServiceControl(const int32 ArgC, TCHAR* ArgV[])
	{
		const FString CmdLine = FCommandLine::GetOriginal();
		if (FParse::Param(*CmdLine, *SInstall))
		{
			const FString DisplayName = (ArgC > 3) ? ArgV[3] : GServiceName;
			const DWORD ServiceType = (ArgC > 4) ? FCString::Atoi(ArgV[4]) : 0;
			const DWORD StartType = (ArgC > 5) ? FCString::Atoi(ArgV[5]) : 0;
			const FString ServiceDescription = (ArgC > 6) ? ArgV[6] : TEXT("");
			const FString LoadOrderGroup = (ArgC > 7) ? ArgV[7] : TEXT("");
			const FString User = (ArgC > 8) ? ArgV[8] : TEXT("");
			const FString Password = (ArgC > 9) ? ArgV[9] : TEXT("");
			const FServiceDesc ServiceDesc(GServiceName, DisplayName, ServiceType, StartType, ServiceDescription, LoadOrderGroup, User, Password);
			return FWindowsService::DoInstallService(ServiceDesc);
		}
		if (FParse::Param(*CmdLine, *SQuery))
		{
			QUERY_SERVICE_CONFIGW ServiceConfig;
			return FWindowsService::DoQueryService(GServiceName, ServiceConfig);
		}
		if (FParse::Param(*CmdLine, *SEnable))
		{
			return FWindowsService::DoEnableService(GServiceName);
		}
		if (FParse::Param(*CmdLine, *SDisable))
		{
			return FWindowsService::DoDisableService(GServiceName);
		}
		if (FParse::Param(*CmdLine, *SStop))
		{
			return FWindowsService::DoStopService(GServiceName);
		}
		if (FParse::Param(*CmdLine, *SDelete))
		{
			return FWindowsService::DoDeleteService(GServiceName);
		}
		return false;
	}

	static bool GetLogonSID(HANDLE HToken, PSID* PPSid)
	{
		BOOL bSUccess = false;
		DWORD DwIndex;
		DWORD DwLength;
		PTOKEN_GROUPS Ptg = nullptr;

		if (PPSid == nullptr)
		{
			goto CLEANUP;
		}

		if (!GetTokenInformation(HToken, TokenGroups, Ptg, 0, &DwLength))
		{
			if (GetLastError() != ERROR_INSUFFICIENT_BUFFER)
			{
				XIAO_LOG(Error, TEXT("ERROR_INSUFFICIENT_BUFFER::%d!"), GetLastError());
				goto CLEANUP;
			}

			Ptg = (PTOKEN_GROUPS)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, DwLength);
			if (Ptg == nullptr)
			{
				XIAO_LOG(Error, TEXT("HeapAlloc::%d!"), GetLastError());
				goto CLEANUP;
			}
		}

		if (!GetTokenInformation(HToken, TokenGroups, Ptg, DwLength, &DwLength))
		{
			XIAO_LOG(Error, TEXT("HeapAlloc::%d!"), GetLastError());
			goto CLEANUP;
		}

		for (DwIndex = 0; DwIndex < Ptg->GroupCount; ++DwIndex)
		{
			DwLength = GetLengthSid(Ptg->Groups[DwIndex].Sid);
			*PPSid = (PSID)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, DwLength);
			if (*PPSid == nullptr)
			{
				goto CLEANUP;
			}

			if (!CopySid(DwLength, *PPSid, Ptg->Groups[DwIndex].Sid))
			{
				HeapFree(GetProcessHeap(), 0, *PPSid);
				goto CLEANUP;
			}
			break;
		}

		bSUccess = true;

	CLEANUP:
		if (Ptg != nullptr)
		{
			HeapFree(GetProcessHeap(), 0, Ptg);
		}

		return bSUccess;
	}

	static void FreeLogonSID(PSID* PPSid)
	{
		HeapFree(GetProcessHeap(), 0, (LPVOID)*PPSid);
	}

	static bool AddAceToWindowStation(HWINSTA Hwinsta, PSID PSid)
	{
		ACCESS_ALLOWED_ACE* Pace = nullptr;
		ACL_SIZE_INFORMATION AclSizeInfo;
		BOOL bDalExist = false;
		BOOL bDalPresent = false;
		BOOL bSuccess = false;
		DWORD DwNewAclSize = 0;
		DWORD DwSidSize = 0;
		DWORD SdSizeNeeded = 0;
		PACL PAcal = nullptr;
		PACL PNewAcl = nullptr;
		PSECURITY_DESCRIPTOR Psd = nullptr;
		PSECURITY_DESCRIPTOR PsdNew = nullptr;
		PVOID PTempAce;
		SECURITY_INFORMATION Si = DACL_SECURITY_INFORMATION;
		unsigned int i;

		__try
		{
			if (!GetUserObjectSecurity(Hwinsta, &Si, Psd, DwSidSize, &SdSizeNeeded))
			{
				if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
				{
					Psd = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, SdSizeNeeded);
					if (Psd == nullptr)
					{
						__leave;
					}

					PsdNew = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, SdSizeNeeded);
					if (PsdNew == nullptr)
					{
						__leave;
					}

					DwSidSize = SdSizeNeeded;

					if (!GetUserObjectSecurity(Hwinsta, &Si, Psd, DwSidSize, &SdSizeNeeded))
					{
						__leave;
					}
				}
				else
				{
					__leave;
				}

				if (!InitializeSecurityDescriptor(PsdNew, SECURITY_DESCRIPTOR_REVISION))
				{
					__leave;
				}

				if (!GetSecurityDescriptorDacl(Psd, &bDalPresent, &PAcal, &bDalExist))
				{
					__leave;
				}

				ZeroMemory(&AclSizeInfo, sizeof(ACL_SIZE_INFORMATION));
				AclSizeInfo.AclBytesInUse = sizeof(ACL);

				if (PAcal != nullptr)
				{
					if (!GetAclInformation(PAcal, &AclSizeInfo, sizeof(ACL_SIZE_INFORMATION), AclSizeInformation))
					{
						__leave;
					}
				}

				DwNewAclSize = AclSizeInfo.AclBytesInUse + (2 * sizeof(ACCESS_ALLOWED_ACE)) + (2 * GetLengthSid(PSid)) - (2 * sizeof(DWORD));

				PNewAcl = (PACL)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, DwNewAclSize);
				if (PNewAcl == nullptr)
				{
					__leave;
				}

				if (!InitializeAcl(PNewAcl, DwNewAclSize, ACL_REVISION))
				{
					__leave;
				}

				if (bDalPresent)
				{
					if (AclSizeInfo.AceCount)
					{
						for (i = 0; i < AclSizeInfo.AceCount; i++)
						{
							if (!GetAce(PAcal, i, &PTempAce))
							{
								__leave;
							}

							if (!AddAce(PNewAcl, ACL_REVISION, MAXDWORD, PTempAce, ((PACE_HEADER)PTempAce)->AceSize))
							{
								__leave;
							}
						}
					}
				}

				Pace = (ACCESS_ALLOWED_ACE*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(ACCESS_ALLOWED_ACE) + GetLengthSid(PSid) - sizeof(DWORD));
				if (Pace == nullptr)
				{
					__leave;
				}
				// Add the first ACE to the window station.
				Pace->Header.AceType = ACCESS_ALLOWED_ACE_TYPE;
				Pace->Header.AceFlags = CONTAINER_INHERIT_ACE | INHERIT_ONLY_ACE | OBJECT_INHERIT_ACE;
				Pace->Header.AceSize = LOWORD(sizeof(ACCESS_ALLOWED_ACE) + GetLengthSid(PSid) - sizeof(DWORD));
				Pace->Mask = WINSTA_ALL_ACCESS;
				if (!AddAce(PNewAcl, ACL_REVISION, MAXDWORD, Pace, Pace->Header.AceSize))
				{
					__leave;
				}

				// Add the second ACE to the window station.
				Pace->Header.AceFlags = NO_PROPAGATE_INHERIT_ACE;
				Pace->Mask = WINSTA_ALL_ACCESS;
				if (!AddAce(PNewAcl, ACL_REVISION, MAXDWORD, Pace, Pace->Header.AceSize))
				{
					__leave;
				}

				if (!SetSecurityDescriptorDacl(PsdNew, TRUE, PNewAcl, FALSE))
				{
					__leave;
				}

				if (!SetUserObjectSecurity(Hwinsta, &Si, PsdNew))
				{
					__leave;
				}

				bSuccess = true;
			}
		}
		__finally
		{
			if (Pace != nullptr)
			{
				HeapFree(GetProcessHeap(), 0, Pace);
			}
			if (PNewAcl != nullptr)
			{
				HeapFree(GetProcessHeap(), 0, PNewAcl);
			}
			if (PSid != nullptr)
			{
				HeapFree(GetProcessHeap(), 0, PSid);
			}
			if (PsdNew != nullptr)
			{
				HeapFree(GetProcessHeap(), 0, PsdNew);
			}
		}
		return bSuccess;
	}

	static bool StartInteractiveClientProcess(const FString& InUsername, const FString& InDomain, const FString& InPassword, const FString& Commandline)
	{
		HANDLE HToken = nullptr;
		HDESK HDesk = nullptr;
		HWINSTA HWinsta = nullptr;
		HWINSTA hWinStaSave = nullptr;
		PROCESS_INFORMATION Pi;
		PSID PSid = nullptr;
		STARTUPINFO Si;
		BOOL bResult = false;
		if (!LogonUser(TEXT("Ghost_Chan"), TEXT("GHOST"), TEXT("Ghost202081")/**InUsername, *InDomain, *InPassword*/, LOGON32_LOGON_INTERACTIVE, LOGON32_PROVIDER_DEFAULT, &HToken))
		{
			XIAO_LOG(Error, TEXT("LogonUser Failed::%d!"), GetLastError());
			goto CLEANUP;
		}
		
		if ((hWinStaSave = GetProcessWindowStation()) == nullptr)
		{
			XIAO_LOG(Error, TEXT("GetProcessWindowStation Failed::%d!"), GetLastError());
			goto CLEANUP;
		}

		if ((HWinsta = OpenWindowStation(_T("winsta0"), false, READ_CONTROL | WRITE_DAC)) == nullptr)
		{
			XIAO_LOG(Error, TEXT("OpenWindowStation Failed::%d!"), GetLastError());
			goto CLEANUP;
		}

		if (!SetProcessWindowStation(HWinsta))
		{
			XIAO_LOG(Error, TEXT("SetProcessWindowStation Failed::%d!"), GetLastError());
			goto CLEANUP;
		}

		HDesk = OpenDesktop(_T("default"), 0, false, READ_CONTROL | WRITE_DAC | DESKTOP_WRITEOBJECTS | DESKTOP_READOBJECTS);

		if (!SetProcessWindowStation(hWinStaSave))
		{
			XIAO_LOG(Error, TEXT("SetProcessWindowStation Failed::%d!"), GetLastError());
			goto CLEANUP;
		}

		if (HDesk == nullptr)
		{
			XIAO_LOG(Error, TEXT("HDesk Failed!"));
			goto CLEANUP;
		}

		if (!GetLogonSID(HToken, &PSid))
		{
			XIAO_LOG(Error, TEXT("GetLogonSID Failed::%d!"), GetLastError());
			goto CLEANUP;
		}

		if (!AddAceToWindowStation(HWinsta, PSid))
		{
			XIAO_LOG(Error, TEXT("AddAceToWindowStation Failed::%d!"), GetLastError());
			goto CLEANUP;
		}

		if (!ImpersonateLoggedOnUser(HToken))
		{
			XIAO_LOG(Error, TEXT("ImpersonateLoggedOnUser Failed::%d!"), GetLastError());
			goto CLEANUP;
		}

		ZeroMemory(&Si, sizeof(STARTUPINFO));
		Si.cb = sizeof(STARTUPINFO);
		Si.lpDesktop = const_cast<LPWSTR>(TEXT("winsta0\\default"));

		bResult = CreateProcessAsUser(
			HToken,
			nullptr,
			const_cast<LPWSTR>(*Commandline),
			nullptr,
			nullptr,
			false,
			NORMAL_PRIORITY_CLASS | CREATE_NEW_CONSOLE,
			nullptr,
			nullptr,
			&Si,
			&Pi
		);

		RevertToSelf();

		if (bResult && Pi.hProcess != INVALID_HANDLE_VALUE)
		{
			WaitForSingleObject(Pi.hProcess, INFINITE);
			CloseHandle(Pi.hProcess);
		}

		if (Pi.hThread != INVALID_HANDLE_VALUE)
		{
			CloseHandle(Pi.hThread);
		}

	CLEANUP:
		if (hWinStaSave != nullptr)
		{
			SetProcessWindowStation(hWinStaSave);
		}
		if (PSid)
		{
			FreeLogonSID(&PSid);
		}
		if (HWinsta)
		{
			CloseWindowStation(HWinsta);
		}
		if (HDesk)
		{
			CloseDesktop(HDesk);
		}
		if (HToken != INVALID_HANDLE_VALUE)
		{
			CloseHandle(HToken);
		}

		return bResult;
	}
};

#include "Windows/HideWindowsPlatformTypes.h"