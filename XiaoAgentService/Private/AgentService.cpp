/**
  * @author cxx2020@outlook.com
  * @datetime 2023 -9:32 PM
 */
#include "AgentService.h"
#include "Global.h"

#include "system_settings.pb.h"
#include "XiaoShareRedis.h"
#include "XiaoAgent.h"
#include "XiaoShareNetwork.h"

#include "Agent/GenericAgentStatsMonitor.h"
#include <format>

#if PLATFORM_MAC
#include <unistd.h>
#include <pwd.h>
#endif


using namespace XiaoNetwork;
#ifdef REDIS_CPLUS_CPLUS
using namespace sw::redis;
#endif
using namespace XiaoRedis;

static constexpr double SOneDaySeconds = 3600.0f * 24.0f;

static ConnectionOptions SRedisConOptions;
static FAgentProto SAgentProto;
static FSystemSettings SSystemSettings;
static FAgentStatsMonitor SAgentStats;
static FProcHandle SUbaAgentHandle;
static void* SUbaAgentPipeReadChild = nullptr;
static void* SUbaAgentPipeWriteChild = nullptr;
static FProcHandle SIPerfProcHandle;
static std::string SIpv4Address = TCHAR_TO_UTF8(*GetLANV4());
static std::string SDefaultGroup = "Default";
static bool SAgentStatusChanged = false;
static uint32 SDetectAgentStatus = 0;


struct FAgentCoreParams
{
	uint32 MaxCon = 0;
	uint32 MaxCpu = 0;
	uint32 LogLevel = 0;
	FUbaAgentSetting AgentSetting;

	bool Init = false;

	explicit FAgentCoreParams()
	{}

	explicit FAgentCoreParams(const uint32 InMaxCon, const uint32 InMaxCpu, const uint32 InLogLevel, const FUbaAgentSetting& InAgentSettings)
		: MaxCon(InMaxCon)
		, MaxCpu(InMaxCpu)
		, LogLevel(InLogLevel)
		, AgentSetting(InAgentSettings)
	{}

	bool operator==(const FAgentCoreParams& InAnother) const
	{
		return this->MaxCon == InAnother.MaxCon && this->MaxCpu == InAnother.MaxCpu && this->LogLevel == InAnother.LogLevel && AgentSetting.IsEqual(InAnother.AgentSetting);
	}

	bool operator!=(const FAgentCoreParams& InAnother)
	{
		return !(*this == InAnother);
	}
}; FAgentCoreParams SAgentCoreParams;

static bool SbNeedRestart = false;
static FDateTime SLastCleanTime = FDateTime::MinValue();
static double SbNeedClean = false;


static std::string GetCpuArchitecture()
{
#if defined(__x86_64__) || defined(_M_X64)
	return "x86_64";
#elif defined(__i386) || defined(_M_IX86)
	return "x86 (32-bit)";
#elif defined(__aarch64__) || defined(_M_ARM64)
	return "ARM64";
#elif defined(__arm__) || defined(_M_ARM)
	return "ARM (32-bit)";
#else
	return "Unknown";
#endif
}

static FORCEINLINE bool TryGetPublicIp()
{
	if (IsConnected())
	{
		SIpv4Address = TCHAR_TO_UTF8(*GetLANV4());
		try
		{
			if (const auto ClientsReplay = SRedisClient->command({ "CLIENT", "LIST" }))
			{
				const FString Result = UTF8_TO_TCHAR(ClientsReplay->str);
				TArray<FString> List;
				Result.ParseIntoArrayLines(List, false);
				const FString TargetName = FString::Printf(TEXT("name=%s"), *GetUniqueDeviceID());
				for (const FString& Line : List)
				{
					if (Line.Contains(TargetName))
					{
						FString Left, Right;
						if (Line.Split(TEXT("addr="), &Left, &Right))
						{
							if (Right.Split(TEXT("fd="), &Left, &Right))
							{
								if (Left.Split(TEXT(":"), &Left, &Right))
								{
									SIpv4Address = TCHAR_TO_UTF8(*Left);
									return true;
								}
							}
						}
						break;
					}
				}
			}
		}
		CATCH_REDIS_EXCEPTRION();
	}
	return false;
}


