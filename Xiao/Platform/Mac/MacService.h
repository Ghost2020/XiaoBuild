/**
  * @author cxx2020@outlook.com
  * @datetime 2023 -11:33 PM
 */

#pragma once

#include "XiaoShare.h"
#include "Xiao/XiaoShareField.h"
#include "Misc/CommandLine.h"

using namespace XiaoAppName::AppControl;

class FMacService
{
public:
	// 注册服务
	static bool DoInstallService(const FServiceDesc& ServiceDesc)
	{
		
	}
	
	// 启动服务
	static void DoInitService()
	{
		
	}
	
	// 开始服务
	static bool DoStartService(const FString& InServiceName)
	{
		return true;	
	}
	
	// 启用服务
	static bool DoEnableService(const FString& InServiceName)
	{
		return true;	
	}
	
	// 禁用服务
	static bool DoDisableService(const FString& InServiceName)
	{
		return true;	
	}
	
	// 更新服务
	static bool DoUpdateService(const FString& InServiceName, const FString& InDesc)
	{
		return true;
	}
	
	// 查询服务
	static bool DoQueryService(const FString& InServiceName, QUERY_SERVICE_CONFIGW& OutConfig)
	{
		return true;	
	}
	
	// 停止服务
	static bool DoStopService(const FString& InServiceName)
	{
		return true;
	}
	
	// 删除服务
	static bool DoDeleteService(const FString& InServiceName)
	{
		return true;
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
			return FMacService::DoInstallService(ServiceDesc);
		}
		if (FParse::Param(*CmdLine, *SQuery))
		{
			QUERY_SERVICE_CONFIGW ServiceConfig;
			return FMacService::DoQueryService(GServiceName, ServiceConfig);
		}
		if (FParse::Param(*CmdLine, *SEnable))
		{
			return FMacService::DoEnableService(GServiceName);
		}
		if (FParse::Param(*CmdLine, *SDisable))
		{
			return FMacService::DoDisableService(GServiceName);
		}
		if (FParse::Param(*CmdLine, *SDelete))
		{
			return FMacService::DoDeleteService(GServiceName);
		}
		return false;
	}
};