#include "CoordiService.h"
#include "XiaoShareNetwork.h"
#include "XiaoShareRedis.h"
#include "XiaoShare.h"
#include "XiaoLog.h"
#include "XiaoShareField.h"
#include "XiaoCompressor.h"
#include "XiaoAgent.h"
#include "system_settings.pb.h"
#include "agent.pb.h"
#include <tuple>
#include <unordered_map>
#include <algorithm>

using namespace XiaoNetwork;
using namespace XiaoRedis;

static constexpr double SOneHourSeconds = 3600.0f;
static constexpr double SOneDaySeconds = SOneHourSeconds * 24.0f;
static constexpr double SOneWeekSeconds = SOneDaySeconds * 7.0f;
static constexpr double SOneMonthSeconds = SOneWeekSeconds * 4.0f;
static double SCleanTraceSeconds = SOneWeekSeconds;
static double SCleanLoginSeconds = SOneDaySeconds;
static double SNetworkPerforSeconds = 600.0f;

static bool SMaster = false;
static std::tuple<std::string, FRedisServerDesc> SRedisMasterNode;
static std::map<std::string, FRedisServerDesc> SRedisSlaveNodes;
float FCoordiService::SSleepTime = 1.0f;
static FString SLocalIp;
static std::string SLocalAddress;
static FProcHandle SRedisProcHandle;
static void* SRedisPipeReadChild = nullptr;
static void* SRedisPipeWriteChild = nullptr;
static std::unordered_map<std::string, std::string> AgentStats;
static std::vector<std::string> EnableHelperIds;
static std::set<std::string> Initiators;
static std::unordered_map<std::string, FAgentProto> Id2Proto;
static std::map<std::string, FNetworkConnectivity> SAgentNetworkMap;


