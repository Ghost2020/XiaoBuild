/**
  * @author cxx2020@outlook.com
  * @datetime 2023 -12:18 AM
 */
#pragma once

#include <string>
#include <memory>
#include <format>
#include <iostream>
#include <set>
#include <functional>
#include <thread>
#ifdef PLATFORM_MAC
#include <future>
#endif
#include "XiaoRedis.h"
#include "system_settings.pb.h"
#include "agent.pb.h"
#include "XiaoShareNetwork.h"
#include "Serialization/JsonSerializerMacros.h"

using namespace Xiao;

enum ERedisStatus
{
	Redis_Ok = 0,
	Redis_Error,
	Redis_IoError,
	Redis_TimeoutError,
	Redis_CloseError,
	Redis_ProtoError,
	Redis_OomError,
	Redis_ReplyError,
	Redis_WatchError,
	Redis_MovedError,
	Redis_AskError,

	Redis_UndefinedError = 255,
};

class FRedisEvent 
{
public:
	void Bind(const std::function<void(uint8)>& listener)
	{
		Listeners.push_back(listener);
	}

	void Trigger(const uint8 InStatus)
	{
		if(IsBind())
		{
			for (const auto& listener : Listeners)
			{
				listener(InStatus);
			}
		}
	}

	bool IsBind() const
	{
		return !Listeners.empty();
	}

	void Clear()
	{
		Listeners.clear();
	}

private:
	std::vector<std::function<void(uint8)>> Listeners;
};
inline FRedisEvent GOnRedisChanged;

#define JSON_MCI_VALUE(var) JSON_SERIALIZE(#var, var)

struct FBuildStats : FJsonSerializable
{
	FString BuildId = FGuid::NewGuid().ToString();
	FString AgentId;
	double Start = FPlatformTime::Seconds();
	double End = 0.0f;
	bool BuildStatus = true;
	uint32 ErrorNum = 0;
	uint32 WarningNum = 0;
	uint32 TotalJobNum = 0;
	uint32 RemainJobNum = 0;

	explicit FBuildStats()
	{
	}

	BEGIN_JSON_SERIALIZER
		JSON_MCI_VALUE(BuildId);
		JSON_MCI_VALUE(AgentId);
		JSON_MCI_VALUE(Start);
		JSON_MCI_VALUE(End);
		JSON_MCI_VALUE(BuildStatus);
		JSON_MCI_VALUE(ErrorNum);
		JSON_MCI_VALUE(WarningNum);
		JSON_MCI_VALUE(TotalJobNum);
	END_JSON_SERIALIZER
};

struct FRedisServerDesc : FJsonSerializable
{
	FString Host = TEXT("127.0.0.1");
	uint32 Port = XiaoNetwork::SCoordiServicePort;
	// true::Master false::Slave
	bool Role = true;
	// 当主服务不在线时，从服务器升级为主服务器的优先级
	uint32 Priority = 0;
	// false 表示未启动, true运行中
	bool Status = false;

	explicit FRedisServerDesc()
	{}

	explicit FRedisServerDesc(const FString& InHost, const uint32 InPort, const bool InRole, const uint32 InPriority, const bool InStatus)
		: Host(InHost)
		, Port(InPort)
		, Role(InRole)
		, Priority(InPriority)
		, Status(InStatus)
	{}

	virtual ~FRedisServerDesc() {};

	FRedisServerDesc& operator=(const FRedisServerDesc& InAnother)
	{
		if(this != &InAnother)
		{
			this->Host = InAnother.Host;
			this->Port = InAnother.Port;
			this->Role = InAnother.Role;
			this->Priority = InAnother.Priority;
			this->Status = InAnother.Status;
		}
		return *this;
	}

	static void CheckNodeAlive(FRedisServerDesc& InOutNode)
	{
		ConnectionOptions Options;
		Options.host = TCHAR_TO_UTF8(*InOutNode.Host);
		Options.port = InOutNode.Port;
		Options.keep_alive = false;
		InOutNode.Status = false;
		try
		{
			Redis Client(Options);
			InOutNode.Status = (Client.ping() == "PONG");
		}
		catch (std::exception& E)
		{}
	}

