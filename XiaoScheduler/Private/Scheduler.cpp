/**
  * @author cxx2020@outlook.com
  * @datetime 2023 -9:32 PM
 */
#include "Scheduler.h"
#include "Runtime/Launch/Resources/Version.h"
#include "Misc/Paths.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Misc/CommandLine.h"
#include "UbaEvent.h"
#include "UbaProcessHandle.h"
#include "UbaNetworkServer.h"
#include "UbaStorageServer.h"
#include "UbaSessionServer.h"
#include "UbaNetwork.h"
#include "UbaProcess.h"
#include "UbaNetworkBackendTcp.h"
#include "UbaVersion.h"
#include "UbaSessionServerCreateInfo.h"
#include "XiaoInterprocess.h"
#include "XiaoAgent.h"
#include "XiaoShareNetwork.h"
#include "XiaoShareRedis.h"
#include "exception.pb.h"
#include "Misc/Paths.h"
#include <memory>


using namespace XiaoRedis;


namespace uba
{
	static constexpr int32 UBAExitCodeStart = 9000;
	static constexpr double SUpdateTime = 3.0f;
	static constexpr u8 STaskTypeExit = 63;

	static uint32 SFinishProcess = 0;
	static uint32 SActiveRemote = 0;
	static FSchedulerInfo GInfo;
	static FString GLANV4;
	static FString GMappedAddress;
	static std::string GLocalMachineDesc;
	static std::string GLocalIpStr;
	FBuildStats FDynamicScheduler::GBuildStats;
	FCriticalSection* FDynamicScheduler::GCriticalSection = nullptr;

	const tchar* Version = GetVersionString();

	const tchar* DefaultTraceDir = []()
		{
			static tchar buf[256];
			if (IsWindows)
				ExpandEnvironmentStringsW(TC("%ProgramData%\\XiaoBuild\\Traces"), buf, sizeof(buf));
			else
				GetFullPathNameW(TC("~/XiaoBuild/Traces"), sizeof_array(buf), buf, nullptr);
			return buf;
		}();

	static FString TraceFile;

	static FORCEINLINE FString String2FString(const std::string& InStdStr)
	{
		return FString(UTF8_TO_TCHAR(InStdStr.c_str()));
	}

	static FORCEINLINE bool TryGetAddressAndPort(const FAgentProto& InProto, FString& OutAddress, uint16& OutPort)
	{
		const FString PortMappedAddress = String2FString(InProto.portmappedaddress());
		if (!PortMappedAddress.IsEmpty())
		{
			FString PortStr;
			if (PortMappedAddress.Split(TEXT(":"), &OutAddress, &PortStr))
			{
				OutPort = static_cast<uint16>(FCString::Atoi(*PortStr));
				return true;
			}
		}
		return false;
	}

	static void CleanHistory(const FString& InTargetFolder)
	{
		if (FPaths::DirectoryExists(InTargetFolder))
		{
			IFileManager& FileManager = IFileManager::Get();

			TArray<FString> OutFolders;
			FileManager.FindFiles(OutFolders, *(InTargetFolder / TEXT("*.*")), false, true);
			for (const auto& Folder : OutFolders)
			{
				const FString HistoryFolder = FPaths::Combine(InTargetFolder, Folder);
				const FFileStatData FolderStat = FileManager.GetStatData(*HistoryFolder);
				if (FolderStat.bIsValid && FolderStat.bIsDirectory)
				{
					const double OverSeconds = (FDateTime::Now() - FolderStat.CreationTime).GetTotalSeconds();
					static constexpr double OneWeekSeconds = 3600.0f * 24.0f * 7.0f;
					if (OverSeconds > OneWeekSeconds)
					{
						FileManager.DeleteDirectory(*HistoryFolder, true, true);
					}
				}
			}
		}
	}

	static int PrintHelp(const tchar* message, LoggerWithWriter& logger)
	{
		if (*message)
		{
			logger.Error(TC("%s"), message);
		}

		return -1;
	}

	static bool ParseCommands(const FString& InCmdLine, LoggerWithWriter& logger, FSchedulerInfo& InInfo)
	{
		auto& UbaScheduler = SOriginalAgentSettings.UbaScheduler;

		if (FParse::Value(*InCmdLine, TEXT("-actionFile="), InInfo.ActionFilePath))
		{
			InInfo.bDynamic = false;
		}
		FParse::Value(*InCmdLine, TEXT("-traceName="), InInfo.TraceName);
		FParse::Value(*InCmdLine, TEXT("-capacity="), UbaScheduler.Capacity);
		FParse::Value(*InCmdLine, TEXT("-maxcpu="), UbaScheduler.MaxCpu);

		// 保证每一个实例的存储目录都不一样！
		UbaScheduler.Dir = FPaths::Combine(UbaScheduler.Dir, FString::Printf(TEXT("%u"), FPlatformProcess::GetCurrentProcessId()));

		FParse::Value(*InCmdLine, TEXT("-port="), UbaScheduler.Port);

		if (FParse::Param(*InCmdLine, TEXT("dynamic")))
		{
			InInfo.bDynamic = true;
			FString WorkDir;
			if (FParse::Value(*InCmdLine, TEXT("-workdir="), WorkDir))
			{
				UbaScheduler.Dir = WorkDir;
			}

			FParse::Value(*InCmdLine, TEXT("-ppid="), InInfo.PPID);
		}
		if (!FPaths::DirectoryExists(UbaScheduler.Dir))
		{
			IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
			if (!PlatformFile.CreateDirectoryTree(*UbaScheduler.Dir))
			{
				logger.Error(TC("CreateDirectoryTree %s failed!"), *UbaScheduler.Dir);
			}
		}
		if (FParse::Param(*InCmdLine, TEXT("visualizer")))
		{
			UbaScheduler.bVisualizer = true;
		}

		if (FParse::Param(*InCmdLine, TEXT("quiet")))
		{
			UbaScheduler.bQuiet = true;
		}
		if (FParse::Param(*InCmdLine, TEXT("nocustomalloc")))
		{
			UbaScheduler.bNoCustoMalloc = true;
		}
		if (FParse::Param(*InCmdLine, TEXT("nostdout")))
		{
			UbaScheduler.bEnableStdOut = false;
		}
		if (FParse::Param(*InCmdLine, TEXT("getcas")))
		{
			UbaScheduler.bGetCas = true;
		}
		if (FParse::Param(*InCmdLine, TEXT("summary")))
		{
			UbaScheduler.bSummary = true;
		}
		if (FParse::Param(*InCmdLine, TEXT("storeraw")))
		{
			UbaScheduler.bStoreRaw = true;
		}
		if (FParse::Param(*InCmdLine, TEXT("?")) || FParse::Param(*InCmdLine, TEXT("help")) || FParse::Param(*InCmdLine, TEXT("h")))
		{
			return PrintHelp(TC(""), logger) == 0;
		}

		return true;
	}

	FSchedulerContext::~FSchedulerContext()
	{
		Reset();
	}