bool FCoordiService::OnInitialize(const FString& InParams)
{
	GNeedFlush = true;
	XIAO_LOG(Log, TEXT("OnInitialize::Begin!"));

	TSharedPtr<FJsonObject> ConfigObj = nullptr;
	if (!ReadConfig(ConfigObj))
	{
		XIAO_LOG(Error, TEXT("Can\'t Get config!"));
		return false;
	}

	GOnRedisChanged.Bind([](uint8 InStatus)
	{
		if (!SRedisMessage.empty())
		{
			XIAO_LOG(Error, TEXT("%s"), UTF8_TO_TCHAR(SRedisMessage.c_str()));
		}

		if (SRedisStatus == ERedisStatus::Redis_TimeoutError || SRedisStatus == ERedisStatus::Redis_IoError || SRedisStatus == ERedisStatus::Redis_CloseError)
		{
			TryBecomeMaster();
		}
	});

	if(!TryRunRedisServer())
	{
		return false;
	}
	
	SLocalIp = GetLANV4();
	SLocalAddress = TCHAR_TO_UTF8(*SLocalIp);

	// 等待一秒
	FPlatformProcess::Sleep(1.0f);

	// 加载代理文件
	LoadAgentSettings(SOriginalAgentSettings);

	// Redis客户端连接主服务器
	const auto& CoordiNet = SOriginalAgentSettings.NetworkCoordinate;
	const auto& NetworkCoordi = SOriginalAgentSettings.NetworkCoordinate;
	GMasterConnection.host = TCHAR_TO_UTF8(*NetworkCoordi.IP);
	GMasterConnection.port = NetworkCoordi.Port;
	if (!TryConnectRedis(false))
	{
		XIAO_LOG(Error, TEXT("Can\'t connect to cache server!"));
		return false;
	}

	XIAO_LOG(Log, TEXT("Cache Server is running!"));

	try
	{
		if (CoordiNet.IP == TEXT("127.0.0.1") || CoordiNet.IP == TEXT("localhost") || CoordiNet.IP == SLocalIp)
		{
			SMaster = true;
		}
		FRedisServerDesc NodeDesc(SLocalIp, SMaster ? CoordiNet.Port : CoordiNet.BackupPort, SMaster, 0, true);
		const std::string JsonContent = TCHAR_TO_UTF8(*NodeDesc.ToJson());
		SRedisClient->hset(Hash::SCacheList, GAgentUID, JsonContent);
		SRedisMasterNode = std::make_tuple(GAgentUID, NodeDesc);
		UpdateRedisCluster();

		XIAO_LOG(Log, TEXT("Current Redis node as \"%s\" node role!"), SMaster ? TEXT("Master") : TEXT("Slave"));

		if (SMaster)
		{
			if (!SRedisClient->exists(String::SAgentUpdateFreqency))
			{
				SRedisClient->set(String::SAgentUpdateFreqency, String::SAgentUpdateFreqStr);
				XIAO_LOG(Verbose, TEXT("Create string:: %hs"), String::SAgentUpdateFreqency.c_str());
			}
			if (!SRedisClient->exists(String::SSystemSettings))
			{
				SetDefaultParams(GModifySystemSettings);
				SRedisClient->set(String::SSystemSettings, GModifySystemSettings.SerializeAsString());
				XIAO_LOG(Verbose, TEXT("Create string::%hs"), String::SSystemSettings.c_str());
			}
			if (!SRedisClient->exists(Hash::SAgentStats))
			{
				SRedisClient->hset(Hash::SAgentStats, {});
				XIAO_LOG(Verbose, TEXT("Create hash::%hs"), Hash::SAgentStats.c_str());
			}

			if (!SRedisClient->exists(Hash::SLoginCache))
			{
				SRedisClient->hset(Hash::SLoginCache, {});
				XIAO_LOG(Verbose, TEXT("Create hash::%hs"), Hash::SLoginCache.c_str());
			}

			if (!SRedisClient->exists(Hash::SAgentHash))
			{
				SRedisClient->hset(Hash::SAgentHash, {});
				XIAO_LOG(Verbose, TEXT("Create hash::%hs"), Hash::SAgentHash.c_str());
			}
			if (!SRedisClient->exists(Hash::STraceLog))
			{
				SRedisClient->hset(Hash::STraceLog, {});
				XIAO_LOG(Verbose, TEXT("Create hash::%hs"), Hash::STraceLog.c_str());
			}

			if (!SRedisClient->exists(List::SEnbaleHelpers))
			{
				SRedisClient->lpush(List::SEnbaleHelpers, {});
				XIAO_LOG(Verbose, TEXT("Create list::%hs"), List::SEnbaleHelpers.c_str());
			}
			if (!SRedisClient->exists(Set::SBuildGroup))
			{
				SRedisClient->sadd(Set::SBuildGroup, { "Default" });
				XIAO_LOG(Verbose, TEXT("Create set::%hs"), Set::SBuildGroup.c_str());
			}

			if (!SRedisClient->exists(Hash::SBuildCount))
			{
				SRedisClient->hset(Hash::SBuildCount, {});
				XIAO_LOG(Verbose, TEXT("Create hash::%hs"), Hash::SBuildCount.c_str());
			}
			if (!SRedisClient->exists(Hash::SBuildState))
			{
				SRedisClient->hset(Hash::SBuildState, {});
				XIAO_LOG(Verbose, TEXT("Create hash::%hs"), Hash::SBuildState.c_str());
			}
			if (!SRedisClient->exists(Hash::SBuildStat))
			{
				SRedisClient->hset(Hash::SBuildStat, {});
				XIAO_LOG(Verbose, TEXT("Create hash::%hs"), Hash::SBuildStat.c_str());
			}
			if (!SRedisClient->exists(Hash::SExceptionDesc))
			{
				SRedisClient->hset(Hash::SExceptionDesc, {});
				XIAO_LOG(Verbose, TEXT("Create hash::%hs"), Hash::SExceptionDesc.c_str());
			}
			if (!SRedisClient->exists(Hash::SCrashDump))
			{
				SRedisClient->hset(Hash::SCrashDump, {});
				XIAO_LOG(Verbose, TEXT("Create hash::%hs"), Hash::SCrashDump.c_str());
			}
		}

		XIAO_LOG(Log, TEXT("OnInitialize::Finish!"));
		return true;
	}
	CATCH_REDIS_EXCEPTRION();
	return true;
}

void FCoordiService::OnDeinitialize()
{
	XIAO_LOG(Log, TEXT("OnDeinitialize::Begin!"));

	if (SRedisPipeReadChild != nullptr || SRedisPipeWriteChild != nullptr)
	{
		FPlatformProcess::ClosePipe(SRedisPipeReadChild, SRedisPipeWriteChild);
		SRedisPipeReadChild = nullptr;
		SRedisPipeWriteChild = nullptr;
	}

	if (SRedisProcHandle.IsValid())
	{
		FPlatformProcess::TerminateProc(SRedisProcHandle);
		FPlatformProcess::CloseProc(SRedisProcHandle);
	}

	XIAO_LOG(Log, TEXT("OnDeinitialize::Finish!"));
}