bool FAgentService::OnInitialize(const FString& InParams)
{
	GNeedFlush = true;
	XIAO_LOG(Display, TEXT("OnInitialize::Begin!"));

	TSharedPtr<FJsonObject> ConfigObj = nullptr;
	if(!ReadConfig(ConfigObj))
	{
		XIAO_LOG(Error, TEXT("Can't read config!"));
		return false;
	}

	GOnRedisChanged.Bind([](const uint8 InStatus)
	{
		if (!SRedisMessage.empty() && InStatus != ERedisStatus::Redis_Ok)
		{
			XIAO_LOG(Error, TEXT("%s"), UTF8_TO_TCHAR(SRedisMessage.c_str()));
		}
	});

	// 读取本地代理参数
	{
		LoadAgentSettings(SOriginalAgentSettings);
		const auto& NetworkCoordi = SOriginalAgentSettings.NetworkCoordinate;
		GMasterConnection.host = TCHAR_TO_UTF8(*NetworkCoordi.IP);
		GMasterConnection.port = NetworkCoordi.Port;
		GMasterConnection.keep_alive = true;
		TryConnectRedis(true);
	}
	const FString CASDir = SOriginalAgentSettings.UbaAgent.Dir;
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	if (!PlatformFile.DirectoryExists(*CASDir))
	{
		PlatformFile.CreateDirectoryTree(*CASDir);
	}
	const auto StatData = PlatformFile.GetStatData(*CASDir);
	SLastCleanTime = StatData.CreationTime;

	SAgentStats.Initialize();

	// 初始化更新代理数据
	UpdateAgentStats(true);

	if (!TryRunUbaAgent())
	{
		XIAO_LOG(Error, TEXT("Failed to run UbaAgent!"));
		return false;
	}

	// IPerf Server
	TryRunIPerfServer();

	XIAO_LOG(Display, TEXT("OnInitialize::Finish!"));
	return true;
}

static void ShutdownUbaAgent()
{
	if (SUbaAgentHandle.IsValid())
	{
		XIAO_LOG(Display, TEXT("For Reboot to shutdown the UbaAgent!"));
		FPlatformProcess::TerminateProc(SUbaAgentHandle);
		FPlatformProcess::CloseProc(SUbaAgentHandle);
	}
}

void FAgentService::OnDeinitialize()
{
	XIAO_LOG(Display, TEXT("OnDeinitialize::Begin!"));
	
	SAgentProto.set_status(Status_Stopped);
	UpdateAgentStats();
	SAgentStats.Deinitialize();
	TryDisconnectRedis();

	if (SUbaAgentPipeReadChild || SUbaAgentPipeWriteChild)
	{
		FPlatformProcess::ClosePipe(SUbaAgentPipeReadChild, SUbaAgentPipeWriteChild);
		SUbaAgentPipeReadChild = nullptr;
		SUbaAgentPipeWriteChild = nullptr;
	}

	if (SIPerfProcHandle.IsValid())
	{
		FPlatformProcess::TerminateProc(SIPerfProcHandle);
		FPlatformProcess::CloseProc(SIPerfProcHandle);
	}

	ShutdownUbaAgent();
	
	XIAO_LOG(Display, TEXT("OnDeinitialize::Finish!"));
}

static void Update()
{
	std::string Protobuf;
	if (SAgentProto.SerializeToString(&Protobuf) && Protobuf.size() > 0)
	{
		SRedisClient->hset(Hash::SAgentStats, GAgentUID, Protobuf);
	}
	else
	{
		XIAO_LOG(Error, TEXT("SAgentProto.SerializeToString failed!"));
	}
}