	bool FSchedulerContext::Run()
	{
		uba::SetCustomAssertHandler([](const uba::tchar* text)
			{
				ProcessHandle Handle;
				FDynamicScheduler::PublishException(Handle, text);
				checkf(false, TEXT("%s"), text);
			});

		LoadAgentSettings(SOriginalAgentSettings);
		auto& UbaScheduler = SOriginalAgentSettings.UbaScheduler;
		LoggerWriter = std::make_unique<FilteredLogWriter>(g_consoleLogWriter, UbaScheduler.bQuiet ? LogEntryType_Info : LogEntryType_Detail);
		Logger = std::make_unique<LoggerWithWriter>(*LoggerWriter, TC(""));
		if (!ParseCommands(FCommandLine::Get(), *Logger, GInfo))
		{
			return false;
		}

		GMasterConnection.host = TCHAR_TO_UTF8(*SOriginalAgentSettings.NetworkCoordinate.IP);
		GMasterConnection.port = SOriginalAgentSettings.NetworkCoordinate.Port;
		GMasterConnection.keep_alive = true;
		if (!XiaoRedis::TryConnectRedis())
		{
			if (!SRedisMessage.empty())
			{
				Logger->Error(TC("%s"), UTF8_TO_TCHAR(SRedisMessage.c_str()));
			}
		}

		CleanHistory(UbaScheduler.Dir);

		const tchar* dbgStr = TC("");
		Logger->Info(TC("XiaoScheduler v%s%s (RootDir: \"%s\", StoreCapacity: %uGb)\n"), Version, dbgStr, *UbaScheduler.Dir, UbaScheduler.Capacity);

		const u64 storageCapacity = u64(UbaScheduler.Capacity) * 1000 * 1000 * 1000;

		NetworkBackend = std::make_unique<uba::NetworkBackendTcp>(*LoggerWriter);
		NetworkServerCreateInfo nsci(*LoggerWriter);
		bool ctorSuccess = true;
		NetworkServer = std::make_unique<uba::NetworkServer>(ctorSuccess, nsci);
		if (!ctorSuccess)
		{
			return false;
		}

		StorageServerCreateInfo storageInfo(*NetworkServer, TCHAR_TO_UBASTRING(*UbaScheduler.Dir), *LoggerWriter);
		storageInfo.casCapacityBytes = storageCapacity;
		storageInfo.storeCompressed = !UbaScheduler.bStoreRaw;
		StorageServer = std::make_unique<uba::StorageServer>(storageInfo);

		if (!FPaths::DirectoryExists(DefaultTraceDir))
		{
			IFileManager::Get().MakeDirectory(UBASTRING_TO_TCHAR(DefaultTraceDir), true);
		}
		TraceFile = FPaths::Combine(DefaultTraceDir, FString::Printf(TEXT("%s.uba"), *FDateTime::Now().ToString()));

		SessionServerCreateInfo ServerInfo(*StorageServer, *NetworkServer);
		ServerInfo.useUniqueId = true;
		ServerInfo.traceEnabled = true;
		ServerInfo.traceOutputFile = TCHAR_TO_UBASTRING(*TraceFile);
		ServerInfo.launchVisualizer = false;
		ServerInfo.allowMemoryMaps = UbaScheduler.bAllowMemoryMaps;
		ServerInfo.disableCustomAllocator = UbaScheduler.bNoCustoMalloc;
		ServerInfo.rootDir = TCHAR_TO_UBASTRING(*UbaScheduler.Dir);
		ServerInfo.traceName = TCHAR_TO_UBASTRING(*GInfo.TraceName);
		ServerInfo.remoteLogEnabled = true;
		ServerInfo.deleteSessionsOlderThanSeconds = 1;
		ServerInfo.logToFile = UbaScheduler.bLog;
#if (ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 6)
		// ServerInfo.allowLocalDetour = false;
		ServerInfo.readIntermediateFilesCompressed = true;
#endif
		SessionServer = std::make_unique<uba::SessionServer>(ServerInfo);

		if (!StorageServer->LoadCasTable(true))
		{
			Logger->Error(TC("Load Cas Table failed!"));
			return false;
		}

		uint16 OutPort = static_cast<uint16>(UbaScheduler.Port);
		const bool bRtn = XiaoNetwork::GetUsablePort(static_cast<uint16>(UbaScheduler.Port), OutPort);
		if (bRtn && OutPort != UbaScheduler.Port)
		{
			UbaScheduler.Port = OutPort;
			Logger->Warning(TC("Warning::use new listen port::%u"), OutPort);
		}

		SchedulerCreateInfo CreateInfo(*SessionServer);
		CreateInfo.maxLocalProcessors = UbaScheduler.MaxLocalCore;
		GInfo.Backend = &(*NetworkBackend);

		Scheduler = std::make_unique<uba::FDynamicScheduler>(CreateInfo, GInfo, *Logger);
		return Scheduler->Init() && Scheduler->Run();
	}

	void FSchedulerContext::Reset()
	{
		if (Scheduler)
		{
			Scheduler->Disconnect();
		}
		if (Scheduler)
		{
			auto& Session = Scheduler->GetSession();
#if (ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 6)
			Session.SaveSnapshotOfTrace(true);
#else
			Session.StopTrace(TCHAR_TO_UBASTRING(*TraceFile));
#endif
		}
		if (StorageServer)
		{
			if (StorageServer->SaveCasTable(true))
			{
				if (Logger)
				{
					Logger->Info(TC("CAS table saved..."));
				}
			}
		}

		Scheduler = nullptr;
		SessionServer = nullptr;
		StorageServer = nullptr;
		NetworkServer->DisconnectClients();
		NetworkServer = nullptr;
		NetworkBackend = nullptr;
		Logger = nullptr;
		LoggerWriter = nullptr;
	}

	FDynamicScheduler::FDynamicScheduler(const SchedulerCreateInfo& InCreateinfo, FSchedulerInfo& InSchedualerInfo, LoggerWithWriter& LogWriter)
		: Scheduler(InCreateinfo)
		, Info(InSchedualerInfo)
		, logger(LogWriter)
		, session(InCreateinfo.session)
		, MessageInfo()
	{
		XiaoIPC::Permissions.set_unrestricted();
		GLANV4 = XiaoNetwork::GetLANV4();
		GLocalIpStr = TCHAR_TO_UTF8(*GLANV4);
		GBuildStats.AgentId = UTF8_TO_TCHAR(GAgentUID.c_str());
		FDynamicScheduler::GCriticalSection = new FCriticalSection();
	}

	FDynamicScheduler::~FDynamicScheduler()
	{
		Disconnect();

		if (GCriticalSection)
		{
			delete GCriticalSection;
		}

		if (Info.bDynamic)
		{
			Info.PPID = 0;
			if (ReadBackThreadFuture.IsValid())
			{
				ReadBackThreadFuture.Wait();
				ReadBackThreadFuture.Reset();
			}

			if (WriteOutThreadFuture.IsValid())
			{
				WriteOutThreadFuture.Wait();
				WriteOutThreadFuture.Reset();
			}
		}
	}