bool FCoordiService::TryRunServer(const FString& InServerName, const FString& InParams, FProcHandle& OutProcHandle, void*& ReadPipe, void*& WritePipe)
{
	const FString ServerExePath = FPaths::ConvertRelativePathToFull(InServerName);
	if (!FPaths::FileExists(ServerExePath))
	{
		XIAO_LOG(Error, TEXT("server executable file not exist::%s!"), *ServerExePath);
		return false;
	}

	FProcHandle ProcHandle = GetProcHandle(ServerExePath);
	if (ProcHandle.IsValid())
	{
		FPlatformProcess::CloseProc(ProcHandle);
		XIAO_LOG(Log, TEXT("Kill %s proc!"), *InServerName);
		FPlatformProcess::Sleep(0.5f);
	}

	if (!FPlatformProcess::CreatePipe(ReadPipe, WritePipe))
	{
		XIAO_LOG(Error, TEXT("CreatePipe Failed!"));
		return false;
	}

	OutProcHandle = FPlatformProcess::CreateProc(
		*ServerExePath,
		*InParams,
		false, false, false,
		nullptr, 0,
		*FPaths::GetPath(ServerExePath),
		WritePipe, ReadPipe
	);
	if (!OutProcHandle.IsValid())
	{
		XIAO_LOG(Error, TEXT("Create Proc failed!"));
		return false;
	}

	return true;
}

bool FCoordiService::TryRunRedisServer()
{
	if(IsAppRunning(XiaoAppName::SCacheServer))
	{
		const FString CacheServerPath = GetXiaoAppPath(XiaoAppName::SCacheServer);
		SRedisProcHandle = GetProcHandle(CacheServerPath);
		if (!SRedisProcHandle.IsValid())
		{
			XIAO_LOG(Error, TEXT("Can\'t Get Cache server proc handle::%s!"), *CacheServerPath);
			return false;
		}
		return true;
	}
	
	// 配置文件
	FString RedisConfigPath = FPaths::ConvertRelativePathToFull(FPaths::Combine(FPaths::GetPath(FPlatformProcess::ExecutablePath()), TEXT("../../Config/cache.conf")));
	FPaths::MakeStandardFilename(RedisConfigPath);
	if (!FPaths::FileExists(RedisConfigPath))
	{
		XIAO_LOG(Error, TEXT("Can\'t Get config::%s!"), *RedisConfigPath);
		return false;
	}

	const FString RedisServerName = XiaoAppName::SCacheServer
#if PLATFORM_WINDOWS
		+ TEXT(".exe")
#endif
	;

	// const FString Params = FString::Printf(TEXT("%s --bind 127.0.0.1 %s"), *RedisConfigPath, *GetLANV4());
	return TryRunServer(RedisServerName, RedisConfigPath, SRedisProcHandle, SRedisPipeReadChild, SRedisPipeWriteChild);
}

void FCoordiService::UpdateRedisCluster()
{
	try
	{
		std::unordered_map<std::string, std::string> Result;
		SRedisClient->hgetall(Hash::SCacheList, std::inserter(Result, Result.begin()));
		for (const auto& Iter : Result)
		{
			const FString JsonContent = UTF8_TO_TCHAR(Iter.second.c_str());
			FRedisServerDesc Node;
			if (Node.FromJson(JsonContent))
			{
				if (!Node.Role)
				{
					SRedisSlaveNodes.insert(std::make_tuple(Iter.first, Node));
				}
			}
		}

		if (SMaster)
		{
			// 更新Cluster状态
			for (auto& Iter : SRedisSlaveNodes)
			{
				auto& Node = Iter.second;
				FRedisServerDesc::CheckNodeAlive(Node);
				const std::string JsonContent = TCHAR_TO_UTF8(*Node.ToJson(false));
				if (!JsonContent.empty())
				{
					SRedisClient->hset(Hash::SCacheList, Iter.first, JsonContent);
				}
			}
		}
	}
	CATCH_REDIS_EXCEPTRION();
}