static bool IsNeedReboot()
{
	return SbNeedRestart && (SAgentProto.status() == 0);
}

void FAgentService::OnTick(const float InDeltaTime)
{
	if (SUbaAgentPipeReadChild)
	{
		const FString Msg = FPlatformProcess::ReadPipe(SUbaAgentPipeReadChild);
		if (!Msg.IsEmpty())
		{
			if (Msg.Contains(TEXT("Connected to server")))
			{
				SAgentStatusChanged = true;
				SDetectAgentStatus = 2;
			}
			else if (Msg.Contains(TEXT("Listening on")) || Msg.Contains(TEXT("Disconnected from server...")))
			{
				SAgentStatusChanged = true;
				SDetectAgentStatus = 0;
			}

			TArray<FString> Logs;
			Msg.ParseIntoArray(Logs, TEXT("\n"));
			for (const auto& Log : Logs)
			{
				XIAO_LOG(VeryVerbose, TEXT("%s"), *Log);
			}
		}
	}

	static double LastTime = 0.0f;
	if ((FPlatformTime::Seconds() - LastTime) < GSleepUpdate)
	{
		return;
	}
	LastTime = FPlatformTime::Seconds();

	if (!FPlatformProcess::IsApplicationRunning(*XiaoAppName::SUbaAgent))
	{
		TryRunUbaAgent();
	}

	const bool Rtn = LoadAgentSettings(SOriginalAgentSettings);
	if(!UpdateClusterConfig() || !UpdateAgentStats())
	{
		if (Rtn)
		{
			const auto& NetworkCoordi = SOriginalAgentSettings.NetworkCoordinate;
			GMasterConnection.host = TCHAR_TO_UTF8(*NetworkCoordi.IP);
			GMasterConnection.port = NetworkCoordi.Port;
			GMasterConnection.keep_alive = true;
			if (TryConnectRedis(true))
			{
				SAgentProto.set_status(0);
				Update();
			}
			else
			{
				XIAO_LOG(Error, TEXT("TryConnectRedis [%s::%u] Failed!"), *NetworkCoordi.IP, NetworkCoordi.Port);
			}
		}
		else
		{
			XIAO_LOG(Error, TEXT("LoadAgentSettings Failed!"));
		}
	}

	// 核心参数变化需要重启
	if (IsNeedReboot())
	{
		ShutdownUbaAgent();
		if (SbNeedClean)
		{
			SLastCleanTime = FDateTime::Now();
			SbNeedClean = false;
			const FString CASDir = SOriginalAgentSettings.UbaAgent.Dir;
			IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
			if (PlatformFile.DirectoryExists(*CASDir))
			{
				PlatformFile.DeleteDirectoryRecursively(*CASDir);
				XIAO_LOG(Log, TEXT("Schdule Clean agent CAS Content::%s"), *CASDir);
			}
			PlatformFile.CreateDirectoryTree(*CASDir);
		}
		TryRunUbaAgent();
	}

	if (!SIPerfProcHandle.IsValid() || !IsAppRunning(XiaoAppName::SIperfServer))
	{
		// TODO iperf服务可能会拒绝连接
		TryRunIPerfServer();
	}
}