	BEGIN_JSON_SERIALIZER
		JSON_MCI_VALUE(Host);
		JSON_MCI_VALUE(Port);
		JSON_MCI_VALUE(Role);
		JSON_MCI_VALUE(Priority);
		JSON_MCI_VALUE(Status);
	END_JSON_SERIALIZER
};

struct FRedisCluster : FJsonSerializable
{
private:
	TArray<FString> _Array;
public:
	virtual ~FRedisCluster() {};

	TArray<FRedisServerDesc> Nodes;

	void Reset()
	{
		_Array.Reset();
		Nodes.Reset();
	}
	
	BEGIN_JSON_SERIALIZER
		// #FIXME 下面的方式会导致GetObject方法未定义，与Windows的API有冲突
		// JSON_SERIALIZE_ARRAY_SERIALIZABLE("Nodes", Nodes, FRedisServerDesc);
		if (Serializer.IsSaving())
		{
			_Array.Reset();
			for (const auto& Node : Nodes)
			{
				_Array.Add(Node.ToJson());
			}
			JSON_SERIALIZE_ARRAY("nodes", _Array);
		}
		else if (Serializer.IsLoading())
		{
			JSON_SERIALIZE_ARRAY("nodes", _Array);
			Nodes.Reset();
			for (const auto& Content : _Array)
			{
				FRedisServerDesc Desc;
				if (Desc.FromJson(Content))
				{
					Nodes.Add(Desc);
				}
			}
		}
	END_JSON_SERIALIZER
};

struct FLoginCache : FJsonSerializable
{
	FString Id = TEXT("");
	double LastLogin = 0.0f;
	FString AuthData = TEXT("");

	explicit FLoginCache()
	{}

	explicit FLoginCache(const FString& InId, const double InLastLogin, const FString& InAuthData)
		: Id(InId)
		, LastLogin(InLastLogin)
		, AuthData(InAuthData)
	{}

	BEGIN_JSON_SERIALIZER
		JSON_MCI_VALUE(Id);
		JSON_MCI_VALUE(LastLogin);
		JSON_MCI_VALUE(AuthData);
	END_JSON_SERIALIZER
};

struct FRedisEventDesc : FJsonSerializable
{
	FString Channel;
	FString Message;
	FText Desc;

	explicit FRedisEventDesc(const FString& InChannel, const FString& InMessage, const FText& Desc)
		: Channel(InChannel)
		, Message(InMessage)
		, Desc(Desc)
	{}

	BEGIN_JSON_SERIALIZER
		JSON_MCI_VALUE(Channel);
		JSON_MCI_VALUE(Message);
		JSON_MCI_VALUE(Desc);
	END_JSON_SERIALIZER
};
#undef JSON_MCI_VALUE

inline FSystemSettings GOriginalSystemSettings;
inline FSystemSettings GModifySystemSettings;
inline Xiao::ConnectionOptions GMasterConnection;
inline const FString SRedisClusterFile = FPaths::ConvertRelativePathToFull(FString::Printf(TEXT("%s/XiaoBuild/%s.json"), *FPaths::GetPath(FPlatformProcess::ApplicationSettingsDir()), TEXT("CacheCluster")));
inline FRedisCluster GCluster;
inline float GSleepUpdate = 5.0f;
inline ERedisStatus SRedisStatus = ERedisStatus::Redis_Ok;
inline std::string SRedisMessage;