void FCoordiService::UpdateAgentStatus()
{
	if (SRedisClient)
	{
		AgentStats.clear();
		Id2Proto.clear();
		EnableHelperIds.clear();
		Initiators.clear();
		try
		{
			// 更新频率
			const auto UpdateOptional = SRedisClient->get(String::SAgentUpdateFreqency);
			if (UpdateOptional.has_value())
			{
				const std::string UpdateValueStr = UpdateOptional.value();
				if (UpdateValueStr.length() > 0)
				{
					SSleepTime = GSleepUpdate = std::stof(UpdateValueStr);
				}
			}

			const auto SettingsOptional = SRedisClient->get(String::SSystemSettings);
			if(SettingsOptional.has_value())
			{ 
				const std::string SystemSettingsStr = SettingsOptional.value();
				if (SystemSettingsStr.length() > 0)
				{
					if (!GModifySystemSettings.ParseFromString(SystemSettingsStr))
					{
						XIAO_LOG(Error, TEXT("SystemSettings ParseFromString failed!"));
					}
				}
			}

			// 获取所有的代理数据
			SRedisClient->hgetall(Hash::SAgentStats, std::inserter(AgentStats, AgentStats.begin()));
			for (const auto& Iter : AgentStats)
			{
				const std::string AgentId = Iter.first;
				if (AgentId.length() == 0)
				{
					XIAO_LOG(VeryVerbose, TEXT("Agent id is empty!"));
					continue;
				}

				FAgentProto Proto;
				if (!Proto.ParseFromString(Iter.second))
				{
					XIAO_LOG(Error, TEXT("ParseFromString failed!"));
					continue;
				}

				// 无效代理
				if (Proto.macaddress().empty() || Proto.routerip().empty() || AgentId != Proto.macaddress())
				{
					XIAO_LOG(Error, TEXT("Agent is not valid!"));
					SRedisClient->hdel(Hash::SAgentStats, AgentId);
					continue;
				}

				if (SLocalAddress != Proto.routerip())
				{
					if (!SAgentNetworkMap.contains(AgentId))
					{
						FNetworkConnectivity Desc;
						Desc.RemoteConnection = UTF8_TO_TCHAR(Proto.routerip().c_str());
						if (Desc.RemoteConnection != SLocalIp)
						{
							SAgentNetworkMap.insert(std::make_tuple(AgentId, Desc));
						}
					}
				}

				// 同步网络相关数据
				const EAgentStatus Status = static_cast<EAgentStatus>(Proto.status());
				if (SAgentNetworkMap.contains(AgentId))
				{
					auto& NetDesc = SAgentNetworkMap[AgentId];
					
					Proto.set_networkspeed(NetDesc.Performance);
					const std::string NetRoundStr = std::string("Sender ") + std::to_string(NetDesc.SendPerfor) + std::string(" Gbits/sec\n") +  
													std::string("Receiver ") + std::to_string(NetDesc.ReceivePerfor) + std::string(" Gbits/sec");
					Proto.set_updowntime(NetRoundStr);

					// 是否活跃
					NetDesc.LastUpdate = Proto.lastupdate();
					if (NetDesc.LastUpdate == NetDesc.LastActive)
					{
						++NetDesc.NotActiveCount;
					}
					else
					{
						NetDesc.LastActive = NetDesc.LastUpdate;
						NetDesc.NotActiveCount = 0;
					}

					if (Status == EAgentStatus::Status_Ready && NetDesc.NotActiveCount > 3)
					{
						Proto.set_status(EAgentStatus::Status_Offline);
					}
					else if (Status == EAgentStatus::Status_Offline && NetDesc.NotActiveCount == 0)
					{
						Proto.set_status(EAgentStatus::Status_Ready);
					}

					std::string RawStr;
					if (Proto.SerializePartialToString(&RawStr) && RawStr.size() > 0)
					{
						SRedisClient->hset(Hash::SAgentStats, AgentId, RawStr);
					}
				}

				if (Status == EAgentStatus::Status_Initiating)
				{
					const std::string Initiator = (Proto.desc().empty() ? Proto.loginuser() : Proto.desc());
					Initiators.insert(Initiator);
				}

				// HelperInfos
				// 初步筛选
				if (!Proto.benablehelper())
				{
					XIAO_LOG(Verbose, TEXT("Not enable as helper"));
					continue;
				}

				// 状态是否合适
				if (Status != EAgentStatus::Status_Ready && Status != EAgentStatus::Status_Helping)
				{
					XIAO_LOG(Verbose, TEXT("Status not satisfy::%d"), Status);
					continue;
				}

				// cpu筛选
				if (Proto.cpuava() < GModifySystemSettings.cpuavailableminimal())
				{
					XIAO_LOG(Verbose, TEXT("Cpu not satisfy"));
					continue;
				}

				// 硬盘空间
				const float AvaHardDiskSpace = Proto.totalhardspace() - Proto.usehardspace();
				const float NeedDiskSpace = GModifySystemSettings.harddiskminimal();
				if (AvaHardDiskSpace < NeedDiskSpace)
				{
					XIAO_LOG(Verbose, TEXT("Harddisk not satisfy::current::%f need::%f"), AvaHardDiskSpace, NeedDiskSpace);
					continue;
				}

				// 物理内存
				const float AvaHardMemory = Proto.totalmemory() - Proto.usememory();
				const float MinHardMemory = GModifySystemSettings.physicalmemory();
				if (AvaHardMemory < MinHardMemory)
				{
					XIAO_LOG(Verbose, TEXT("Physical memory not satisfy::current::%f need::%f"), AvaHardMemory, MinHardMemory);
					continue;
				}

				// 网络利用率
				//if ((Proto.avalnet()+1.0f) < GModifySystemSettings.networkavamin())
				//{
				//	XIAO_LOG(Verbose, TEXT("Current Network utilization over the system limit %.1f%% > %.1f%%"), Proto.avalnet(), GModifySystemSettings.networkavamin());
				//	continue;
				//}

				//// Gpu利用率
				//if ((Proto.avagpu() + 1.0f) < GModifySystemSettings.gpuavamin())
				//{
				//	XIAO_LOG(Verbose, TEXT("Current GPU utilization over the system limit %.1f%% > %.1f%%"), Proto.avagpu(), GModifySystemSettings.gpuavamin());
				//	continue;
				//}

				Id2Proto.insert_or_assign(AgentId, Proto);
				EnableHelperIds.push_back(AgentId);
			}

			// 然后进行排序
			std::sort(EnableHelperIds.begin(), EnableHelperIds.end(), [](const std::string& L, const std::string& R) 
			{
				const FAgentProto& LP = Id2Proto[L]; 
				const FAgentProto& RP = Id2Proto[R];

				// Ready 状态为最优先级
				if (LP.status() != RP.status())
				{
					return LP.status() == 0 && RP.status() != 0;
				}

				// 分配优先级
				if (LP.allocationpriority() != RP.allocationpriority())
				{
					return LP.allocationpriority() > RP.allocationpriority();
				}
				
				// 空闲cpu
				if (!FMath::IsNearlyEqual(LP.cpuava(), RP.cpuava(), 1.0f))
				{
					return LP.cpuava() > RP.cpuava();
				}

				// 空闲RAM
				const float AvlRAML = LP.totalmemory() - LP.usememory();
				const float AvlRAMR = RP.totalmemory() - RP.usememory();
				if (!FMath::IsNearlyEqual(AvlRAML, AvlRAMR, 1.0f))
				{
					return AvlRAML > AvlRAMR;
				}

				// 空闲硬盘空间
				const float AvlHardL = LP.totalhardspace() - LP.usehardspace();
				const float AvlHardR = RP.totalhardspace() - RP.usehardspace();
				if (!FMath::IsNearlyEqual(AvlHardL, AvlHardR, 1.0f))
				{
					return AvlHardL > AvlHardR;
				}

				// 是否固定为helper
				if (LP.bfixedhelper() != !RP.bfixedhelper())
				{
					return LP.bfixedhelper() && !RP.bfixedhelper();
				}

				// 协助核心数量
				if (LP.helpercore() != RP.helpercore())
				{
					return LP.helpercore() > RP.helpercore();
				}

				// 空闲网络
				if (!FMath::IsNearlyEqual(LP.avalnet(), RP.avalnet(), 1.0f))
				{
					return LP.avalnet() > RP.avalnet();
				}

				// 空闲GPU
				if (!FMath::IsNearlyEqual(LP.avagpu(), RP.avagpu(), 1.0f))
				{
					return LP.avagpu() > RP.avagpu();
				}

				// 物理核心数
				if (LP.physicalcore() != RP.physicalcore())
				{
					return LP.physicalcore() > RP.physicalcore();
				}

				// 网络速率
				if (!FMath::IsNearlyEqual(LP.networkspeed(), RP.networkspeed(), 1.0f))
				{
					return LP.networkspeed() > RP.networkspeed();
				}

				// Cpu架构
				const int ArchCompare = LP.cpuarch().compare(RP.cpuarch());
				if (ArchCompare > 0)
				{
					return true;
				}
				if (ArchCompare < 0)
				{
					return false;
				}

				// 操作系统
				const int IsCompare = LP.opsystem().compare(RP.opsystem());
				if (IsCompare > 0)
				{
					return true;
				}
				if (IsCompare < 0)
				{
					return false;
				}

				return true;
			});

			//更新可用代理列表
			if (const int HelperLen = SRedisClient->llen(List::SEnbaleHelpers); HelperLen > 0)
			{
				SRedisClient->ltrim(List::SEnbaleHelpers, HelperLen, 0);
			}
			if (EnableHelperIds.size() > 0)
			{
				SRedisClient->rpush(List::SEnbaleHelpers, EnableHelperIds.begin(), EnableHelperIds.end());
			}
		}
		CATCH_REDIS_EXCEPTRION();
	}
}

