/**
  * @author cxx2020@outlook.com
  * @date 10:24 PM
 */
#pragma once

#include "XiaoShare.h"
#include "XiaoShareNetwork.h"
#include "XiaoUbaSettings.h"

using namespace XiaoAgentParam;

static const FString SAppSettingsFile = FPaths::ConvertRelativePathToFull(FString::Printf(TEXT("%s/XiaoBuild/%s.json"), 
#if PLATFORM_WINDOWS
	*FPaths::GetPath(FPlatformProcess::ApplicationSettingsDir()),
#elif PLATFORM_MAC
	TEXT("/Users/Shared"),
#elif PLATFORM_UNIX
	TEXT("/etc",)
#endif
	TEXT("AgentSettings"))
);
static const std::string GAgentUID = TCHAR_TO_UTF8(*GetUniqueDeviceID());

#define UTF82TCHAR(X) UTF8_TO_TCHAR(X.c_str())

#ifndef XIAO_LOG
#define XIAO_LOG(Verbosity, FMT, ...)
#endif


struct FSettingsBase
{
	virtual ~FSettingsBase()
	{}
	
	virtual void SetJsonObject(TSharedPtr<FJsonObject>& InOutJson) = 0;
	virtual bool UpdateByJson(const TSharedPtr<FJsonObject>& InJson) = 0;
	
	FORCEINLINE FString ToJsonString()
	{
		TSharedPtr<FJsonObject> SettingsObject = MakeShareable( new FJsonObject );
		SetJsonObject(SettingsObject);
		FString Temp;
		Json2String(SettingsObject, Temp);
		return Temp;
	}
	
	mutable bool bSame = true;
};


struct FAgentGeneral : FSettingsBase
{
	virtual ~FAgentGeneral() {};
	bool operator==(const FAgentGeneral& Other) const
	{
		bSame = KeepBuildsNum == Other.KeepBuildsNum &&
			Level == Other.Level &&
			bGenerateDetails == Other.bGenerateDetails &&
			bGenerateDetailsLocal == Other.bGenerateDetailsLocal;
		return bSame;
	}

	virtual void SetJsonObject(TSharedPtr<FJsonObject>& InOutJson) override
	{
		if(InOutJson.IsValid())
		{
			InOutJson->SetStringField(UTF82TCHAR(SIpAddress), Ip);
			InOutJson->SetStringField(UTF82TCHAR(SCpuInfor), CpuInfo);
			InOutJson->SetStringField(UTF82TCHAR(SMacAddress), MacAddress);
			InOutJson->SetNumberField(UTF82TCHAR(SAgentGeneralKeepBuildNum), KeepBuildsNum);
			InOutJson->SetNumberField(UTF82TCHAR(SAgentGeneralLevel), Level);
			InOutJson->SetBoolField(UTF82TCHAR(SAgentGeneralGeneralDetails), bGenerateDetails);
			InOutJson->SetBoolField(UTF82TCHAR(SAgentGeneralGeneralDetailsLocal), bGenerateDetailsLocal);
		}
	}

	virtual bool UpdateByJson(const TSharedPtr<FJsonObject>& InJson) override
	{
		if(InJson.IsValid())
		{
			InJson->TryGetStringField(UTF82TCHAR(SIpAddress), Ip);
			InJson->TryGetStringField(UTF82TCHAR(SCpuInfor), CpuInfo);
			InJson->TryGetStringField(UTF82TCHAR(SMacAddress), MacAddress);
			InJson->TryGetNumberField(UTF82TCHAR(SAgentGeneralKeepBuildNum), KeepBuildsNum);
			uint8 Temp = 0;
			if(InJson->TryGetNumberField(UTF82TCHAR(SAgentGeneralLevel), Temp))
			{
				Level = static_cast<ELogLevel>(Temp);
			}
			InJson->TryGetBoolField(UTF82TCHAR(SAgentGeneralGeneralDetails), bGenerateDetails);
			InJson->TryGetBoolField(UTF82TCHAR(SAgentGeneralGeneralDetailsLocal), bGenerateDetailsLocal);
			return true;
		}
		return false;
	}