bool FAgentService::UpdateClusterConfig()
{
	if (!IsConnected())
	{
		XIAO_LOG(Error, TEXT("UpdateClusterConfig::Redis Client is not valid!"));
		return false;
	}

	std::unordered_map<std::string, std::string> Result;
	FRedisCluster Cluster;
	try
	{
		SRedisClient->hgetall(Hash::SCacheList, std::inserter(Result, Result.begin()));
		for (const auto& Iter : Result)
		{
			const FString NodeJson = UTF8_TO_TCHAR(Iter.second.c_str());
			FRedisServerDesc Node;
			if (Node.FromJson(NodeJson))
			{
				Cluster.Nodes.Add(Node);
			}
		}
		const FString JsonContent = Cluster.ToJson(true);
		if (!JsonContent.IsEmpty())
		{
			const FString ConfigFolder = FPaths::GetPath(SRedisClusterFile);
			if (!FPaths::DirectoryExists(ConfigFolder))
			{
				IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
				if (!PlatformFile.CreateDirectoryTree(*ConfigFolder))
				{
					XIAO_LOG(Error, TEXT("Failed to create folder::%s!"), *ConfigFolder);
				}
			}
			if (!FFileHelper::SaveStringToFile(JsonContent, *SRedisClusterFile))
			{
				XIAO_LOG(Error, TEXT("Failed to save redis cluster config file!"));
			}
		}

		return true;
	}
	CATCH_REDIS_EXCEPTRION();
	XIAO_LOG(Error, TEXT("UpdateClusterConfig::RedisException::%s"), UTF8_TO_TCHAR(SRedisMessage.c_str()));
	return false;
}

bool FAgentService::UpdateAgentStats(const bool bInit)
{
	if(!IsConnected())
	{
		XIAO_LOG(Error, TEXT("UpdateAgentStats::Redis Client is not valid!"));
		return false;
	}

	try
	{
		// 先获取,或许有改变
		if (SRedisClient->hexists(Hash::SAgentStats, GAgentUID))
		{
			const auto Optional = SRedisClient->hget(Hash::SAgentStats, GAgentUID);
			if (Optional.has_value())
			{
				if (const std::string Value = Optional.value(); !(Value.size() > 0 && SAgentProto.ParseFromString(Value)))
				{
					XIAO_LOG(Error, TEXT("Agent proto parse failed!"));
				}
			}

			if (SAgentCoreParams.Init && !SbNeedRestart)
			{
				// 是否需要定时清理
				const uint32 ScheduleTime = SSystemSettings.scheduletime() > 1 ? SSystemSettings.scheduletime() : 3;
				if (SSystemSettings.bscheduleclean() && (FDateTime::Now() - SLastCleanTime).GetTotalSeconds() > (ScheduleTime * SOneDaySeconds))
				{
					SbNeedClean = true;
				}

				// 是否参数发生变化
				const FAgentCoreParams TempParams(static_cast<uint32>(SAgentProto.maxcon()), static_cast<uint32>(SAgentProto.helpercore()), static_cast<uint32>(SAgentProto.loglevel()), SOriginalAgentSettings.UbaAgent);
				if (SAgentCoreParams != TempParams || SbNeedClean)
				{
					SAgentCoreParams.AgentSetting = SOriginalAgentSettings.UbaAgent;
					SbNeedRestart = true;
					XIAO_LOG(Warning, TEXT("Detect different param value with server need restart the agent!"));
				}
			}
		}
		
		// 本地统计数据
		SAgentStats.UpdateAgentStats(SAgentProto, SOriginalAgentSettings.UbaAgent.Dir);

		// 更新核心状态
		UpdateAgentProtobuf(bInit);

		// 更新代理设置
		UpdateAgentSettings();
		return true;
	}
	CATCH_REDIS_EXCEPTRION();
	XIAO_LOG(Error, TEXT("UpdateAgentStats::RedisException::%s"), UTF8_TO_TCHAR(SRedisMessage.c_str()));
	return false;
}

void FAgentService::UpdateAgentSettings()
{
	const auto UpdateFrequencyStr = SRedisClient->get(String::SSystemSettings);
	if (UpdateFrequencyStr.has_value())
	{
		const std::string SettingsStr = UpdateFrequencyStr.value();
		if (SettingsStr.size() > 0 && SSystemSettings.ParseFromString(SettingsStr))
		{
			GSleepUpdate = SSystemSettings.syncfreq();
		}
	}
}