	bool FDynamicScheduler::Init()
	{
		RegistCallback();

		if (!Info.ActionFilePath.IsEmpty() && FPaths::FileExists(Info.ActionFilePath))
		{
			if (!StaticInit())
			{
				return false;
			}
		}
		else
		{
			if (!DynamicInit())
			{
				return false;
			}
		}

		auto& Server = session.GetServer();
		Server.UnregisterService(uba::SessionServiceId);
		Server.RegisterService(
			uba::SessionServiceId,
			[this](const ConnectionInfo& connectionInfo, 
#if (ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 6)
				const WorkContext& workContext,
#endif
				uba::MessageInfo& messageInfo, uba::BinaryReader& reader, uba::BinaryWriter& writer)
			{
				if (messageInfo.type == STaskTypeExit)
				{
					Event.Set();
					Disconnect();
					return true;
				}

				return session.HandleMessage(connectionInfo, 
#if (ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 6)
					workContext, messageInfo,
#else
					messageInfo.type,
#endif
					reader, writer);

			},
			[](u8 type)
			{
				switch (type)
				{
#if (ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 6)
					#define UBA_SESSION_MESSAGE(x) case SessionMessageType_##x: return AsView(TC("")#x);
					UBA_SESSION_MESSAGES
					#undef UBA_SESSION_MESSAGE
				default:
					return ToView(TC("Unknown"));
#else
					#define UBA_SESSION_MESSAGE(x) case SessionMessageType_##x: return TC("")#x;
					UBA_SESSION_MESSAGES
					#undef UBA_SESSION_MESSAGE
					case STaskTypeExit: return TC("Exit");

					default: return TC("Unknown");
#endif
				}
			});

		const auto& UbaScheduler = SOriginalAgentSettings.UbaScheduler;

		// 查询是否已经有同样的Scheduler存在
		if (!IsAppRunning(XiaoAppName::SXiaoScheduler, FPlatformProcess::GetCurrentProcessId()))
		{
			try
			{
				// 本地共享对象，用于同步进度信息
				ProgressShm = MakeUnique<shared_memory_object>(open_or_create, XiaoIPC::SMonitorProgressMemoryName.c_str(), read_write, XiaoIPC::Permissions);
				if (ProgressShm)
				{
					ProgressShm->truncate(XiaoIPC::SMonitorProgressMemorySize);
					ProgressRegion = MakeUnique<mapped_region>(*ProgressShm, read_write);
					FMemory::Memset(ProgressRegion->get_address(), 0, ProgressRegion->get_size());
					if (!ProgressRegion)
					{
						logger.Warning(TC("Cant Create Progress Map Region!"));
					}
				}
				else
				{
					logger.Warning(TC("Cant OpenOrCreate IPC Progress Memory!"));
				}
			}
			catch (interprocess_exception& Ex)
			{
				logger.Warning(TC("Interprocess Object Create Exception::%s!"), Ex.what());
				return false;
			}

			// 更新进度信息
			
			const FString SchedulerMessage = TEXT("Initiating ") + Info.TraceName;
			if (!UpdateAgent(GAgentUID, EAgentStatus::Status_Initiating, TCHAR_TO_UTF8(*SchedulerMessage), UbaScheduler.Port))
			{
				return true;
			}
		}

		if (UbaScheduler.bStandalone)
		{
			logger.Info(TC("Running in Standalone mode!"));
			return true;
		}

		uint32 CurInitiatorNum;
		UpdateSystemSettings(CurInitiatorNum);
		if (CurInitiatorNum == 0)
		{
			logger.Warning(TC("Can't be as initiator, current system is full!"));
			return true;
		}

		// 获取本机约束条件
		MaxProcessorCount = UbaScheduler.MaxCpu;
		MaxAgentConCount = UbaScheduler.MaxCon;
		std::string Stats;
		try
		{
			const auto StatsVal = XiaoRedis::SRedisClient->hget(XiaoRedis::Hash::SAgentStats, GAgentUID);
			if (StatsVal.has_value())
			{
				Stats = StatsVal.value();
			}
		}
		CATCH_REDIS_EXCEPTRION();
		if (Stats.empty())
		{
			return true;
		}

		if (Stats.size() > 0 && InitiatorProto.ParseFromString(Stats))
		{
			if (InitiatorProto.maxcpu() >= 0)
			{
				MaxProcessorCount = InitiatorProto.maxcpu();
			}
			if (InitiatorProto.maxcon() >= 0)
			{
				MaxAgentConCount = InitiatorProto.maxcon();
			}

			uint16 OutPort;
			TryGetAddressAndPort(InitiatorProto, GMappedAddress, OutPort);
		}

		if (!InitiatorProto.benableinitator())
		{
			logger.Info(TC("Not Allow to be as initiator, Running in Standalone mode!"));
			return true;
		}

		if (!session.GetServer().StartListen(*Info.Backend, static_cast<uint16>(UbaScheduler.Port)))
		{
			logger.Error(TC("Start listen port [%u] failed!"), UbaScheduler.Port);
			return false;
		}

		// 设置本机最大运行核心
		const uint32 MaxLocalCpu = InitiatorProto.localmaxcpu();
		if (MaxLocalCpu > 0)
		{
			SetMaxLocalProcessors(MaxLocalCpu);
			MaxCoreAvailable += MaxLocalCpu;
		}

		GBuildStats.RemainJobNum = GetTotal();
		TryConnectAgents();
		return true;
	}

	bool FDynamicScheduler::Run()
	{
#if PLATFORM_WINDOWS
		constexpr int32 HighPriority = REALTIME_PRIORITY_CLASS;
		SetCurrentProcessPriority(HighPriority);
#endif
		logger.Info(TC("---------------------------------------XiaoBuild Start---------------------------------------"));
		logger.Info(TC("Build ID: {%s}"), *GBuildStats.BuildId);

		Event.Create(true);

		const u64 start = GetTime();
		Start();

		const auto& UbaScheduler = SOriginalAgentSettings.UbaScheduler;
		if (UbaScheduler.bVisualizer)
		{
			RunXiaoApp(XiaoAppName::SBuildApp, *FString::Printf(TEXT("-app=%s -host=\"%s\" -port=%u"), *XiaoAppName::SBuildMonitor, *GLANV4, UbaScheduler.Port));
		}

		if (!Event.IsSet())
			return false;
		const u64 time = GetTime() - start;

		bDisconnected = true;

		if (UbaScheduler.bSummary)
		{
			logger.BeginScope();
			auto& Session = GetSession();
			Session.PrintSummary(logger);
			Session.GetStorage().PrintSummary(logger);
			Session.GetServer().PrintSummary(logger);
			KernelStats::GetGlobal().Print(logger, true);
			logger.EndScope();
		}

		if (GBuildStats.RemainJobNum > 0)
		{
			GBuildStats.BuildStatus = false;
		}
		
		SetProcessFinishedCallback([](const ProcessHandle& InPh) {});
		logger.Info(TC("Scheduler run took %s"), TimeToText(time).str);
		logger.Info(TC("---------------------------------------XiaoBuild Finish with (%d)---------------------------------------"), GBuildStats.BuildStatus ? 0 : -1);
		GBuildStats.End = FPlatformTime::Seconds();
		UpdateBuildStats();
		
		return GBuildStats.BuildStatus;
	}

	void FDynamicScheduler::Disconnect()
	{
		if (!bDisconnected)
		{
			UpdateAgents(EAgentStatus::Status_Ready, EAgentStatus::Status_Ready, true);
			UpdateProgress(0, 0.0f, true);

			bDisconnected = true;
		}
	}