	static FString GetLevelString(const ELogLevel InLevel)
	{
		switch (InLevel)
		{
		case ELogLevel::Level_Minimal:
			return TEXT("LogXiao Fatal, LogInit Fatal, LogMemory Fatal");
		case ELogLevel::Level_Basic:
			return TEXT("LogXiao Error, LogInit Error, LogMemory Error");
		case ELogLevel::Level_Intermeidate:
			return TEXT("LogXiao Display, LogInit Display, LogMemory Display");
		case ELogLevel::Level_Extended:
			return TEXT("LogXiao Log, LogInit Log, LogMemory Log");
		case ELogLevel::Level_Detailed:
			return TEXT("LogXiao Verbose, LogInit Verbose, LogMemory Verbose");
		}

		return TEXT("LogXiao Display, LogInit Display, LogMemory Display");
	}

	FString Ip = TEXT("127.0.0.1");
	FString CpuInfo = TEXT("");
	FString MacAddress = TEXT("");
	uint32 KeepBuildsNum = 60;
	ELogLevel Level = ELogLevel::Level_Minimal;
	bool bGenerateDetails = false;
	bool bGenerateDetailsLocal = false;
};


struct FNetworkGeneral : FSettingsBase
{
	virtual ~FNetworkGeneral() {};
	
	bool operator==(const FNetworkGeneral& Other) const
	{
		bSame = bAutoSelectPort == Other.bAutoSelectPort &&
				PrimaryPort == Other.PrimaryPort &&
				SecondPort == Other.SecondPort;
		return bSame;
	}

	virtual void SetJsonObject(TSharedPtr<FJsonObject>& InOutJson) override
	{
		if(InOutJson.IsValid())
		{
			InOutJson->SetBoolField(UTF82TCHAR(SNetworkGeneralAutoSelect), bAutoSelectPort);
			InOutJson->SetNumberField(UTF82TCHAR(SNetworkGeneralPrimaryPort), PrimaryPort);
			InOutJson->SetNumberField(UTF82TCHAR(SNetworkGeneralSecondPort), SecondPort);
		}
	}

	virtual bool UpdateByJson(const TSharedPtr<FJsonObject>& InJson) override
	{
		if(InJson.IsValid())
		{
			InJson->TryGetBoolField(UTF82TCHAR(SNetworkGeneralAutoSelect), bAutoSelectPort);
			InJson->TryGetNumberField(UTF82TCHAR(SNetworkGeneralPrimaryPort), PrimaryPort);
			InJson->TryGetNumberField(UTF82TCHAR(SNetworkGeneralSecondPort), SecondPort);
			return true;
		}
		return false;
	}
	
	bool bAutoSelectPort = false;
	uint16 PrimaryPort = XiaoNetwork::SAgentServicePort+1;
	uint16 SecondPort = XiaoNetwork::SAgentServicePort+2;

	mutable bool bSame = true;
};


struct FNetworkCoordinate : FSettingsBase
{
	virtual ~FNetworkCoordinate() {};
	
	bool operator==(const FNetworkCoordinate& Other) const
	{
		bSame = IP == Other.IP &&
				Port == Other.Port &&
				Desc == Other.Desc &&
				Group == Other.Group;
		return bSame;
	}

	virtual void SetJsonObject(TSharedPtr<FJsonObject>& InOutJson) override
	{
		if(InOutJson.IsValid())
		{
			InOutJson->SetStringField(UTF82TCHAR(SNetworkCoordiIp), IP);
			InOutJson->SetNumberField(UTF82TCHAR(SNetworkCoordiPort), Port);
			InOutJson->SetNumberField(UTF82TCHAR(SNetworkCoordiBackupPort), BackupPort);
			InOutJson->SetStringField(UTF82TCHAR(SNetworkCoordiDesc), Desc);
			InOutJson->SetStringField(UTF82TCHAR(SNetworkCoordiGroup), Group);
		}
	}

