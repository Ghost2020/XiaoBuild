/**
  * @author cxx2020@outlook.com
  * @datetime 2023 -9:32 PM
 */
#pragma once

#include "CoreMinimal.h"
#include "Containers/Queue.h"
#include "Containers/StringConv.h"
#include "Serialization/MemoryReader.h"
#include "UbaScheduler.h"
#include "UbaLogger.h"
#include "UbaSessionServer.h"
#include "UbaNetworkServer.h"
#include "UbaStorageServer.h"
#include "UbaNetworkClient.h"
#include "UbaCacheClient.h"
#include <mutex>
#include "agent.pb.h"
#include "build_progress.pb.h"


#ifdef Yield
#undef Yield
#endif

#if !UE_BUILD_SHIPPING
	#ifdef MemoryBarrier
	#undef MemoryBarrier
	#endif
#endif


#include "XiaoShareRedis.h"
#include "boost/interprocess/interprocess_fwd.hpp"

#include "Async/Future.h"



namespace boost::interprocess
{
	class shared_memory_object;
	class mapped_region;
	class named_sharable_mutex;
}
using namespace boost::interprocess;


namespace uba
{
	using FStringToUbaStringConversion = TStringConversion<TStringConvert<TCHAR, uba::tchar>>;
	using FUbaStringToStringConversion = TStringConversion<TStringConvert<uba::tchar, TCHAR>>;

#if PLATFORM_WINDOWS
	#define TCHAR_TO_UBASTRING(STR) STR
#else
	#define TCHAR_TO_UBASTRING(STR) FStringToUbaStringConversion(STR).Get()
#endif

	#define UBASTRING_TO_TCHAR(STR) FUbaStringToStringConversion(STR).Get()

	class NetworkServer;
	class NetworkBackend;
	class StorageServer;
	class FDynamicScheduler;
	struct BinaryReader;
	struct BinaryWriter;
	struct ConnectionInfo;

	struct FSchedulerInfo
	{
		FString ActionFilePath = TEXT("");
		uba::NetworkBackend* Backend = nullptr;
		const uint8* Crypto = nullptr;
		bool bImmediatelyExit = true;
		bool bDynamic = false;
		FString TraceName;
		uint32 PPID = 0;
		bool bExclusive = false;

		FSchedulerInfo()
		{
		}

		FSchedulerInfo(const FString& InActionFilePath, NetworkBackend* InBackend, const uint8* InCrypto, const bool InbDynamic, const FString& InTraceName, const uint32 InPPID, const bool InbExclusize)
			: ActionFilePath(InActionFilePath)
			, Backend(InBackend)
			, Crypto(InCrypto)
			, bDynamic(InbDynamic)
			, TraceName(InTraceName)
			, PPID(InPPID)
			, bExclusive(InbExclusize)
		{
		}

		FSchedulerInfo& operator=(const FSchedulerInfo& InAnotherInfo)
		{
			if (this != &InAnotherInfo)
			{
				ActionFilePath = InAnotherInfo.ActionFilePath;
				Backend = InAnotherInfo.Backend;
				Crypto = InAnotherInfo.Crypto;
				bDynamic = InAnotherInfo.bDynamic;
				TraceName = InAnotherInfo.TraceName;
				PPID = InAnotherInfo.PPID;
				bExclusive = InAnotherInfo.bExclusive;
			}
			return *this;
		}
	};

	struct FSchedulerContext
	{
		~FSchedulerContext();

		std::unique_ptr<uba::NetworkBackend> NetworkBackend = nullptr;
		std::unique_ptr<uba::StorageServer> StorageServer = nullptr;
		std::unique_ptr<uba::NetworkServer> NetworkServer = nullptr;
		std::unique_ptr<uba::SessionServer> SessionServer = nullptr;
		std::unique_ptr<uba::NetworkClient> NetworkClient = nullptr;
		std::unique_ptr<uba::CacheClient> CacheClient = nullptr;
		std::unique_ptr<uba::FilteredLogWriter> LoggerWriter = nullptr;
		std::unique_ptr<uba::LoggerWithWriter> Logger = nullptr;
		std::unique_ptr<FDynamicScheduler> Scheduler = nullptr;

		bool Run();

		void Reset();
	};

	struct FTaskResponse
	{
		uint32 ID = 0;
		int32 ReturnCode = 0;
	};

	class FDynamicScheduler : public uba::Scheduler
	{
	public:
		FDynamicScheduler(const SchedulerCreateInfo& InCreateinfo, FSchedulerInfo& InSchedualerInfo, LoggerWithWriter& LogWriter);
		~FDynamicScheduler();

		bool Init();
		bool Run();
		void Disconnect();

		static void PublishException(const ProcessHandle& InProcessHandle, const FString& InError);
		
	private:
		void RegistCallback();
		bool StaticInit();
		bool DynamicInit();

		bool EnqueueFromJson(const FString& InJsonFilePath);

		void RemoteActionFailedCrash(const ProcessHandle& InProcessHandle, const FString& InError);

		void TryConnectAgents();
		void ReleaseAgent(const uint32 InClientId, const uba::SessionServer::ClientSession* InSession = nullptr);
		void TryReleaseAgents();

		bool HandleTask(FMemoryReader& InReader);
		void HandleResponse(const ProcessHandle& InPh, const u32 InTaskId);

		static void FillKnownInputsBuffer(const TArray<FString>& InKnownInputs, TArray<uba::tchar>& OutKnownInputsBuffer, uint32& OutKnownInputCount, const bool bContainEndSymbol);

		bool CanBeInitiator() const;
		void UpdateAgents(const int InLocalStatus, const int InAgentStatus, const bool bImmediate);
		void UpdateProgress(const int InStatus, const float InProgress, const bool bImmediate = false);
		static void UpdateBuildStats();

		SessionServer::ClientSession* GetClientSession(const std::string& InAgentId);

	private:
		FSchedulerInfo Info;

		Event Event;

		TUniquePtr<message_queue> InputMq = nullptr;
		TFuture<void> ReadBackThreadFuture;
		TUniquePtr<message_queue> OutputMq = nullptr;
		TFuture<void> WriteOutThreadFuture;

		TQueue<FTaskResponse, EQueueMode::Spsc> PendingOutput;

		bool bDisconnected = false;

		uint32 MaxCoreAvailable = 0;
		uint32 MaxProcessorCount = 0;
		uint32 MaxAgentConCount = 0;

		LoggerWithWriter& logger;
		SessionServer& session;

		std::mutex Mutex;

		TArray<uba::tchar> KnownInputsBuffer;
		uint32 KnownInputCount = 0;

		TSet<FString> HostsFailed;

		ConnectionInfo InitiatorInfo;
		MessageInfo MessageInfo;

		TMap<FString, std::string> Ip2Id;
		TMap<uint32, std::string> CId2Id;
		std::map<std::string, uint32> Id2CId;
		TSet<uint32> AreadyDisconnecedSet;
		std::set<std::string> AreadyConnectedSet;
		std::set<std::string> CantConnectSet;
		std::string NewAgentId;

		std::unordered_map<ProcessHandle, bool> LocalRetryProcess;
		TUniquePtr<shared_memory_object> ProgressShm = nullptr;
		TUniquePtr<mapped_region> ProgressRegion = nullptr;

		FAgentProto InitiatorProto;
		FBuildProgress BuildProgress;

		bool bContainObserver = false;

		static FBuildStats GBuildStats;
		static FCriticalSection* GCriticalSection;
	};
}