void FAgentService::UpdateAgentProtobuf(const bool bInit)
{
	const auto LogicCoreNum = GetLogicProcessorNum();

	if (bInit)
	{
		SAgentProto.set_status(0);
		*SAgentProto.mutable_id() = GAgentUID;
		SAgentProto.set_helperport(XiaoNetwork::SAgentServicePort);
		SAgentProto.set_logiccore(LogicCoreNum);
		SAgentProto.set_physicalcore(FPlatformMisc::NumberOfCores());
#if PLATFORM_MAC
		uid_t uid = geteuid();  // 获取当前用户 ID
		struct passwd* pw = getpwuid(uid);
		if (pw)
		{
			*SAgentProto.mutable_username() = pw->pw_name;
			*SAgentProto.mutable_loginuser() = pw->pw_name;
		}
#else
		*SAgentProto.mutable_username() =  TCHAR_TO_UTF8(*FPlatformMisc::GetLoginId());
		*SAgentProto.mutable_loginuser() = TCHAR_TO_UTF8(FPlatformProcess::ComputerName());
#endif
		if (SAgentProto.routerip().empty())
		{
			*SAgentProto.mutable_ip() = SIpv4Address;
			*SAgentProto.mutable_routerip() = SIpv4Address;
		}
		
		*SAgentProto.mutable_cpuinfo() = TCHAR_TO_UTF8(*FString::Printf(TEXT("%s-%s"), *FPlatformMisc::GetCPUVendor(), *FPlatformMisc::GetCPUBrand()));
		*SAgentProto.mutable_macaddress() = TCHAR_TO_UTF8(*FPlatformMisc::GetMacAddressString());
		FString OSVersionLabel, OSSubVersionLabel;
		FPlatformMisc::GetOSVersions(OSVersionLabel, OSSubVersionLabel);
		*SAgentProto.mutable_opsystem() = TCHAR_TO_UTF8(*(OSVersionLabel + OSSubVersionLabel));
		*SAgentProto.mutable_username() = TCHAR_TO_UTF8(FPlatformProcess::UserName());
		const std::string LastCon = TCHAR_TO_UTF8(*FDateTime::Now().ToString());
		SAgentProto.set_lastcon(LastCon);
		const std::string CpuArch = GetCpuArchitecture();
		SAgentProto.set_cpuarch(CpuArch);
		*SAgentProto.mutable_version() = TCHAR_TO_UTF8(XB_VERSION_STRING);
		*SAgentProto.mutable_gpudesc() = TCHAR_TO_UTF8(*SAgentStats.GetGPUDesc());
	}

	// 初始化默认值
	if (SAgentProto.group().empty())
	{
		SAgentProto.set_group(SDefaultGroup);
	}
	if (SAgentProto.helpercore() <= 0)
	{
		SAgentProto.set_helpercore(LogicCoreNum);
	}
	if (SAgentProto.maxcon() <= 0)
	{
		SAgentProto.set_maxcon(FPlatformMisc::NumberOfCores());
	}
	if (SAgentProto.maxcpu() <= 0)
	{
		SAgentProto.set_maxcpu(200);
	}
	if (SAgentProto.localmaxcpu() <= 0)
	{
		SAgentProto.set_localmaxcpu(LogicCoreNum);
	}
	// Schedualer意外终止检查
	if (SAgentProto.status() == 1 && !FPlatformProcess::IsApplicationRunning(*XiaoAppName::SXiaoScheduler))
	{
		SAgentStatusChanged = true;
		SDetectAgentStatus = 0;
	}
	if (SAgentStatusChanged)
	{
		SAgentProto.set_status(SDetectAgentStatus);
		if (SDetectAgentStatus == 0)
		{
			std::string EmptyMessage;
			SAgentProto.set_message(EmptyMessage);
		}
		SAgentStatusChanged = false;
	}
	// IP地址保证1Min左右检测一次,IP地址有可能发生改变
	static double LastCheckIp = FPlatformTime::Seconds();
	if ((FPlatformTime::Seconds() - LastCheckIp) > 60.0f)
	{
		LastCheckIp = FPlatformTime::Seconds();
		SIpv4Address = TCHAR_TO_UTF8(*GetLANV4());
		if (!SIpv4Address.empty())
		{
			// 无效IP地址
			const std::string LANIP = SAgentProto.routerip();
			if (LANIP.empty() || SIpv4Address != LANIP)
			{
				*SAgentProto.mutable_routerip() = SIpv4Address;
			}
		}
	}

	// 代理设置数据
	SAgentProto.set_lastupdate(FPlatformTime::Cycles64());
	SAgentProto.set_helperport(XiaoNetwork::SAgentServicePort);

	// Update更新代理数据
	Update();
}

