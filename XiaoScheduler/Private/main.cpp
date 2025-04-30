/**
  * @author cxx2020@outlook.com
  * @datetime 2023 -9:32 PM
 */

#include "Misc/CommandLine.h"
#include "Runtime/Launch/Resources/Version.h"
#include "XiaoAgent.h"
#include "XiaoShareRedis.h"
#include "exception.pb.h"
#include "Scheduler.h"

#if PLATFORM_WINDOWS
#include "Windows/WindowsPlatformCrashContext.h"
#include <windows.h>
#include <dbghelp.h>
#pragma comment (lib, "Dbghelp.lib")
#endif

TCHAR GInternalProjectName[64] = TEXT("XiaoScheduler");
const TCHAR* GForeignEngineDir = nullptr;

namespace uba
{
	FSchedulerContext* GContext = nullptr;

	static void OnCancelKeyPress()
	{
		if (GContext)
		{
			GContext->Reset();
		}
	}

#if PLATFORM_WINDOWS
	static void WriteDump(LPEXCEPTION_POINTERS InExceptionInfo)
	{
		// Try to create file for minidump.
		FString ReportDirectoryAbsolutePath;
		FPlatformCrashContext::CreateCrashReportDirectory(*FGuid::NewGuid().ToString(), 0, ReportDirectoryAbsolutePath);
		const FString DumpPath = FPaths::Combine(ReportDirectoryAbsolutePath, TEXT("XiaoScheduler.dmp"));
		HANDLE FileHandle = ::CreateFileW(*DumpPath, GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
		if (FileHandle == INVALID_HANDLE_VALUE)
		{
			return;
		}

		// Initialise structure required by MiniDumpWriteDumps
		MINIDUMP_EXCEPTION_INFORMATION DumpExceptionInfo = {};
		DumpExceptionInfo.ThreadId = FPlatformTLS::GetCurrentThreadId();
		DumpExceptionInfo.ExceptionPointers = InExceptionInfo;
		DumpExceptionInfo.ClientPointers = 0;

		MiniDumpWriteDump(GetCurrentProcess(), FPlatformProcess::GetCurrentProcessId(), FileHandle, MiniDumpWithFullMemory, InExceptionInfo ? &DumpExceptionInfo : nullptr, nullptr, nullptr);
		CloseHandle(FileHandle);
	}

	static int ReportSEH(LPEXCEPTION_POINTERS InExceptionInfo)
	{
		StringBuffer<4096> assertInfo;
		assertInfo.Appendf(TC("SEH EXCEPTION %u (0x%llx)"), InExceptionInfo->ExceptionRecord->ExceptionCode, InExceptionInfo->ExceptionRecord->ExceptionAddress);

		WriteAssertInfo(assertInfo, nullptr, nullptr, 0, nullptr, 
#if (ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION <= 5)
			1
#else
			nullptr
#endif
		);
		WriteDump(InExceptionInfo);

		FException Exception;
		Exception.set_application("XiaoScheduler");
		Exception.set_exitcode(InExceptionInfo->ExceptionRecord->ExceptionCode);
		const FString Guid = FGuid::NewGuid().ToString();
		const std::string StdGuid = TCHAR_TO_UTF8(*Guid);
		Exception.set_buildid(StdGuid);
		const std::string AssetInfo = TCHAR_TO_UTF8(assertInfo.data);
		*Exception.mutable_description() = AssetInfo;
		const std::string Protobuf = Exception.SerializePartialAsString();
		XiaoRedis::PublishException(Protobuf, StdGuid);
		return EXCEPTION_CONTINUE_SEARCH;
	};

	static BOOL ConsoleHandler(DWORD signal)
	{
		if (signal == CTRL_C_EVENT)
			OnCancelKeyPress();
		return 0;
	}
#else
	static void ConsoleHandler(int sig)
	{
		OnCancelKeyPress();
		exit(-1);
	}
#endif

	static int WrappedMain(int argc, tchar* argv[])
	{
		GNeedFlush = true;

		const FString CmdLine = FCommandLine::BuildFromArgV(nullptr, argc, argv, TC(""));
		FCommandLine::Set(*CmdLine);

		FSchedulerContext Context;
		GContext = &Context;
		return Context.Run() ? 0 : -1;
	}
}


#if PLATFORM_WINDOWS
int wmain(int argc, wchar_t* argv[])
{
	using namespace uba;
	__try
	{
		return WrappedMain(argc, argv);
	}
	__except (ReportSEH(GetExceptionInformation()))
	{
		return -1;
	}
}
#else
int main(int argc, char* argv[])
{
	return uba::WrappedMain(argc, argv);
}
#endif