	virtual bool UpdateByJson(const TSharedPtr<FJsonObject>& InJson) override
	{
		if(InJson.IsValid())
		{
			InJson->TryGetStringField(UTF82TCHAR(SNetworkCoordiIp), IP);
			InJson->TryGetNumberField(UTF82TCHAR(SNetworkCoordiPort), Port);
			InJson->TryGetNumberField(UTF82TCHAR(SNetworkCoordiBackupPort), BackupPort);
			InJson->TryGetStringField(UTF82TCHAR(SNetworkCoordiDesc), Desc);
			InJson->TryGetStringField(UTF82TCHAR(SNetworkCoordiGroup), Group);
			return true;
		}
		return false;
	}
	
	FString IP = "127.0.0.1";
	uint16 Port = XiaoNetwork::SCoordiServicePort;
	uint16 BackupPort = XiaoNetwork::SCoordiServicePort;
	FString Desc = "None";
	FString Group = "Default";
};


struct FInitiatorGeneral : FSettingsBase
{
	virtual ~FInitiatorGeneral() {};
	
	bool operator==(const FInitiatorGeneral& Other) const
	{
		bSame = bRestartRemoteProcess == Other.bRestartRemoteProcess &&
				bAvoidTaskExecution == Other.bAvoidTaskExecution &&
				bEnableStandoneMode == Other.bEnableStandoneMode &&
				bLimitLogicCore == Other.bLimitLogicCore &&
				LimitMaximumCoreNum == Other.LimitMaximumCoreNum;
		return bSame;
	}

	virtual void SetJsonObject(TSharedPtr<FJsonObject>& InOutJson) override
	{
		if(InOutJson.IsValid())
		{
			InOutJson->SetBoolField(UTF82TCHAR(SInitiatorGeneralRestartRemote), bRestartRemoteProcess);
			InOutJson->SetBoolField(UTF82TCHAR(SInitiatorGeneralAvoidTask), bAvoidTaskExecution);
			InOutJson->SetBoolField(UTF82TCHAR(SInitiatorGeneralEnableStandone), bEnableStandoneMode);
			InOutJson->SetBoolField(UTF82TCHAR(SInitiatorGeneralLitmitCore), bLimitLogicCore);
			InOutJson->SetNumberField(UTF82TCHAR(SInitiatorGeneralLimitMaxCoreNum), LimitMaximumCoreNum);
		}
	}

	virtual bool UpdateByJson(const TSharedPtr<FJsonObject>& InJson) override
	{
		if(InJson.IsValid())
		{
			InJson->TryGetBoolField(UTF82TCHAR(SInitiatorGeneralRestartRemote), bRestartRemoteProcess);
			InJson->TryGetBoolField(UTF82TCHAR(SInitiatorGeneralAvoidTask), bAvoidTaskExecution);
			InJson->TryGetBoolField(UTF82TCHAR(SInitiatorGeneralEnableStandone), bEnableStandoneMode);
			InJson->TryGetBoolField(UTF82TCHAR(SInitiatorGeneralLitmitCore), bLimitLogicCore);
			InJson->TryGetNumberField(UTF82TCHAR(SInitiatorGeneralLimitMaxCoreNum), LimitMaximumCoreNum);
			return true;
		}
		return false;
	}
	
	bool bRestartRemoteProcess = false;
	bool bAvoidTaskExecution = false;
	bool bEnableStandoneMode = false;
	bool bLimitLogicCore = false;
	uint32 LimitMaximumCoreNum = 0;
};


struct FAgentSettings : FSettingsBase
{
	virtual ~FAgentSettings() {};
	
	FAgentSettings& operator=(const FAgentSettings& Other) = default;
	