void FCoordiService::UpdateNetworkStatus()
{
	for (auto& AgentDesc : SAgentNetworkMap)
	{
		auto& Desc = AgentDesc.second;

		if (!AgentStats.contains(AgentDesc.first))
		{
			continue;
		}

		if ((FPlatformTime::Seconds() - Desc.LastActiveTestTime) > FNetworkConnectivity::SActive)
		{
			Desc.LastActiveTestTime = FPlatformTime::Seconds();
			if (Desc.bWorking && Desc.ProcHandle.IsValid())
			{
				FPlatformProcess::CloseProc(Desc.ProcHandle);
			}

			XIAO_LOG(Verbose, TEXT("Network test::Remote Agent ip::%s"), *Desc.RemoteConnection);
			Desc.OnNetworkTest();
		}
	}

	// 尝试重置状态
	std::set<std::string> NeedReset;
	for (const auto& Iter : Id2Proto)
	{
		const EAgentStatus Status = static_cast<EAgentStatus>(Iter.second.status());
		if (Status == EAgentStatus::Status_Helping)
		{
			const std::string HelpingMsg = Iter.second.message();
			bool bNeedReset = true;
			for (const std::string& Initiator : Initiators)
			{
				if (HelpingMsg.ends_with(Initiator))
				{
					bNeedReset = false;
					break;
				}
			}
			if (bNeedReset)
			{
				NeedReset.insert(Iter.first);
			}
		}
	}
	for (const std::string& Id : NeedReset)
	{
		if (AgentStats.contains(Id))
		{
			UpdateAgent(Id, 0, "");
		}
	}
}