#define CATCH_REDIS_EXCEPTRION() \
    catch (const Xiao::ClosedError& Ex) \
    { \
		SRedisMessage = std::string("ClosedError::") + Ex.what(); \
    	GSleepUpdate = 5.0f; \
    	SRedisStatus = ERedisStatus::Redis_CloseError; \
		GOnRedisChanged.Trigger(static_cast<uint8>(SRedisStatus)); \
    } \
    catch (const Xiao::TimeoutError& Ex) \
    { \
    	SRedisMessage = std::string("TimeoutError::") + Ex.what(); \
    	GSleepUpdate = 3.0f; \
    	SRedisStatus = ERedisStatus::Redis_TimeoutError; \
		GOnRedisChanged.Trigger(static_cast<uint8>(SRedisStatus)); \
    } \
	catch (const Xiao::IoError& Ex) \
    { \
		SRedisMessage = std::string("IoError::") + Ex.what(); \
    	GSleepUpdate = 1.0f; \
    	SRedisStatus = ERedisStatus::Redis_IoError; \
		GOnRedisChanged.Trigger(static_cast<uint8>(SRedisStatus)); \
    } \
    catch(const Xiao::ProtoError& Ex) \
    { \
    	SRedisMessage = std::string("ProtoError::") + Ex.what(); \
		GSleepUpdate = 2.0f; \
    	SRedisStatus = ERedisStatus::Redis_ProtoError; \
		GOnRedisChanged.Trigger(static_cast<uint8>(SRedisStatus)); \
    } \
	catch(const Xiao::Error& Ex) \
    { \
    	SRedisMessage = std::string("Error::") + Ex.what(); \
		GSleepUpdate = 2.0f; \
    	SRedisStatus = ERedisStatus::Redis_UndefinedError; \
		GOnRedisChanged.Trigger(static_cast<uint8>(SRedisStatus)); \
    } \
	catch(const std::exception& Ex) \
	{ \
		SRedisMessage = std::string("std::exception::") + Ex.what(); \
    	GSleepUpdate = 5.0f; \
    	SRedisStatus = ERedisStatus::Redis_UndefinedError; \
		GOnRedisChanged.Trigger(static_cast<uint8>(SRedisStatus)); \
	}


namespace XiaoRedis
{
	namespace Key
	{
		static const std::string SPong("PONG");
	}

	namespace String
	{
		// 代理更新频率Key
		static const std::string SAgentUpdateFreqency("agent-update-frequency");
		// 代理信息更新频率Key
		static const std::string SAgentUpdateFreqStr("5.0f");
		// 系统设置
		static const std::string SSystemSettings("system-settings");
		// 系统同步
		static const std::string SSystemSync("sync_data");
	}
	
	namespace Hash
	{
		// 缓存服务器列表
		static const std::string SCacheList("cache-list");
		// 用于缓存代理的相关数据信息
		static const std::string SAgentStats("agent-stats");
		// 构建次数
		static const std::string SBuildCount("build-count");
		// 构建状态
		static const std::string SBuildState("build-state");
		// 构建统计
		static const std::string SBuildStat("build-stat");
		// 登录缓存
		static const std::string SLoginCache("login-cache");
		// 日志文件
		static const std::string SAgentLog("agent-logs");
		// Trace文件
		static const std::string STraceLog("trace-logs");
		// 代理hash
		static const std::string SAgentHash("agent-hash");
		// 异常描述
		static const std::string SExceptionDesc("exception");
		// 崩溃dump文件
		static const std::string SCrashDump("crash-dump");
		// 协助描述
		static const std::string SHelper2Initiaotr("helper-Initiator");
		// 用户描述
		static const std::string SUserDetail("user-details");
	}

	namespace List
	{
		// 可供协助的代理列表
		static const std::string SEnbaleHelpers("agent-helper-ids");
	}

	namespace Set
	{
		// 构建群组
		static const std::string SBuildGroup("build-group");
	}

	namespace Channel
	{
		static const std::string SProcessException("exception");
	}
	
	inline std::shared_ptr<Xiao::Redis> SRedisClient = nullptr;

	static void SetDefaultParams(FSystemSettings& OutSettings)
	{
		OutSettings.set_harddiskminimal(5.0f);
		OutSettings.set_virtualmemoryminimal(1.0f);
		OutSettings.set_physicalmemory(1.0f);
		OutSettings.set_cpuavailableminimal(40.0f);
		OutSettings.set_diskavamin(50.0f);
		OutSettings.set_benablehelper(true);
		OutSettings.set_helpercoreavailablepercentminimal(30.0f);
		OutSettings.set_networkavamin(10.0f);
		OutSettings.set_gpuavamin(10.0f);

		OutSettings.set_maxinitiatornum(10);
		OutSettings.set_maxcorenum(500);
		OutSettings.set_maxconnum(20);

		OutSettings.set_agentserviceport(XiaoNetwork::SAgentServicePort);
		OutSettings.set_webuiport(XiaoNetwork::SUIServicePort);
		OutSettings.set_coordiserviceport(XiaoNetwork::SCoordiServicePort);
		OutSettings.set_licenseserviceport(XiaoNetwork::SLicenseServicePort);
		OutSettings.set_cacheserviceport(XiaoNetwork::SCacheServicePort);

		OutSettings.set_syncfreq(1.0f);
		OutSettings.set_bencypttransport(true);
		OutSettings.set_bforbidenstanby(true);
		OutSettings.set_bshowwindowsfire(false);
		OutSettings.set_bhelperenhance(true);

		OutSettings.set_bscheduleclean(true);
		OutSettings.set_scheduletime(3);

		OutSettings.set_bignorearch(true);
	}