	bool operator==(const FAgentSettings& Other) const
	{
		bSame = AgentGeneral == Other.AgentGeneral && NetworkGeneral == Other.NetworkGeneral && NetworkCoordinate == Other.NetworkCoordinate && UbaAgent == Other.UbaAgent && UbaScheduler == Other.UbaScheduler && Localization == Other.Localization;
		return bSame;
	}

	bool IsPatchFinsih() const
	{
		return AgentGeneral.bSame && NetworkGeneral.bSame && NetworkCoordinate.bSame;
	}

	void ResetFlag(const std::string& InSeciton)
	{
		if (InSeciton == XiaoAgentParam::SAgentGeneral)
		{
			AgentGeneral.bSame = true;
		}
		else if(InSeciton == XiaoAgentParam::SNetworkGeneral)
		{
			NetworkGeneral.bSame = true;
		}
		else if(InSeciton == XiaoAgentParam::SNetworkCoordi)
		{
			NetworkCoordinate.bSame = true;
		}
		else if (InSeciton == XiaoAgentParam::SUbaAgent)
		{
			UbaAgent.bSame = true;
		}
		else if (InSeciton == XiaoAgentParam::SUbaScheduler)
		{
			UbaScheduler.bSame = true;
		}
	}

	virtual void SetJsonObject(TSharedPtr<FJsonObject>& InOutJson) override
	{
		if (InOutJson.IsValid())
		{
			TSharedPtr<FJsonObject> AgentGeneralObject = MakeShareable(new FJsonObject);
			AgentGeneral.SetJsonObject(AgentGeneralObject);
			InOutJson->SetObjectField(UTF82TCHAR(SAgentGeneral), AgentGeneralObject);

			TSharedPtr<FJsonObject> NetworkGeneralObject = MakeShareable(new FJsonObject);
			NetworkGeneral.SetJsonObject(NetworkGeneralObject);
			InOutJson->SetObjectField(UTF82TCHAR(SNetworkGeneral), NetworkGeneralObject);
			TSharedPtr<FJsonObject> NetworkCoordiObject = MakeShareable(new FJsonObject);
			NetworkCoordinate.SetJsonObject(NetworkCoordiObject);
			InOutJson->SetObjectField(UTF82TCHAR(SNetworkCoordi), NetworkCoordiObject);

			TSharedPtr<FJsonObject> UbaAgentObject = MakeShareable(new FJsonObject);
			if (String2Json(UbaAgent.ToJson(), UbaAgentObject))
			{
				InOutJson->SetObjectField(UTF82TCHAR(XiaoAgentParam::SUbaAgent), UbaAgentObject);
			}

			TSharedPtr<FJsonObject> UbaSchedulerObject = MakeShareable(new FJsonObject);
			if (String2Json(UbaScheduler.ToJson(), UbaSchedulerObject))
			{
				InOutJson->SetObjectField(UTF82TCHAR(XiaoAgentParam::SUbaScheduler), UbaSchedulerObject);
			}

			InOutJson->SetBoolField(SEnableUBAC, bEnableUbac);
			InOutJson->SetStringField(SLocalization, Localization);
		}
	}