bool FAgentService::TryRunUbaAgent()
{
	FString XiaoAgentName = XiaoAppName::SUbaAgent;
#if PLATFORM_WINDOWS
	XiaoAgentName += TEXT(".exe");
#endif

	const FString XiaoAgentExePath = FPaths::ConvertRelativePathToFull(FPaths::Combine(FPaths::GetPath(FPlatformProcess::ExecutablePath()), SMiddlePath, XiaoAgentName));
	if (!FPaths::FileExists(XiaoAgentExePath))
	{
		XIAO_LOG(Error, TEXT("XiaoAgent executable file not exist::%s!"), *XiaoAgentExePath);
		return false;
	}

	SUbaAgentHandle = GetProcHandle(XiaoAgentExePath);
	if (SUbaAgentHandle.IsValid())
	{
		XIAO_LOG(Display, TEXT("XiaoAgent Alreay running!"));
		SbNeedRestart = false;
		return true;
	}

	if (!FPlatformProcess::CreatePipe(SUbaAgentPipeReadChild, SUbaAgentPipeWriteChild))
	{
		XIAO_LOG(Error, TEXT("CreatePipe Failed!"));
		return false;
	}

	uint32 MaxCon = SOriginalAgentSettings.UbaAgent.MaxCon;
	if (SAgentProto.maxcon() > 0)							
	{
		MaxCon = SAgentProto.maxcon();
		SAgentCoreParams.MaxCon = MaxCon;
	}
	uint32 MaxCpu = SOriginalAgentSettings.UbaAgent.MaxCpu;
	if (SAgentProto.maxcpu() >= 0)
	{
		MaxCpu = SAgentProto.helpercore();
		SAgentCoreParams.MaxCpu = MaxCpu;
	}
	SAgentCoreParams.LogLevel = SAgentProto.loglevel();
	SAgentCoreParams.Init = true;
	SbNeedRestart = false;

	// 检测端口是否已被占用
	uint16 OutPort = XiaoNetwork::SAgentServicePort;
	const bool bRtn = GetUsablePort(static_cast<uint16>(SAgentProto.helperport()), OutPort);
	if (!bRtn)
	{
		XIAO_LOG(Error, TEXT("Not Usable port to use!"));
	}

	if (bRtn)
	{
		if (OutPort != SAgentProto.helperport())
		{
			XiaoNetwork::SAgentServicePort = OutPort;
		}
	}

	const FUbaAgentSetting& s = SOriginalAgentSettings.UbaAgent;
	const FString Params = FString::Printf(TEXT("%s -listen=%u -maxcpu=%u -mulcpu=%u -maxcon=%u -maxworkers=%u -capacity=%u %s %s %s %s %s %s -sendsize=%u %s %s %s %s %s -maxidle=%u %s %s %s %s %s %s -memwait=%u -memkill=%u %s %s"),
		(s.Dir.IsEmpty() || !FPaths::DirectoryExists(s.Dir)) ? TEXT("") : *FString::Printf(TEXT("-dir=\"%s\""), *s.Dir),
		OutPort, MaxCpu, s.Mulcpu, MaxCon, MaxCpu, s.Capacity,
		(s.Config.IsEmpty() || !FPaths::FileExists(s.Config)) ? TEXT("") : *FString::Printf(TEXT("-config=\"%s\""), *s.Config),
		s.bVerbose ? TEXT("-verbose") : TEXT(""),
		s.bLog ? TEXT("-log") : TEXT(""),
		s.bNoCustoMalloc ? TEXT("-nocustomalloc") : TEXT(""),
		s.bStoreRaw ? TEXT("-storeraw") : TEXT(""),
		s.bSendRaw ? TEXT("-sendraw") : TEXT(""),
		s.SendSize,
		s.Named.IsEmpty() ? TEXT("") : *FString::Printf(TEXT("-named=\"%s\""), *s.Named),
		s.bNoPoll ? TEXT("-nopoll") : TEXT(""),
		s.bNoStore ? TEXT("-nostore") :  TEXT(""),
		s.bResetStore ? TEXT("-resetstore") : TEXT(""),
		s.bQuiet ? TEXT("-quiet") : TEXT(""),
		s.MaxIdle,
		s.bBinaryVersion ? TEXT("-binasversion") : TEXT(""),
		s.bSummary ? TEXT("-summary") : TEXT(""),
		(s.EventFile.IsEmpty() || !FPaths::FileExists(s.EventFile)) ? TEXT("") : *FString::Printf(TEXT("-eventfile=\"%s\""), *s.EventFile),
		s.bSentry ? TEXT("-sentry") : TEXT(""),
		s.bNoProxy ? TEXT("-noproxy") : TEXT(""),
		s.bKillRandom ? TEXT("-killrandom") : TEXT(""),
		s.MemWait, s.MemKill, 
		s.Crypto.IsEmpty() ? TEXT("") : *FString::Printf(TEXT("-crypto=\"%s\""), *s.Crypto),
		s.PopulateCas.IsEmpty() ? TEXT("") : *FString::Printf(TEXT("-populateCas=\"%s\""), *s.PopulateCas)
	);
	SUbaAgentHandle = FPlatformProcess::CreateProc(
		*XiaoAgentExePath,
		*Params,
		false, false, false,
		nullptr, 0,
		*FPaths::GetPath(XiaoAgentExePath),
		SUbaAgentPipeWriteChild, SUbaAgentPipeReadChild
	);
	XIAO_LOG(Display, TEXT("FPlatformProcess::CreateProc AppPath::%s Params::%s"), *XiaoAgentExePath, *Params);

	return SUbaAgentHandle.IsValid();
}