	static bool IsEqual(const FSystemSettings& L, const FSystemSettings& R)
	{
		return !FMath::IsNearlyEqual(L.harddiskminimal(), R.harddiskminimal(), 0.01f)
		|| !FMath::IsNearlyEqual(L.virtualmemoryminimal(), R.virtualmemoryminimal(), 0.01f)
		|| !FMath::IsNearlyEqual(L.physicalmemory(), R.physicalmemory(), 0.01f)
		|| !FMath::IsNearlyEqual(L.cpuavailableminimal(), R.cpuavailableminimal(), 0.1f)
		|| !FMath::IsNearlyEqual(L.diskavamin(), R.diskavamin(), 0.1f)
		|| L.benablehelper() != R.benablehelper()
		|| !FMath::IsNearlyEqual(L.helpercoreavailablepercentminimal(), R.helpercoreavailablepercentminimal(), 0.1f)
		|| !FMath::IsNearlyEqual(L.networkavamin(), R.networkavamin(), 0.1f)
		|| !FMath::IsNearlyEqual(L.gpuavamin(), R.gpuavamin(), 0.1f)

		|| L.maxinitiatornum() != R.maxinitiatornum()
		|| L.maxcorenum() != R.maxcorenum()
		|| L.maxconnum() != R.maxconnum()

		|| L.agentserviceport() != R.agentserviceport()
		|| L.webuiport() != R.webuiport()
		|| L.coordiserviceport() != R.coordiserviceport()
		|| L.licenseserviceport() != R.licenseserviceport()
		|| L.cacheserviceport() != R.cacheserviceport()

		|| L.syncfreq() != R.syncfreq()
		|| L.bencypttransport() != R.bencypttransport()
		|| L.bforbidenstanby() != R.bforbidenstanby()
		|| L.bshowwindowsfire() != R.bshowwindowsfire()
		|| L.bhelperenhance() != R.bhelperenhance()

		|| L.bscheduleclean() != R.bscheduleclean()
		|| L.scheduletime() != R.scheduletime()
		|| L.bignorearch() != R.bignorearch();
	}

	static void Assign(const FSystemSettings& L, FSystemSettings& Out)
	{
		Out.set_agentserviceport(L.agentserviceport());
		Out.set_webuiport(L.webuiport());
		Out.set_coordiserviceport(L.coordiserviceport());
		Out.set_licenseserviceport(L.licenseserviceport());
		Out.set_cacheserviceport(L.cacheserviceport());

		Out.set_harddiskminimal(L.harddiskminimal());
		Out.set_virtualmemoryminimal(L.virtualmemoryminimal());
		Out.set_physicalmemory(L.physicalmemory());
		Out.set_cpuavailableminimal(L.cpuavailableminimal());
		Out.set_diskavamin(L.diskavamin());
		Out.set_networkavamin(L.networkavamin());
		Out.set_gpuavamin(L.gpuavamin());

		Out.set_syncfreq(L.syncfreq());

		Out.set_maxconnum(L.maxconnum());
		Out.set_maxcorenum(L.maxcorenum());
		Out.set_maxinitiatornum(L.maxinitiatornum());

		Out.set_helpercoreavailablepercentminimal(L.helpercoreavailablepercentminimal());
		Out.set_benablehelper(L.benablehelper());
		Out.set_bencypttransport(L.bencypttransport());
		Out.set_bforbidenstanby(L.bforbidenstanby());
		Out.set_bhelperenhance(L.bhelperenhance());

		Out.set_bscheduleclean(L.bscheduleclean());
		Out.set_scheduletime(L.scheduletime());

		Out.set_bignorearch(L.bignorearch());
	}