	virtual bool UpdateByJson(const TSharedPtr<FJsonObject>& InJson) override
	{
		bool Flag = true;
		if(InJson.IsValid())
		{
			Flag = Flag && InJson->TryGetBoolField(SEnableUBAC, bEnableUbac);
			FString Temp;
			InJson->TryGetStringField(SLocalization, Temp);
			Localization = Temp;

			const TSharedPtr<FJsonObject>* AgentGeneralObject = nullptr;
			if(InJson->TryGetObjectField(UTF82TCHAR(SAgentGeneral), AgentGeneralObject))
			{
				Flag = AgentGeneral.UpdateByJson(*AgentGeneralObject) && Flag;
			}

			const TSharedPtr<FJsonObject>* NetworkGeneralObject = nullptr;
			if(InJson->TryGetObjectField(UTF82TCHAR(SNetworkGeneral), NetworkGeneralObject))
			{
				Flag = NetworkGeneral.UpdateByJson(*NetworkGeneralObject) && Flag;
			}
			const TSharedPtr<FJsonObject>* NetworkCoordiObject = nullptr;
			if(InJson->TryGetObjectField(UTF82TCHAR(SNetworkCoordi), NetworkCoordiObject))
			{
				Flag = NetworkCoordinate.UpdateByJson(*NetworkCoordiObject) && Flag;
			}

			const TSharedPtr<FJsonObject>* UbaAgentObject = nullptr;
			if (InJson->TryGetObjectField(UTF82TCHAR(XiaoAgentParam::SUbaAgent), UbaAgentObject))
			{
				FString JsonStr;
				Json2String(*UbaAgentObject, JsonStr);
				Flag = UbaAgent.FromJson(JsonStr) && Flag;
			}

			const TSharedPtr<FJsonObject>* UbaSchedulerObject = nullptr;
			if (InJson->TryGetObjectField(UTF82TCHAR(XiaoAgentParam::SUbaScheduler), UbaSchedulerObject))
			{
				FString JsonStr;
				Json2String(*UbaSchedulerObject, JsonStr);
				Flag = UbaScheduler.FromJson(JsonStr) && Flag;
			}
		}
		return Flag;
	}
	
	FNetworkGeneral NetworkGeneral;
	FNetworkCoordinate NetworkCoordinate;

	FAgentGeneral AgentGeneral;
	FUbaAgentSetting UbaAgent;

	FUbaSchedulerSetting UbaScheduler;

	bool bEnableUbac = true;
	// 0::Chinese 1::English
	FString Localization = TEXT("zh-CN");
};
inline FAgentSettings SModifiedAgentSettings;
inline FAgentSettings SOriginalAgentSettings;
static const TMap<FString, FString> GLauguage2Loc = {
	{TEXT("简体中文"), TEXT("zh-CN")},
	{TEXT("English"), TEXT("en")}
};

static bool SaveAgentSettings(FAgentSettings& InSettings)
{
	if (!FFileHelper::SaveStringToFile(InSettings.ToJsonString(), *SAppSettingsFile))
	{
		XIAO_LOG(Error, TEXT("SaveStringToFile Failed::%s"), *SAppSettingsFile);
		return false;
	}

	return true;
}

static bool LoadAgentSettings(FAgentSettings& OutSettings, const bool bIgonreTime = false)
{
	if (!FPaths::FileExists(SAppSettingsFile))
	{
		XIAO_LOG(Error, TEXT("File not exists::%s"), *SAppSettingsFile);
		GLog->Flush();
		return false;
	}

	static bool bFirst = true;
	static FDateTime DataTime;
	if (!bFirst && !bIgonreTime)
	{
		const FFileStatData StatData = IFileManager::Get().GetStatData(*SAppSettingsFile);
		if ((StatData.ModificationTime - DataTime).GetTotalSeconds() <= 1.0f)
		{
			return true;
		}
		DataTime = StatData.ModificationTime;
	}
	bFirst = false;

	FString Content;
	if (!FFileHelper::LoadFileToString(Content, *SAppSettingsFile))
	{
		XIAO_LOG(Error, TEXT("LoadFileToString Failed::%s"), *SAppSettingsFile);
		GLog->Flush();
		return false;
	}

	TSharedPtr<FJsonObject> SettingsObj = nullptr;
	if (!String2Json(Content, SettingsObj))
	{
		XIAO_LOG(Error, TEXT("Can't Parse::%s"), *SAppSettingsFile);
		GLog->Flush();
		return false;
	}

	OutSettings.UpdateByJson(SettingsObj);

	// 更新全局参数
	XiaoNetwork::SLicenseServiceListen = FString::Printf(TEXT("http://%s:%d"), *OutSettings.NetworkCoordinate.IP, XiaoNetwork::SLicenseServicePort);
	XiaoNetwork::SCoordiServiceListen = FString::Printf(TEXT("http://%s:%d"), *OutSettings.NetworkCoordinate.IP, XiaoNetwork::SCoordiServicePort);
	XiaoNetwork::SPerfServiceListen = FString::Printf(TEXT("http://%s:%d"), *OutSettings.NetworkCoordinate.IP, XiaoNetwork::SIPerfServicePort);
	return true;
}