static void CleanCache()
{
	try
	{
		// 清理Trace文件
		static double LastCleanTrace = 0.0f;
		const double PastTraceSeconds = FPlatformTime::Seconds() - LastCleanTrace;
		if (PastTraceSeconds > SCleanTraceSeconds)
		{
			LastCleanTrace = FPlatformTime::Seconds();
			XIAO_LOG(Log, TEXT("Clean Trace Cache."));
			std::vector<std::string> LogKeys;
			SRedisClient->hkeys(Hash::STraceLog, std::inserter(LogKeys, LogKeys.begin()));
			for (const std::string& Key : LogKeys)
			{
				const FString KeyStr = UTF8_TO_TCHAR(Key.c_str());
				TArray<FString> Parts;
				KeyStr.ParseIntoArray(Parts, TEXT("#"));
				if (Parts.Num() == 0)
				{
					continue;
				}

				// 写入时间
				const uint64 WriteCycles = FCString::Strtoui64(*Parts[0], nullptr, Parts[0].Len());
				const double PastSeconds = FPlatformTime::ToSeconds64(FPlatformTime::Cycles64() - WriteCycles);
				if (PastSeconds > SOneWeekSeconds)
				{
					SRedisClient->hdel(Hash::STraceLog, Key);
					XIAO_LOG(Log, TEXT("Delete the agent log with key::%s"), UTF8_TO_TCHAR(Key.c_str()));
				}
			}
		}
		
		// 清理登录缓存
		static double LastCleanLogin = 0.0f;
		const double PastLoginSeconds = FPlatformTime::Seconds() - LastCleanLogin;
		if (PastLoginSeconds > SCleanLoginSeconds)
		{
			LastCleanLogin = FPlatformTime::Seconds();

			std::vector<std::string> LoginKeys;
			SRedisClient->hkeys(Hash::SLoginCache, std::inserter(LoginKeys, LoginKeys.begin()));
			for (const std::string& Key : LoginKeys)
			{
				if (Key.empty())
				{
					continue;
				}

				const auto Option = SRedisClient->hget(Hash::SLoginCache, Key);
				if (!Option.has_value())
				{
					continue;
				}

				const FString Content = UTF8_TO_TCHAR(Option.value().c_str());
				FLoginCache Cache;
				if (Cache.FromJson(Content))
				{
					const double PastTime = FPlatformTime::Seconds() - Cache.LastLogin;
					if (PastTime > SCleanLoginSeconds)
					{
						SRedisClient->hdel(Hash::SLoginCache, Key);
						XIAO_LOG(Log, TEXT("Delete the LoginCache::%s."), UTF8_TO_TCHAR(Key.c_str()));
					}
				}
			}
		}
	}
	CATCH_REDIS_EXCEPTRION();
}