	static bool IsConnected()
	{
		return SRedisStatus == ERedisStatus::Redis_Ok && SRedisClient;
	}

	static void PublishException(const std::string& InException, const std::string& InGuid)
	{
		if (IsConnected())
		{
			try
			{ 
				SRedisClient->hset(Hash::SExceptionDesc, InGuid, InException);
				SRedisClient->publish(Channel::SProcessException, InException);
			}
			CATCH_REDIS_EXCEPTRION()
		}
	}

	static void TryDisconnectRedis()
	{
		if (IsConnected())
		{
			SRedisClient.reset();
		}
		SRedisStatus = ERedisStatus::Redis_Error;
	}

	static bool _ConnectRedis(const ConnectionOptions& InOptions)
	{
		try
		{
			TryDisconnectRedis();
			SRedisClient = std::make_shared<Xiao::Redis>(InOptions);
			if (SRedisClient->ping() == Key::SPong)
			{
				SRedisStatus = ERedisStatus::Redis_Ok;
				GOnRedisChanged.Trigger(SRedisStatus);
				return true;
			}
			else
			{
				GSleepUpdate = 5.0f;
				SRedisStatus = ERedisStatus::Redis_TimeoutError;
			}
		}
		CATCH_REDIS_EXCEPTRION();
		if (!GOnRedisChanged.IsBind())
		{
			XIAO_LOG(Error, TEXT("%hs"), SRedisMessage.c_str());
		}
		TryDisconnectRedis();
		return false;
	}

	static bool UpdateAgent(const std::string& InAgentId, const int InStatus, const std::string& InMessage, const int32 InTracePort=0)
	{
		if (!IsConnected())
		{
			return false;
		}

		try
		{
			if (const auto AgentOption = SRedisClient->hget(Hash::SAgentStats, InAgentId))
			{
				const std::string Value = AgentOption.value();
				if (FAgentProto Proto; Value.size() > 0 && Proto.ParseFromString(Value))
				{
					Proto.set_status(InStatus);
					Proto.set_message(InMessage);
					if (InTracePort > 0)
					{
						Proto.set_traceport(InTracePort);
					}
					std::string Buffer;
					if (Proto.SerializeToString(&Buffer))
					{
						SRedisClient->hset(Hash::SAgentStats, InAgentId, Buffer);
					}
					return true;
				}
			}
		}
		CATCH_REDIS_EXCEPTRION()
		return false;
	}

	static bool _ConnectClusterRedis()
	{
		// 获取Cluster
		GCluster.Reset();
		FString JsonContent;
		if (FFileHelper::LoadFileToString(JsonContent, *SRedisClusterFile))
		{
			GCluster.FromJson(JsonContent);
		}

		// 没有备用服务器
		if (GCluster.Nodes.Num() <= 1)
		{
			return false;
		}
		// 然后尝试备用服务器
		for (int Index = 1; Index < GCluster.Nodes.Num(); ++Index)
		{
			const auto& Node = GCluster.Nodes[Index];
			if (Node.Status)
			{
				ConnectionOptions Options;
				Options.host = TCHAR_TO_UTF8(*Node.Host);
				Options.port = Node.Port;
				Options.keep_alive = true;
				if (_ConnectRedis(Options))
				{
					return true;
				}
			}
		}
		return false;
	}

	static bool TryConnectRedis(const bool bInTry = true)
	{
#if PLATFORM_MAC
		GMasterConnection.connect_timeout = std::chrono::milliseconds(5000);
		GMasterConnection.socket_timeout = std::chrono::milliseconds(5000);
#endif
		// 首先尝试主服务器
		if (_ConnectRedis(GMasterConnection))
		{
			return true;
		}

		return bInTry ? _ConnectClusterRedis() : false;
	}

	static void AsyncReconnectRedis()
	{
		static double LastTry = 0.0;
		static bool bWorking = false;
		if (bWorking || (FPlatformTime::Seconds() - LastTry) < GSleepUpdate)
		{
			return;
		}

		bWorking = true;
		LastTry = FPlatformTime::Seconds();
#if PLATFORM_MAC
		std::async(std::launch::async,
#else
		AsyncThread(
#endif
			[&]()
			{
				TryConnectRedis(true);
				bWorking = false;
			}
		);
	}
};