static void RunXiaoApp(const FString& InAppName, const FString& InParam = TEXT(""), bool bShowConsole = false, const bool bDetach = true, const bool bAsAdmin = false, const bool bHasExtension = false, const bool bIgnoreTime = true)
{
	FString FakeAppName = TEXT("XiaoAgentSettings");
	if (FParse::Value(*InParam, TEXT("app"), FakeAppName))
	{
		FakeAppName = FakeAppName.RightChop(1);
	}

	LoadAgentSettings(SOriginalAgentSettings, bIgnoreTime);
	const FString Params = FString::Printf(TEXT("%s -LogCmds=\"%s\" -CULTURE=%s -LOG=%s.log"),
		*InParam,
		*FAgentGeneral::GetLevelString(SOriginalAgentSettings.AgentGeneral.Level),
		*SOriginalAgentSettings.Localization,
		*FakeAppName
	);
	XIAO_LOG(Log, TEXT("RunXiaoApp::AppName:%s Params:%s"), *InAppName, *Params);

	void* PipeRead = nullptr;
	void* PipeWrite = nullptr;
	uint32 ProcessID = 0;

	if (bAsAdmin)
	{
		RunAsAdmin(InAppName, Params, true);
	}
	else
	{
		const FString XiaoAppPath = GetXiaoAppPath(InAppName, TEXT(""), false);
		const FString WorkingDir = FPlatformProcess::GetCurrentWorkingDirectory();
		FProcHandle Handle = FPlatformProcess::CreateProc(*XiaoAppPath, *Params, bDetach, false, false, &ProcessID, 0, *WorkingDir, PipeRead, PipeWrite);
		if (!Handle.IsValid())
		{
			XIAO_LOG(Error, TEXT("FPlatformProcess::CreateProc Failed::%s Params::%s"), *XiaoAppPath, *Params);
		}
	}
}

static bool SetServiceState(const FString& InServiceName, const bool InbStart, FString& OutError)
{
	int32 RtnCode = -1;
	FString Out;
#if PLATFORM_WINDOWS
	const FString Url = TEXT("C:/Windows/System32/sc.exe");
	const FString Param = FString::Printf(TEXT("%s %s"), InbStart ? TEXT("start") : TEXT("stop"), *InServiceName);
	FPlatformProcess::CreateProc(*Url, *Param, false, true, true, nullptr, 0, nullptr, nullptr, nullptr);
	return true;
#elif PLATFORM_UNIX
	const FString Url = TEXT("/usr/bin/systemctl");
	const FString Param = FString::Printf(TEXT("%s %s"), InbStart ? TEXT("restart") : TEXT("stop"), *InServiceName);
#elif PLATFORM_MAC
	const bool bIsDaemons = InServiceName==TEXT("XiaoCoordiService");
	const FString Scope = bIsDaemons ? TEXT("system") : FString::Printf(TEXT("gui/%d"), getuid());
	const FString BootStrap = bIsDaemons ? TEXT("bootstrap") : TEXT("load");
	const FString BootOut = bIsDaemons ? TEXT("bootout") : TEXT("unload");
	const FString Url = TEXT("/bin/launchctl");
	const FString Param = FString::Printf(TEXT("%s %s /Library/Launch%s/com.XiaoBuild.%s.plist"), 
							*(InbStart ? BootStrap : BootOut), 
							*Scope,
							(bIsDaemons ? TEXT("Daemons") : TEXT("Agents")), 
							*InServiceName
						);
#else
	check(0);
#endif
	return FPlatformProcess::ExecProcess(*Url, *Param, &RtnCode, &Out, &OutError);
}

#undef UTF82TCHAR