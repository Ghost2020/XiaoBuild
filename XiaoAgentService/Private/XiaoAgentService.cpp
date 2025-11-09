/**
  * @author cxx2020@outlook.com
  * @date 10:49 AM
 */

#include "XiaoAgentService.h"
#include "RequiredProgramMainCPPInclude.h"
#include "Service/GenericService.h"
#include <iostream>
#include <string>
#include "Version.h"

IMPLEMENT_APPLICATION(XiaoAgentService, XB_PRODUCT_NAME);

static void PrintHelp()
{
	const std::string HelpString =
"-----------------------------------------------------------------------\n"
+ std::string(XB_PRODUCT_NAME) + std::string(" ") + TCHAR_TO_UTF8(XB_VERSION_STRING) + std::string("\n")
+ std::string(XB_COPYRIGHT_STRING) + std::string("\n") +
"-----------------------------------------------------------------------\n";
    std::cout << HelpString << std::endl;
}

static bool ParseCommandLine(const int32 ArgC, TCHAR* ArgV[])
{
	const FString CmdLine = FCommandLine::BuildFromArgV(nullptr, ArgC, ArgV, nullptr);
	if(CmdLine.IsEmpty())
	{
		return false;
	}

	FCommandLine::Set(*CmdLine);

	return true;
}

static void BeforeExit()
{
	ReleaseAppMutex();
	FEngineLoop::AppExit();
}

INT32_MAIN_INT32_ARGC_TCHAR_ARGV()
{
	GEngineLoop.PreInit(ArgC, ArgV, *FString::Printf(TEXT("-LOG=%s"), *XiaoAppName::SBuildAgentService));
	XIAO_LOG(Verbose, TEXT("XiaoAgentService::Begin!"));

	atexit(BeforeExit);

	// 解析参数
	ParseCommandLine(ArgC, ArgV);

	bool Code = false;
	const FString CmdLine = FCommandLine::GetOriginal();
	const auto Options = FServiceCommandLineOptions::FromString(*CmdLine);
	const FServiceDesc Desc(
		ProductName,
		ProductName,
		SERVICE_WIN32_SHARE_PROCESS, // # TODO
		SERVICE_AUTO_START,
		TEXT("Reports Agent status to the Coordinator, and manages use of local resources by other Agents on the network.")///,
		// TEXT("network")
	);

	if(const TSharedPtr<FGenericService> Service = MakeShared<FXiaoService>(Options, Desc))
	{
		if (FParse::Param(*CmdLine, TEXT("install")))
		{
			Code = Service->OnInstall();
		}
		else if (FParse::Param(*CmdLine, TEXT("delete")))
		{
			Code = Service->OnDelete();
		}
		else
		{
#if PLATFORM_WINDOWS
			Code = Service->OnRegister();
#else
			Code = Service->OnStart();
#endif
		}
	}

	if(!Code)
	{
		PrintHelp();
	}
	
	return Code ? 0 : -1;
}