void FCoordiService::TryBecomeMaster()
{
	try
	{
		// 当前的从服务器是否是具有第一顶替优先级
		TArray<FRedisServerDesc> SlaveNodes;
		for (const auto& Iter : SRedisSlaveNodes)
		{
			const auto& Node = Iter.second;
			if (!Node.Role && Node.Status)
			{
				SlaveNodes.Add(Node);
			}
		}
		SlaveNodes.Sort([](const FRedisServerDesc& L, const FRedisServerDesc& R)
		{
			return L.Priority <= R.Priority ? true : false;
		});

		// 如果是顺位第一位则选举为下一任Master
		for (const auto& Node : SlaveNodes)
		{
			if (Node.Host == SLocalIp)
			{
				GMasterConnection.host = TCHAR_TO_UTF8(*Node.Host);
				GMasterConnection.port = Node.Port;
				if (TryConnectRedis(false))
				{
					// 将当前的从服务器升格为主服务器
					XIAO_LOG(Warning, TEXT("About to switch to master server"));
					SRedisClient->command({ "REPLICAOF", "NO", "ONE" }, true);

					// 将原有Master状态设置为false
					auto& MasterNode = std::get<1>(SRedisMasterNode);
					MasterNode.Status = false;
					const std::string JsonContent = TCHAR_TO_UTF8(*MasterNode.ToJson());
					if (!JsonContent.empty())
					{
						SRedisClient->hset(Hash::SCacheList, std::get<0>(SRedisMasterNode), JsonContent);
					}

					SRedisMasterNode = std::make_tuple(GAgentUID, SlaveNodes[0]);
					SMaster = true;
					XIAO_LOG(Warning, TEXT("New Master server::%s:%d."), *SLocalIp, SlaveNodes[0].Port);
					return;
				}
			}
		}
	}
	CATCH_REDIS_EXCEPTRION();
}

void FCoordiService::BecomeSlave()
{

}

void FCoordiService::OnTick()
{
	if (SRedisPipeReadChild)
	{
		const FString Msg = FPlatformProcess::ReadPipe(SRedisPipeReadChild);
		if (!Msg.IsEmpty())
		{
			XIAO_LOG(VeryVerbose, TEXT("%s"), *Msg);
		}
	}

	if (!SRedisProcHandle.IsValid() || !FPlatformProcess::IsProcRunning(SRedisProcHandle))
	{
		XIAO_LOG(Error, TEXT("cache-server not running!"));
		TryRunRedisServer();
		return;
	}

	if (SMaster)
	{
		// 更新Cluster状态
		UpdateRedisCluster();

		// 更新代理状态
		UpdateAgentStatus();

		// 更新代理网络
		UpdateNetworkStatus();

		// 清理缓存
		CleanCache();
	}
	else
	{
		// 检查主服务器
		auto& MasterNode = std::get<1>(SRedisMasterNode);
		FRedisServerDesc::CheckNodeAlive(MasterNode);
		if (!MasterNode.Status)
		{
			// 升级为主服务器
			TryBecomeMaster();
		}
	}
}