	void FDynamicScheduler::RegistCallback()
	{
		auto& UbaScheduler = SOriginalAgentSettings.UbaScheduler;
		auto& Server = session.GetServer();
		if (UbaScheduler.Crypto.Len() == 32)
		{
			const std::string Crypto = TCHAR_TO_UTF8(*UbaScheduler.Crypto);
			if (!Server.RegisterCryptoKey((uint8*)Crypto.c_str()))
			{
				logger.Error(TC("RegisterCryptoKey failed::%s"), *UbaScheduler.Crypto);
			}
		}
		Server.UnregisterOnClientConnected(uba::SessionServiceId);
		Server.RegisterOnClientConnected(
			uba::SessionServiceId,
			[this](const Guid& clientUid, u32 clientId)
			{
				CId2Id.Emplace(clientId, NewAgentId);
			}
		);
		Server.UnregisterOnClientDisconnected(uba::SessionServiceId);
		Server.RegisterOnClientDisconnected(
			uba::SessionServiceId,
			[this](const Guid& InClientUid, u32 InClientId)
			{
				session.OnDisconnected(InClientUid, InClientId);

				/*if (Info.bDynamic)
				{
					if (InitiatorInfo.internalData && InitiatorInfo.GetUid() == InClientUid && InitiatorInfo.GetId() == InClientId)
					{
						logger.Info(TC("UbacController clientId \"%d\" disconnected!"), InClientId);
						Event.Set();
					}
				}*/
				
				if (CId2Id.Contains(InClientId))
				{
					UpdateAgent(CId2Id[InClientId], EAgentStatus::Status_Ready, "");
				}
			}
		);

		SetProcessFinishedCallback([&](const ProcessHandle& InPh)
		{
			std::scoped_lock<std::mutex> Lock(Mutex);

			const auto& si = InPh.GetStartInfo();
			const uint32 ExitCode = InPh.GetExitCode();
			if (ExitCode != 0)
			{
				if (ExitCode == ProcessCancelExitCode)
				{
					return;
				}

				// 远程运行的任务尝试在本地运行
				if (InPh.IsRemote() && !LocalRetryProcess.contains(InPh))
				{
					// 记录失败
					const FString ExeHost = InPh.GetExecutingHost();
					HostsFailed.Add(ExeHost);

					// 断开对应的代理连接
					if (Ip2Id.Contains(ExeHost))
					{
						const std::string AgentId = Ip2Id[ExeHost];
						if (Id2CId.contains(AgentId))
						{
							ReleaseAgent(Id2CId[AgentId]);
						}
					}

					const auto& Logs = InPh.GetLogLines();
					if (Logs.size() == 0)
					{
						RemoteActionFailedCrash(InPh, TEXT(""));
						return;
					}
					if (ExitCode == 0xC0000005)
					{
						RemoteActionFailedCrash(InPh, TEXT("Access violation"));
						return;
					}
					if (ExitCode == 0xC0000409)
					{
						RemoteActionFailedCrash(InPh, TEXT("Stack buffer overflow"));
						return;
					}
					if (ExitCode == 0xC0000602)
					{
						RemoteActionFailedCrash(InPh, TEXT("Fail Fast Exception"));
						return;
					}
					if (ExitCode > UBAExitCodeStart && ExitCode < 10000)
					{
						RemoteActionFailedCrash(InPh, TEXT("UBA error"));
						return;
					}
					if (std::any_of(Logs.begin(), Logs.end(), [](const ProcessLogLine& InLogLine) { return InLogLine.text.find(TC(" C1001: ")) > 0; }))
					{
						RemoteActionFailedCrash(InPh, TEXT("C1001"));
						return;
					}
					// # TODO 需要尝试本地再次运行
					/*else if (SOriginalAgentSettings.UbaScheduler.Loop > 1)
					{
						RemoteActionFailedCrash(InPh, TEXT("Force local retry"));
						return;
					}*/
				}
				
				logger.Error(TC("Error %s with code:%u [Application::%s\nArguments::%s]"), si.description, ExitCode, si.application, si.arguments);

				GBuildStats.BuildStatus = false;
				++GBuildStats.ErrorNum;
			}
			// #FIXME 验证outputs文件的大小是否有效

			static Atomic<uint32> counter = 0;
			uint32 queued, activeLocal, activeRemote, outFinished;
			queued = 0; activeLocal = 0; activeRemote = 0; outFinished = 0;
			GetStats(queued, activeLocal, activeRemote, outFinished);

			const u32 c = ++counter;
			GBuildStats.TotalJobNum = GetTotal();
			SActiveRemote = activeRemote;

			UpdateProgress(1, float(c) / float(GBuildStats.TotalJobNum));
			UpdateAgents(EAgentStatus::Status_Initiating, EAgentStatus::Status_Helping, false);

			logger.BeginScope();
			StringBuffer<128> extra;
			if (InPh.IsRemote())
				extra.Append(TC(" [RemoteExecutor: ")).Append(InPh.GetExecutingHost()).Append(']');
			else if (InPh.GetExecutionType() == ProcessExecutionType_Native)
				extra.Append(TC(" (Not detoured)"));
			else if (InPh.GetExecutionType() == ProcessExecutionType_FromCache)
				extra.Append(TC(" (From cache)"));
			verify(si.description != nullptr);
			verify(extra.data != nullptr);
			logger.Info(TC("[%u/%u] %s%s"), c, GBuildStats.TotalJobNum, si.description, extra.data);
			for (const auto& Line : InPh.GetLogLines())
			{
				if (!Line.text.empty())
				{
					const auto Text = Line.text.c_str();
					if (Line.text != si.description && (!StartsWith(Text, TC("   Creating library")) && !StartsWith(Text, TC("Note:"))))
					{
						if (StartsWith(Text, TC("warning")))
						{
							++GBuildStats.WarningNum;
						}
						logger.Log(Line.type, Text, static_cast<u32>(Line.text.size()));
					}
				}
			}
			logger.EndScope();

			SFinishProcess = c;

			// 尝试释放计算资源
			GBuildStats.RemainJobNum = GBuildStats.TotalJobNum - c;
			if (InitiatorProto.benableinitator())
			{
				TryReleaseAgents();

				// 尝试获取计算资源(连接数没有达到限制且队列中的任务是当前所拥有的核心数的4倍)
				if (MaxAgentConCount > AreadyConnectedSet.size() && queued > MaxCoreAvailable)
				{
					TryConnectAgents();
				}
			}
			
			// 非动态任务是否已经完成
			if (!Info.bDynamic)
			{
				if (c == GBuildStats.TotalJobNum || (queued + activeLocal + activeRemote) == 0)
				{
					Event.Set();
				}
			}

#if (ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 6)
			static double LastTime = 0.0f;
			if(FPlatformTime::Seconds() - LastTime > 3.0f)
			{
				LastTime = FPlatformTime::Seconds();
				// Save current snapshot of UBA trace in case this failure crashes
				GetSession().SaveSnapshotOfTrace();
			}
#endif

			if (!GBuildStats.BuildStatus && Info.bImmediatelyExit)
			{
				Event.Set();
			}
		});

		session.RegisterGetNextProcess([this](Process& ProcessHandle, NextProcessInfo& OutNextProcess, u32 PrevExitCode) 
		{
			/*if (Info.bDynamic)
			{
				static double LastCheck = FPlatformTime::Seconds();
				static uint32 queued, activeLocal, activeRemote, outFinished;
				queued = 0; activeLocal = 0; activeRemote = 0; outFinished = 0;
				GetStats(queued, activeLocal, activeRemote, outFinished);
				if ((activeLocal + activeRemote + outFinished) == 0)
				{
					if (FPlatformTime::Seconds() - LastCheck > 10.f)
					{
						logger.Warning(TC("Aready not has task over 10.0 seconds, ready to exit the proc!"));
						Event.Set();
						return false;
					}
				}
				else
				{
					LastCheck = FPlatformTime::Seconds();
				}
			}*/

			// 请求过滤
			if (ProcessHandle.IsRemote() && HostsFailed.Contains(ProcessHandle.GetExecutingHost()))
			{
				logger.Warning(TC("Warning::Ignore current GetNextProcess!"));
				return false;	
			}

			return HandleReuseMessage(ProcessHandle, OutNextProcess, PrevExitCode);
		});

		// # TODO
		/*
		SessionServer& SessionServer = GetSession();
		SessionServer.SetRemoteProcessSlotAvailableEvent([]()
		{

		});
		SessionServer.SetRemoteProcessReturnedEvent([](Process& Process) 
		{

		});*/

		GOnRedisChanged.Bind([this](uint8 InStatus)
		{
			if (!SRedisMessage.empty())
			{
				logger.Error(TC("Redis Exception::%s"), *String2FString(SRedisMessage));
			}
			if (InStatus != ERedisStatus::Redis_ReplyError && InStatus != ERedisStatus::Redis_TimeoutError)
			{
				UpdateAgent(GAgentUID, EAgentStatus::Status_UnCondi, SRedisMessage);
			}
		});
	}

	bool FDynamicScheduler::StaticInit()
	{
		if (Info.ActionFilePath.EndsWith(TEXT(".xml")))
		{
			if (!EnqueueFromFile(TCHAR_TO_UBASTRING(*Info.ActionFilePath)))
			{
				return false;
			}
		}
		else if(Info.ActionFilePath.EndsWith(TEXT(".json")))
		{
			if (!EnqueueFromJson(Info.ActionFilePath))
			{
				return false;
			}
		}
		else
		{
			return false;
		}

		Info.bDynamic = false;	

		return true;
	}

