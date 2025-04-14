/**
  * @author cxx2020@outlook.com
  * @datetime 2023 -11:45 PM
 */

#pragma once

#include "XiaoShare.h"
#include "Templates/UnrealTemplate.h"

#if !PLATFORM_WINDOWS
#define SERVICE_WIN32_SHARE_PROCESS 0
#define SERVICE_AUTO_START 1
#endif

struct FServiceCommandLineOptions
{
	FString Address;
	FString Port;
	FString RedeployFromPid;

	static FServiceCommandLineOptions FromString(const TCHAR* CommandLine)
	{
		return FServiceCommandLineOptions();
	}
	FString ToString(bool bIncludeRedeploy = false) const
	{
		return TEXT("");
	}
};

class FGenericService : FNoncopyable
{
public:
	explicit FGenericService(const FServiceCommandLineOptions& InOptions, const FServiceDesc& InServiceDesc)
		: Desc(InServiceDesc)
	{};
	virtual ~FGenericService() {};

	static FGenericService* Get() { return Singleton; }

	virtual bool OnInstall() { return false; };
	virtual bool OnRegister() { return false; };
	virtual bool OnStart() { return false; };
	virtual bool OnEnable() { return false; };
	virtual bool OnDisable() { return false; };
	virtual bool OnQuery() { return false; };
	virtual bool OnPause() { return false; };
	virtual bool OnContinue() { return false; };
	virtual bool OnTick(const float Internal) { return false; };
	virtual bool OnStop() { return false; };
	virtual bool OnDelete() { return false; };

protected:
	FServiceDesc Desc;
	static inline FGenericService* Singleton = nullptr;
	static inline constexpr float SIdleFrameTime = 1.0f / 1.0f;
};

#if PLATFORM_WINDOWS or PLATFORM_APPLE or PLATFORM_UNIX
#include COMPILED_PLATFORM_HEADER(XiaoService.h)
#else
typedef FGenericService FXiaoService;
#endif