void FAgentService::TryRunIPerfServer()
{
	const FString BinariesDir = FPaths::GetPath(FPlatformProcess::ExecutablePath());
	FString ConfigPath = FPaths::ConvertRelativePathToFull(BinariesDir, TEXT("../../Config/config.json"));
	FPaths::MakeStandardFilename(ConfigPath);
	if (!FPaths::FileExists(ConfigPath))
	{
		XIAO_LOG(Error, TEXT("Can\'t Get config::%s!"), *ConfigPath);
		return;
	}

	FString IPerfServerName = TEXT("iperf3");
#if PLATFORM_WINDOWS
	IPerfServerName += TEXT(".exe");	
#endif

	FString ServerExePath = FPaths::ConvertRelativePathToFull(FPaths::Combine(BinariesDir, IPerfServerName));
	FPaths::MakeStandardFilename(ServerExePath);
	if (!FPaths::FileExists(ServerExePath))
	{
		XIAO_LOG(Error, TEXT("server executable file not exist::%s!"), *ServerExePath);
		return;
	}

	SIPerfProcHandle = GetProcHandle(ServerExePath);
	if (SIPerfProcHandle.IsValid())
	{
		return;
	}

	SIPerfProcHandle = FPlatformProcess::CreateProc(
		*ServerExePath,
		*FString::Printf(TEXT("-s -p %d"), XiaoNetwork::SIPerfServicePort),
		false, false, false,
		nullptr, 0,
		*FPaths::GetPath(ServerExePath),
		nullptr
	);
}