	bool FDynamicScheduler::DynamicInit()
	{
		if (Info.PPID == 0)
		{
			logger.Info(TC("Command line should contain -ppid for Parent Process Id"));
			return false;
		}

		static FProcHandle ParentHandle = FPlatformProcess::OpenProcess(Info.PPID);
		if (!ParentHandle.IsValid())
		{
			logger.Info(TC("Error::Can\'t open the Parent Process with process id::%u"), Info.PPID);
			return false;
		}

		permissions QueuePermission;
		QueuePermission.set_default();
		static const std::string InputQueueName = XiaoIPC::SInputQueueName + "-" + std::to_string(Info.PPID);
		try
		{
			InputMq = MakeUnique<message_queue>(open_only, InputQueueName.c_str());
		}
		catch (interprocess_exception& Ex)
		{ 
			logger.Info(TC("Error:: Create Input Message queue \"%s\" failed::\"%s\""), UTF8_TO_TCHAR(InputQueueName.c_str()), UTF8_TO_TCHAR(Ex.what()));
			return false;
		}
		logger.Info(TC("InputMq Created"));
		ReadBackThreadFuture = AsyncThread([this]()
		{
			logger.Info(TC("ReadBackThread Begin"));

			try
			{
				static constexpr uint32 BufferSize = 8192 * 16;
				TArray<uint8> Buffer;
				std::size_t ReceiveSize = 0;
				uint32 Priority = 0;

				while (FPlatformProcess::IsProcRunning(ParentHandle))
				{
					ReceiveSize = 0;
					Buffer.Reset();
					Buffer.SetNumZeroed(BufferSize);
					const boost::posix_time::ptime AbsTime = boost::posix_time::microsec_clock::universal_time() + boost::posix_time::milliseconds(10);
					if (InputMq->timed_receive(Buffer.GetData(), BufferSize, ReceiveSize, Priority, AbsTime))
					{
						Buffer.SetNum(ReceiveSize);
						FMemoryReader Reader(Buffer);
						HandleTask(Reader);
					}
					else
					{
						FPlatformProcess::Sleep(1.0f);
					}
				}

				message_queue::remove(InputQueueName.c_str());						
			}
			catch (interprocess_exception& Ex)
			{
				logger.Info(TC("Error::Message queue \"%s\" Exception::\"%s\""), UTF8_TO_TCHAR(InputQueueName.c_str()), UTF8_TO_TCHAR(Ex.what()));
			}

			logger.Info(TC("ReadBackThread Finish"));

			Event.Set();
			Disconnect();
		});

		static const std::string OutputQueueName = XiaoIPC::SOutputQueueName + "-" + std::to_string(Info.PPID);
		try
		{
			OutputMq = MakeUnique<message_queue>(open_only, OutputQueueName.c_str());
		}
		catch (interprocess_exception& Ex)
		{
			logger.Info(TC("Error::Create Output Message queue \"%s\" failed::\"%s\""), UTF8_TO_TCHAR(OutputQueueName.c_str()), UTF8_TO_TCHAR(Ex.what()));
			return false;
		}
		logger.Info(TC("OutputMq Created"));
		WriteOutThreadFuture = AsyncThread([this]()
		{
			logger.Info(TC("WriteOutThread Begin"));

			static constexpr uint32 OutputMessageSize = sizeof(FTaskResponse);

			try
			{
				while (FPlatformProcess::IsProcRunning(ParentHandle))
				{
					FTaskResponse TaskResponse;
					if (PendingOutput.Dequeue(TaskResponse))
					{
						OutputMq->try_send(&TaskResponse, OutputMessageSize, 0);
					}
					else if(PendingOutput.IsEmpty())
					{
						FPlatformProcess::Sleep(1.0f);
					}
				}
			}
			catch (interprocess_exception& Ex)
			{
				logger.Info(TC("Error::Message queue \"%s\" Exception::\"%s\""), UTF8_TO_TCHAR(OutputQueueName.c_str()), UTF8_TO_TCHAR(Ex.what()));
			}

			logger.Info(TC("WriteOutThread Finish"));
			Event.Set();
		});

		return true;
	}

	bool FDynamicScheduler::EnqueueFromJson(const FString& InFilePath)
	{
		FString JsonStr;
		if (!FFileHelper::LoadFileToString(JsonStr, *InFilePath))
		{
			logger.Error(TC("Can\t load \"%s\" file to string!"), *InFilePath);
			return false;
		}

		TSharedPtr<FJsonObject> JsonObject;
		TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonStr);
		if (!FJsonSerializer::Deserialize(Reader, JsonObject) || !JsonObject.IsValid())
		{
			logger.Error(TC("Can\'t Deserialize \"%s\" file to JsonObject!"), *InFilePath);
			return false;
		}

		const FString EnviField(TEXT("environment"));
		FString Environment;
		if (!JsonObject->TryGetStringField(EnviField, Environment))
		{
			logger.Error(TC("Not contain \"%s\" field!"), *EnviField);
			return false;
		}
#if PLATFORM_WINDOWS
		SetEnvironmentVariable(TC("PATH"), *Environment);
#endif

		struct FDepDesc : FJsonSerializable
		{
			TArray<FString> KnownInputs;
			TArray<uba::tchar> KnownInputsBuffer;
			uint32 KnownInputCount = 0;

			BEGIN_JSON_SERIALIZER
				JSON_SERIALIZE_ARRAY("known_inputs", KnownInputs);
				FillKnownInputsBuffer(KnownInputs, KnownInputsBuffer, KnownInputCount, false);
			END_JSON_SERIALIZER
		};
		const FString DepField(TEXT("dep_map"));
		const TSharedPtr<FJsonObject>* DepMapObject;
		if (!JsonObject->TryGetObjectField(DepField, DepMapObject) || !DepMapObject)
		{
			logger.Error(TC("Not contain \"%s\" field!"), *DepField);
			return false;
		}
		TMap<FString, FDepDesc> App2Dep;
		for (const auto& Iter : (*DepMapObject)->Values)
		{
			if (!Iter.Value.IsValid() || Iter.Value->Type != EJson::Object)
			{
				continue;
			}

			const TSharedPtr<FJsonObject>& DepObject = Iter.Value->AsObject();
			check(DepObject.IsValid());
			FDepDesc DepDesc;
			if (DepDesc.FromJson(DepObject))
			{
				App2Dep.Add(MakeTuple(Iter.Key, std::move(DepDesc)));
			}
		}

		const FString TasksField(TEXT("tasks"));
		const TArray<TSharedPtr<FJsonValue>>* Tasks = nullptr;
		if (!JsonObject->TryGetArrayField(TasksField, Tasks) || !Tasks)
		{
			logger.Error(TC("Not contain \"%s\" field!"), *TasksField);
			return false;
		}

		struct FTaskDesc : FJsonSerializable
		{
			int32 Id = 0;
			FString App, Arg, Dir, Desc;
			TArray<FString> KnownInputs, OutputFiles;
			bool bRemote = true;
			bool bDetour = true;
			float Weight = 1.0;
			TArray<int32> Deps;

			TArray<uba::tchar> KnownInputsBuffer;
			uint32 KnownInputCount = 0;

			BEGIN_JSON_SERIALIZER
				JSON_SERIALIZE("id", Id);
				JSON_SERIALIZE("app", App);
				JSON_SERIALIZE("arg", Arg);
				JSON_SERIALIZE("dir", Dir);
				JSON_SERIALIZE("desc", Desc);
				JSON_SERIALIZE_ARRAY("known_inputs", KnownInputs);
				FillKnownInputsBuffer(KnownInputs, KnownInputsBuffer, KnownInputCount, true);
				JSON_SERIALIZE_ARRAY("output_files", OutputFiles);
				JSON_SERIALIZE("remote", bRemote);
				JSON_SERIALIZE("detour", bDetour);
				JSON_SERIALIZE("weight", Weight);
				JSON_SERIALIZE_ARRAY("deps", Deps);
			END_JSON_SERIALIZER
		};

