/**
  * @author cxx2020@outlook.com
  * @date 10:49 AM
 */

#include "XiaoCoordiService.h"
#include "Service/GenericService.h"
#include <iostream>
#include <string>
#include "RequiredProgramMainCPPInclude.h"
#include "Version.h"


IMPLEMENT_APPLICATION(XiaoCoordiService, XB_PRODUCT_NAME);


static void PrintHelp()
{
	const std::string HelpString =
"-----------------------------------------------------------------------\n"
+ std::string(XB_PRODUCT_NAME) + std::string(" ") + std::string(XB_VERSION_STRING) + std::string("\n")
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
	GNeedFlush = true;
	GEngineLoop.PreInit(ArgC, ArgV, *FString::Printf(TEXT("-LOG=%s"), *XiaoAppName::SBuildCoordiService));
	XIAO_LOG(Warning, TEXT("XiaoCoordiService::Begin!"));

	atexit(BeforeExit);

	if (!CheckSingleton())
	{
		XIAO_LOG(Error, TEXT("Must run in singleton mode!"));
		return -1;
	}

	// 解析参数
	ParseCommandLine(ArgC, ArgV);

	bool Code = false;
	const FString CmdLine = FCommandLine::GetOriginal();
	const auto Options = FServiceCommandLineOptions::FromString(FCommandLine::GetOriginal());
	const FServiceDesc Desc(
		ProductName,
		ProductName,
		SERVICE_WIN32_SHARE_PROCESS,
		SERVICE_AUTO_START,
		TEXT("Keeps track of available Agents, and assigns computing resources to distributed jobs...")
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
		// FWindowsCoordiService::MainProc(ArgC, ArgV);
	}
	
	XIAO_LOG(Log, TEXT("XiaoCoordiService::Finish::Code::%d!"), Code);
	return Code ? 0 : -1;
}