		TSet<FString> AreadyRegistedFiles;
		for (const TSharedPtr <FJsonValue>& JsonValue : *Tasks)
		{
			if (!JsonValue.IsValid() || JsonValue->Type != EJson::Object)
			{
				logger.Error(TC("Invalid task entry. Not an object"));
				continue;
			}

			const TSharedPtr<FJsonObject>& TaskObject = JsonValue->AsObject();
			check(TaskObject.IsValid());

			FTaskDesc TaskDesc;
			if (!TaskDesc.FromJson(TaskObject))
			{
				logger.Error(TC("Invalid task entry. Not an object"));
				continue;
			}

			ProcessStartInfo SI;
			SI.application = TCHAR_TO_UBASTRING(*TaskDesc.App);
			SI.arguments = TCHAR_TO_UBASTRING(*TaskDesc.Arg);
			SI.workingDir = TCHAR_TO_UBASTRING(*TaskDesc.Dir);
			SI.description = TCHAR_TO_UBASTRING(*TaskDesc.Desc);

			EnqueueProcessInfo info{ SI };
			info.dependencies = reinterpret_cast<const u32*>(TaskDesc.Deps.GetData());
			info.dependencyCount = static_cast<u32>(TaskDesc.Deps.Num());
			info.canDetour = TaskDesc.bDetour;
			info.canExecuteRemotely = TaskDesc.bRemote;
			info.weight = TaskDesc.Weight;
			FDepDesc* DepPtr = App2Dep.Find(FPaths::GetCleanFilename(TaskDesc.App));
			TArray<uba::tchar> KnownInputsBuffer = DepPtr ? DepPtr->KnownInputsBuffer : TArray<uba::tchar>();
			KnownInputsBuffer.Append(TaskDesc.KnownInputsBuffer);
			info.knownInputs = KnownInputsBuffer.GetData();
			info.knownInputsBytes = KnownInputsBuffer.Num() * sizeof(uba::tchar);
			info.knownInputsCount = TaskDesc.KnownInputCount + (DepPtr ? DepPtr->KnownInputCount : 0);

			for (const FString& InputFile : TaskDesc.KnownInputs)
			{
				if (!AreadyRegistedFiles.Contains(InputFile))
				{
					session.RegisterNewFile(TCHAR_TO_UBASTRING(*InputFile));
					AreadyRegistedFiles.Add(InputFile);
				}
			}

			EnqueueProcess(info);
		}

		return true;
	}
			
	void FDynamicScheduler::RemoteActionFailedCrash(const ProcessHandle& InProcessHandle, const FString& InError)
	{
		PublishException(InProcessHandle, InError);

		/*if (_bIsCancelled)
		{
			HandleActionCancelled(queue, null, action);
			return;
		}*/

		const auto& StartInfo = InProcessHandle.GetStartInfo();
		logger.Warning(TC("Warning::%s %s [RemoteExecutor:%s]: Exited with error code %d (%s). This action will retry locally"), StartInfo.application, StartInfo.description, InProcessHandle.GetExecutingHost(), InProcessHandle.GetExitCode(), InError.IsEmpty() ? TEXT("with no output") : *InError);

		LocalRetryProcess.insert_or_assign(InProcessHandle, false);
		
		session.ReEnqueueProcess(InProcessHandle);
		/*queue.RequeueAction(action);

		lock(_actionsChangedLock)
		{
			_bActionsChanged = true;
		}*/
	}

	void FDynamicScheduler::PublishException(const ProcessHandle& InProcessHandle, const FString& InError)
	{
		if (IsConnected())
		{
			const auto& StartInfo = InProcessHandle.GetStartInfo();

			static FException Exception;
			Exception.Clear();
			*Exception.mutable_timestamp() = TCHAR_TO_UTF8(*FDateTime::Now().ToIso8601());  
			*Exception.mutable_buildid() = TCHAR_TO_UTF8(*(GBuildStats.BuildId));
			*Exception.mutable_mac() = TCHAR_TO_UTF8(*GetUniqueDeviceID()); 
			*Exception.mutable_exechost() = TCHAR_TO_UTF8(InProcessHandle.GetExecutingHost());
			Exception.set_exitcode(InProcessHandle.GetExitCode());
			*Exception.mutable_application() = TCHAR_TO_UTF8(StartInfo.application); 
			*Exception.mutable_arguments() = TCHAR_TO_UTF8(StartInfo.arguments);
			*Exception.mutable_workingdir() = TCHAR_TO_UTF8(StartInfo.workingDir); 
			*Exception.mutable_description() = TCHAR_TO_UTF8(StartInfo.description ? StartInfo.description : *InError);
			*Exception.mutable_logfile() = TCHAR_TO_UTF8(StartInfo.logFile);

			const std::string Protobuf = Exception.SerializeAsString();
			XiaoRedis::PublishException(Protobuf, Exception.buildid());
		}
	}

	void FDynamicScheduler::UpdateSystemSettings(uint32& OutCurInitAvaNum) const
	{
		OutCurInitAvaNum = 0;

		if (!IsConnected())
		{
			return;
		}

		std::unordered_map<std::string, std::string> AgentStats;
		bool Rtn = false;

		try
		{
			const auto Optional = SRedisClient->get(String::SSystemSettings);
			if (Optional.has_value())
			{
				const std::string SystemSettingsStr = Optional.value();
				if (SystemSettingsStr.length() > 0)
				{
					if (!GModifySystemSettings.ParseFromString(SystemSettingsStr))
					{
						logger.Error(TC("SystemSettings ParseFromString failed!"));
						return;
					}
				}
			}

			SRedisClient->hgetall(Hash::SAgentStats, std::inserter(AgentStats, AgentStats.begin()));
			Rtn = true;
		}
		CATCH_REDIS_EXCEPTRION()

		if (Rtn)
		{
			uint32 CurInitiatorNum = 0;
			for (const auto& Iter : AgentStats)
			{
				const std::string AgentId = Iter.first;
				if (AgentId.length() == 0)
				{
					continue;
				}

				FAgentProto Proto;
				if (!Proto.ParseFromString(Iter.second))
				{
					logger.Warning(TC("FAgentProto ParseFromString failed!"));
					continue;
				}

				if (Proto.status() == 1)
				{
					++CurInitiatorNum;
				}
			}
			OutCurInitAvaNum = GModifySystemSettings.maxinitiatornum() - CurInitiatorNum;
		}
	}

	void FDynamicScheduler::UpdateAgents(const int InLocalStatus, const int InAgentStatus, const bool bImmediate)
	{
		static double LastUpdateTime;
		if (!bImmediate)
		{
			if ((FPlatformTime::Seconds() - LastUpdateTime) < SUpdateTime)
			{
				return;
			}
			LastUpdateTime = FPlatformTime::Seconds();
		}

		if (!XiaoRedis::IsConnected())
		{
			return;
		}

		auto& Session = GetSession();

		std::string LocalMessage = "";
		if (!bImmediate)
		{
			LocalMessage = TCHAR_TO_UTF8(*FString::Printf(TEXT("Running [%u/%u/%u]"), Session.GetActiveProcessCount(), SFinishProcess, GBuildStats.TotalJobNum));
		}
		
		if (!UpdateAgent(GAgentUID, InLocalStatus, LocalMessage))
		{
			return;
		}

		/*SessionServer& SessionServer = GetSession();
		const auto& ClientSessions = SessionServer.GetClientSessions();*/
		for (const auto& Iter : Ip2Id)
		{
			std::string AgentMessage = "Helping " + GLocalMachineDesc;

			/*if (const auto Client = GetClientSession(Iter.Value))
			{
				AgentMessage = std::format("Helping \"{}\" with {}/{} core(s)", GLocalMachineDesc.c_str(), Client->usedSlotCount, Client->processSlotCount);
			}*/
			if (!UpdateAgent(Iter.Value, InAgentStatus, AgentMessage))
			{
				return;
			}
		}
	}

	void FDynamicScheduler::UpdateProgress(const int InStatus, const float InProgress, const bool bImmediate)
	{
		static double LastUpdate = 0.0f;
		if (FPlatformTime::Seconds() - LastUpdate < 0.5f)
		{
			return;
		}
		LastUpdate = FPlatformTime::Seconds();

		if (ProgressRegion)
		{
			auto Address = ProgressRegion->get_address();
			if (!Address)
			{
				static auto OnceCall = [this]() ->int {
					logger.Warning(TC("WARNING::ProgressRegion address is null"));
					return 0;
				}();
				
				return;
			}
			BuildProgress.set_status(InStatus);
			BuildProgress.set_progress(InProgress);
			
			try
			{
				BuildProgress.SerializePartialToArray(Address, BuildProgress.ByteSizeLong());
			}
			catch (std::exception& Ex)
			{
				logger.Warning(TC("UpdateProgress::Exception::%s"), *String2FString(Ex.what()));
			}
		}
	}

	void FDynamicScheduler::UpdateBuildStats()
	{
		if (IsConnected())
		{
			try
			{
				const std::string StatsKey = TCHAR_TO_UTF8(*GBuildStats.BuildId);
				const std::string JsonContent = TCHAR_TO_UTF8(*GBuildStats.ToJson(false));
				SRedisClient->hset(Hash::SBuildStat, StatsKey, JsonContent);
			}
			CATCH_REDIS_EXCEPTRION()
		}
	}

	SessionServer::ClientSession* FDynamicScheduler::GetClientSession(const std::string& InAgentId)
	{
		if (!Id2CId.contains(InAgentId))
		{
			logger.Warning(TC("Can\'t find the agent %s client"), InAgentId.c_str());
			return nullptr;
		}

		const uint32 ClientId = Id2CId[InAgentId];

		SessionServer& SessionServer = GetSession();
		const auto& Sessions = SessionServer.GetClientSessions();
		for (const auto& Client : Sessions)
		{
			if (Client && Client->
#if (ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 6)
				clientId
#else
				id
#endif
				== ClientId)
			{
				return Client;
			}
		}
		logger.Warning(TC("Can\'t find the %d client"), ClientId);
		return nullptr;
	}

	void FDynamicScheduler::TryConnectAgents()
	{
		static double LastTry = 0.0f;
		if (FPlatformTime::Seconds() - LastTry < 10.0f)
		{
			return;
		}
		LastTry = FPlatformTime::Seconds();

		try
		{
			std::vector<std::string> AgentHelpers;
			const int64 HelperNum = XiaoRedis::SRedisClient->llen(XiaoRedis::List::SEnbaleHelpers);
			if (HelperNum <= 0)
			{
				return;
			}			

			// 根据相关约束条件选择对满足条件的代理机器请求协助
			uint32 ConnectedAgentCount = 0;
			FString LocalGroup = SDefaultStr;
			if (!InitiatorProto.group().empty())
			{
				LocalGroup = String2FString(InitiatorProto.group());
			}
			GLocalMachineDesc = (InitiatorProto.desc().size() == 0) ? InitiatorProto.loginuser() : InitiatorProto.desc();
			XiaoRedis::SRedisClient->lrange(XiaoRedis::List::SEnbaleHelpers, 0, HelperNum, std::inserter(AgentHelpers, AgentHelpers.begin()));
			static bool bPrint = false;
			if (!bPrint)
			{
				bPrint = true;
				logger.Info(TC("Max number of Agents can help to scheduler:%u. Max number of processes that can be started:%u"), HelperNum, MaxProcessorCount);
			}
			const std::string Message = "Helping " + GLocalMachineDesc;
			int AgentIndex = 1;// 从目前看是从1开始的
			for (const auto& Id : AgentHelpers)
			{
				// 连接过后，不允许再连接 或者在之前已经无法连接
				if (AreadyConnectedSet.contains(Id) || CantConnectSet.contains(Id))
				{
					continue;
				}

				const auto Val = XiaoRedis::SRedisClient->hget(XiaoRedis::Hash::SAgentStats, Id);
				if (!Val)
				{
					logger.Warning(TC("Can\'t get agent ip[%s] %s data"), *String2FString(Id));
					continue;
				}
				
				const std::string Value = Val.value();
				if (Value.size() <= 0)
				{
					logger.Warning(TC("ConnectAgents Value is empty!"));
					continue;
				}

				FAgentProto Proto;
				if (!Proto.ParseFromString(Value))
				{
					logger.Warning(TC("FAgentProto ParseFromString failed!"));
					continue;
				}
				if (Proto.macaddress() != Id)
				{
					continue;
				}

				// 本地IP忽略
				FString Ip = String2FString(Proto.routerip());
				if (Ip == GLANV4 || Id == GAgentUID)
				{
					continue;
				}

				// 是否cpu架构是否一致
				if (!GModifySystemSettings.bignorearch())
				{
					if (InitiatorProto.cpuarch() != Proto.cpuarch())
					{
						continue;
					}
				}

				const FString AgentDesc = String2FString(Proto.desc().empty() ? Proto.loginuser() : Proto.desc());

				// 组别判断
				FString AgentGroup = SDefaultStr;
				if (!Proto.group().empty())
				{
					AgentGroup = String2FString(Proto.group());
				}
				if (LocalGroup != AgentGroup)
				{
					logger.Info(TC("Agent %s group:[%s] different group:[%s] with us!"), *AgentDesc, *AgentGroup, *LocalGroup);
					CantConnectSet.insert(Id);
					continue;
				}

				// 静态任务
				if (!Info.bDynamic)
				{
					// 是否超过最大能够启动的最大Processor
					if (MaxCoreAvailable >= MaxProcessorCount)
					{
						logger.Info(TC("Already over %d max processors!"), MaxProcessorCount);
						return;
					}

					// 是否已经足够连接的代理运行的了
					if (GBuildStats.RemainJobNum > uint32(InitiatorProto.localmaxcpu()))
					{
						if (MaxCoreAvailable >= (GBuildStats.RemainJobNum - InitiatorProto.localmaxcpu()))
						{
							logger.Info(TC("Already enough %u agent(s)! no need more agent(s)"), MaxAgentConCount);
							return;
						}
					}
					else
					{
						return;
					}
				}

				// 判断最大连接代理数量,避免占用不必要的计算资源
				if (ConnectedAgentCount < MaxAgentConCount)
				{
					// 是否是同一个网络中
					FString MappedAddress;
					const uint16 HelpPort = static_cast<uint16>(Proto.helperport());
					/*if (TryGetAddressAndPort(Proto, MappedAddress, HelpPort))
					{
						if (MappedAddress != GMappedAddress)
						{
							Ip = MappedAddress;
						}
					}*/

					NewAgentId = Id;

					verify(Info.Backend);

					if (session.GetServer().AddClient(*Info.Backend, TCHAR_TO_UBASTRING(*Ip), HelpPort, Info.Crypto))
					{
						Id2CId.insert_or_assign(NewAgentId, AgentIndex++);
						++ConnectedAgentCount;
						MaxCoreAvailable += Proto.helpercore();
						UpdateAgent(Id, EAgentStatus::Status_Helping, Message);
						Ip2Id.Add(MakeTuple(Ip, Id));
						AreadyConnectedSet.insert(NewAgentId);
					}
					else
					{
						CantConnectSet.insert(Id);
						logger.Warning(TC("Can\'t connect to agent:%s"), *AgentDesc);
					}
				}
				else
				{
					logger.Info(TC("Already connected %u Agents, over allow max %u agents!"), ConnectedAgentCount, MaxAgentConCount);
					return;
				}
			}
		}
		CATCH_REDIS_EXCEPTRION()
	}

	void FDynamicScheduler::ReleaseAgent(const uint32 InClientId, const uba::SessionServer::ClientSession* InSession)
	{
		if (AreadyDisconnecedSet.Contains(InClientId))
		{
			return;
		}

		logger.Info(TC("Disconnect with client::%d for release resource."), InClientId);
		const auto& Sessions = GetSession().GetClientSessions();
		const uba::SessionServer::ClientSession* Session = InSession;
		if (!Session)
		{
			for (int Index = Sessions.size() - 1; Index >= 0; --Index)
			{
				Session = Sessions[Index];
				if (!Session)
				{
					continue;
				}

				if (Session->
#if (ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 6)
					clientId
#else
					id
#endif
					== InClientId)
				{
					break;
				}
			}
		}

		if (!Session)
		{
			logger.Error(TC("Can\'t find the client session with id::%d"), InClientId);
			return;
		}

		MaxCoreAvailable -= Session->processSlotCount;
		session.GetServer().DisconnectClient(InClientId);
		AreadyDisconnecedSet.Add(InClientId);
		std::string AgentId;
		for (const auto& Iter : Id2CId)
		{
			if (Iter.second == InClientId)
			{
				AgentId = Iter.first;
			}
		}
		// 更新代理状态
		if (!AgentId.empty())
		{
			UpdateAgent(AgentId, EAgentStatus::Status_Ready, "");
			Id2CId.erase(AgentId);
			if (const auto AgentIp = Ip2Id.FindKey(AgentId))
			{
				Ip2Id.Remove(*AgentIp);
			}
		}
	}

	void FDynamicScheduler::TryReleaseAgents()
	{
		// 动态任务
		if (Info.bDynamic)
		{
			return;
		}

		// 独占式发起者
		if (InitiatorProto.bfixedinitator())
		{
			return;
		}

		// 动态的等所有的任务都完成时才进行后续判断
		if (Info.bDynamic && GBuildStats.RemainJobNum > 0)
		{
			return;
		}

		static double LastRelease = 0.0f;
		if ((FPlatformTime::Seconds() - LastRelease) < SUpdateTime)
		{
			return;
		}
		LastRelease = FPlatformTime::Seconds();

		const uint32 MaxRemoteCanRunNumNow = GetProcessCountThatCanRunRemotelyNow() - SActiveRemote;
		const uint32 MaxLocalProcessors = GetMaxLocalProcessors();
		if ((MaxCoreAvailable - MaxLocalProcessors) < MaxRemoteCanRunNumNow)
		{
			return;
		}

		// 协助核心总数 > 剩余远程运行任务数量已经
		const auto& Sessions = GetSession().GetClientSessions();
		if (Sessions.size() <= 0)
		{
			return;
		}

		// 尝试释放代理
		for (int Index = Sessions.size() - 1; Index >= 0; --Index)
		{
			const auto Client = Sessions[Index];
			if (!Client)
			{
				continue;
			}

			// 按需进行释放
			if ((MaxCoreAvailable - MaxLocalProcessors - Client->processSlotCount) < MaxRemoteCanRunNumNow)
			{
				return;
			}

			// 有些是观察者 
			if (Client->processSlotCount == 0)
			{
				continue;
			}

			// 还有在运行的Slot
			if (Client->usedSlotCount > 0)
			{
				continue;
			}

			// 或者已经断开
			if (AreadyDisconnecedSet.Contains(Client->
#if (ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 6)
				clientId
#else
				id
#endif
			) || !Client->enabled || Client->abort)
			{
				continue;
			}

			// 释放代理
			ReleaseAgent(Client->
#if (ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 6)
				clientId
#else
				id
#endif
				, Client);
		}
	}

	bool FDynamicScheduler::HandleTask(FMemoryReader& InReader)
	{
		uba::ProcessStartInfo ProcessInfo;

		uint32 JobId;
		InReader << JobId;

		FString Application;
		InReader << Application;
		if (Application.IsEmpty())
		{
			logger.Error(TC("JobId::%u Application::Is empty!"), JobId);
			return false;
		}
		ProcessInfo.application = TCHAR_TO_UBASTRING(*Application);

		FString Arguments;
		InReader << Arguments;
		if (Arguments.IsEmpty())
		{
			logger.Error(TC("JobId::%u Arguments::Is empty!"), JobId);
			return false;
		}
		ProcessInfo.arguments = TCHAR_TO_UBASTRING(*Arguments);

		FString Description;
		InReader << Description;
		ProcessInfo.description = TCHAR_TO_UBASTRING(*Description);

		FString WorkingDir;
		InReader << WorkingDir;
		if (WorkingDir.IsEmpty())
		{
			logger.Error(TC("JobId::%u WorkingDir::Is empty!"), JobId);
			return false;
		}
		ProcessInfo.workingDir = TCHAR_TO_UBASTRING(*WorkingDir);

		FString LogFile;
		InReader << LogFile;
		if (!LogFile.IsEmpty())
		{
			ProcessInfo.logFile = TCHAR_TO_UBASTRING(*LogFile);
		}

		InReader << ProcessInfo.writeOutputFilesOnFail;

		struct FExitedInfo
		{
			u32 jobId = 0;
			FString InputFile;
			FString OutputFile;
			FDynamicScheduler* Scheduler = nullptr;
		};

		const auto ExitInfo = new FExitedInfo;
		ExitInfo->jobId = JobId;
		ExitInfo->Scheduler = this;

		InReader << ExitInfo->InputFile;
		if (ExitInfo->InputFile.IsEmpty())
		{
			logger.Warning(TC("JobId::%u InputFile::Is empty!"), JobId);
			return false;
		}
		session.RegisterNewFile(TCHAR_TO_UBASTRING(*ExitInfo->InputFile));

		InReader << ExitInfo->OutputFile;
		if (ExitInfo->OutputFile.IsEmpty())
		{
			logger.Warning(TC("JobId::%u OutputFile::Is empty!"), JobId);
			return false;
		}

		ProcessInfo.userData = ExitInfo;
		ProcessInfo.exitedFunc = [](void* userData, const ProcessHandle& ph
#if (ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 6)
			, ProcessExitedResponse& rp
#endif
			)
			{
				FScopeLock ScopedLock(GCriticalSection);

				if (userData)
				{
					if (const auto InnerExitInfo = static_cast<FExitedInfo*>(userData))
					{
						if (InnerExitInfo->Scheduler)
						{
							if (ph.GetExitCode() == 0 && FPaths::FileExists(InnerExitInfo->InputFile))
							{
								IFileManager::Get().Delete(*InnerExitInfo->InputFile, true , true, true);
							}
							InnerExitInfo->Scheduler->session.RegisterDeleteFile(TCHAR_TO_UBASTRING(*InnerExitInfo->InputFile));
							InnerExitInfo->Scheduler->HandleResponse(ph, InnerExitInfo->jobId);
							InnerExitInfo->Scheduler->session.GetStorage().DeleteCasForFile(TCHAR_TO_UBASTRING(*InnerExitInfo->InputFile));
						}
						delete InnerExitInfo;
					}
				}
			};

		FString KnownInputs;
		InReader << KnownInputs;
		if (KnownInputsBuffer.IsEmpty())
		{
			TArray<FString> Sections;
			KnownInputs.ParseIntoArray(Sections, TEXT(";"), true);
			FillKnownInputsBuffer(Sections, KnownInputsBuffer, KnownInputCount, true);
		}

		uba::EnqueueProcessInfo Epi(ProcessInfo);
		InReader << Epi.weight;
		Epi.knownInputs = KnownInputsBuffer.GetData();
		Epi.knownInputsBytes = KnownInputsBuffer.Num() * sizeof(uba::tchar);
		Epi.knownInputsCount = KnownInputCount;

		checkf(Epi.knownInputs != nullptr, TEXT("Epi.knownInputs should not be nullptr"));

		EnqueueProcess(Epi);
		return true;
	}

	void FDynamicScheduler::HandleResponse(const ProcessHandle& InPh, const u32 InTaskId)
	{
		FTaskResponse Response;

		Response.ID = InTaskId;
		Response.ReturnCode = InPh.GetExitCode();

		PendingOutput.Enqueue(Response);
		
		if (Response.ID != 0)
		{
			FString Logs;
			for (const auto& LogLine : InPh.GetLogLines())
			{
				logger.Warning(TC("%s"), LogLine.text.c_str());
			}
		}
	}

	void FDynamicScheduler::FillKnownInputsBuffer(const TArray<FString>& InKnownInputs, TArray<uba::tchar>& OutKnownInputsBuffer, uint32& OutKnownInputCount, const bool bContainEndSymbol)
	{
		for (const FString& File : InKnownInputs)
		{
			if (File.IsEmpty())
			{
				continue;
			}

#if PLATFORM_WINDOWS
			auto& FileData = File.GetCharArray();
			const uba::tchar* FileName = FileData.GetData();
			const size_t FileNameLen = FileData.Num();
#else
			FStringToUbaStringConversion Conv(*File);
			const uba::tchar* FileName = Conv.Get();
			const size_t FileNameLen = strlen(FileName) + 1;
#endif
			const auto Num = OutKnownInputsBuffer.Num();
			OutKnownInputsBuffer.SetNum(Num + FileNameLen);
			FMemory::Memcpy(OutKnownInputsBuffer.GetData() + Num, FileName, FileNameLen * sizeof(uba::tchar));
			++OutKnownInputCount;
		}
		if (bContainEndSymbol)
		{
			OutKnownInputsBuffer.Add('\0');
		}
	}
}