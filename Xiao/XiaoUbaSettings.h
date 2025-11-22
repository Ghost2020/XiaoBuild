/**
  * @author cxx2020@outlook.com
  * @date 10:24 PM
 */
#pragma once


#include "Serialization/JsonSerializerMacros.h"

#if PLATFORM_WINDOWS
#include "Windows/AllowWindowsPlatformTypes.h"
#include "ShlObj_core.h"
#include "Windows/HideWindowsPlatformTypes.h"
#endif


#define JSON_MCI_VALUE(var) JSON_SERIALIZE(#var, var)


static FString GetDefaultDataDir(const FString AppName)
{
	FString DefaultDataDir;
#if PLATFORM_WINDOWS
	TCHAR* ProgramPath;
	HRESULT Ret = SHGetKnownFolderPath(FOLDERID_ProgramData, 0, NULL, &ProgramPath);
	if (SUCCEEDED(Ret))
	{
		DefaultDataDir = FPaths::ConvertRelativePathToFull(FPaths::Combine(ProgramPath, TEXT("XiaoBuild"), *AppName));
		CoTaskMemFree(ProgramPath);
	}
#elif PLATFORM_MAC
	DefaultDataDir = FPaths::ConvertRelativePathToFull(FPaths::Combine(TEXT("~/Library/Caches/XiaoBuild"), *AppName));
#else
	DefaultDataDir = FPaths::ConvertRelativePathToFull(FPaths::Combine(TEXT("~/.cache/XiaoBuild"), *AppName));
#endif

	return DefaultDataDir;
}


struct FUbaSchedulerSetting : FJsonSerializable
{
	virtual ~FUbaSchedulerSetting() {};

	FUbaSchedulerSetting& operator=(const FUbaSchedulerSetting& Other)
	{
		if (this != &Other)
		{
			Dir = Other.Dir; Port = Other.Port; bStandalone = Other.bStandalone; bLog = Other.bLog;
			bQuiet = Other.bQuiet; Loop = Other.Loop; Capacity = Other.Capacity;
			bCheckCas = Other.bCheckCas; CheckFileTable = Other.CheckFileTable; bCheckAws = Other.bCheckAws;
			bDeletecas = Other.bDeletecas; bGetCas = Other.bGetCas; bSummary = Other.bSummary; bNoCustoMalloc = Other.bNoCustoMalloc;
			bAllowMemoryMaps = Other.bAllowMemoryMaps; bEnableStdOut = Other.bEnableStdOut; bStoreRaw = Other.bStoreRaw;
			MaxLocalCore = Other.MaxLocalCore; MaxCpu = Other.MaxCpu; MaxCon = Other.MaxCon; bVisualizer = Other.bVisualizer;
			Crypto = Other.Crypto; bUseCache = Other.bUseCache;
		} 
		return *this;
	}

	bool operator==(const FUbaSchedulerSetting& Other) const
	{
		bSame = Dir == Other.Dir && Port == Other.Port && bStandalone == Other.bStandalone && bLog == Other.bLog && bQuiet == Other.bQuiet
			&& Loop == Other.Loop && Capacity == Other.Capacity && bCheckCas == Other.bCheckCas && CheckFileTable == Other.CheckFileTable
			&& bCheckAws == Other.bCheckAws && bDeletecas == Other.bDeletecas && bGetCas == Other.bGetCas && bSummary == Other.bSummary
			&& bNoCustoMalloc == Other.bNoCustoMalloc && bAllowMemoryMaps == Other.bAllowMemoryMaps && bEnableStdOut == Other.bEnableStdOut && bStoreRaw == Other.bStoreRaw
			&& MaxCpu == Other.MaxCpu && MaxLocalCore == Other.MaxLocalCore && MaxCon == Other.MaxCon && bVisualizer == Other.bVisualizer && Crypto == Other.Crypto
			&& bUseCache == Other.bUseCache;
		return bSame;
	}

	mutable bool bSame = true;

	FString Dir = GetDefaultDataDir(TEXT("XiaoScheduler"));// The directory used to store data
	uint32 Port = 1346;				// 用于Trace的端口
	bool bStandalone = false;
	bool bLog = false;				// Log all processes detouring information to file (only works with debug builds)
	bool bQuiet = false;			// Does not output any logging in console except errors
	uint32 Loop = 1;				// Loop the commandline <count> number of times. Will exit when/if it fails
	uint32 Capacity = 20;			// Capacity of local store. Defaults to %u gigabytes
	bool bCheckCas = false;			// Check so all cas entries are correct
	FString CheckFileTable;			// Check so file table has correct cas stored
	bool bCheckAws = false;			// Check if we are inside aws and output information about aws
	bool bDeletecas = false;		// Deletes the casdb
	bool bGetCas = false;			// Will print hash of application
	bool bSummary = false;			// Print summary at the end of a session
	bool bNoCustoMalloc = false;	// Disable custom allocator for processes. If you see odd crashes this can be tested
	bool bAllowMemoryMaps = false;
	bool bEnableStdOut = true;		// Enable stdout from process
	bool bStoreRaw = false;			// Disable compression of storage. This will use more storage and might improve performance
	uint32 MaxLocalCore = FPlatformMisc::NumberOfCoresIncludingHyperthreads();
	uint32 MaxCpu = 1024;			// Max number of processes that can be started.
	uint32 MaxCon = 8;				// Max number of Agents can connect help to scheduler
	bool bVisualizer = false;		// Spawn a visualizer that visualizes progress
	FString Crypto;					// Will enable crypto on network client/server
	bool bUseCache = false;			// Connect to cache server

	BEGIN_JSON_SERIALIZER
		JSON_MCI_VALUE(Dir);
		JSON_MCI_VALUE(Port);
		JSON_MCI_VALUE(bStandalone);
		JSON_MCI_VALUE(bLog);
		JSON_MCI_VALUE(bQuiet);
		JSON_MCI_VALUE(Loop);
		JSON_MCI_VALUE(Capacity);
		JSON_MCI_VALUE(bCheckCas);
		JSON_MCI_VALUE(CheckFileTable);
		JSON_MCI_VALUE(bCheckAws);
		JSON_MCI_VALUE(bDeletecas);
		JSON_MCI_VALUE(bGetCas);
		JSON_MCI_VALUE(bSummary);
		JSON_MCI_VALUE(bNoCustoMalloc);
		JSON_MCI_VALUE(bAllowMemoryMaps);
		JSON_MCI_VALUE(bEnableStdOut);
		JSON_MCI_VALUE(bStoreRaw);
		JSON_MCI_VALUE(MaxLocalCore);
		JSON_MCI_VALUE(MaxCon);
		JSON_MCI_VALUE(MaxCpu);
		JSON_MCI_VALUE(bVisualizer);
		JSON_MCI_VALUE(Crypto);
		JSON_MCI_VALUE(bUseCache);
	END_JSON_SERIALIZER
};


struct FUbaAgentSetting : FJsonSerializable
{
	explicit FUbaAgentSetting()
	{}

	explicit FUbaAgentSetting(const FUbaAgentSetting& InOther)
		: bEnableAgent(InOther.bEnableAgent), Dir(InOther.Dir)
		, ListenPort(InOther.ListenPort), MaxCpu(InOther.MaxCpu), MaxCon(InOther.MaxCon)
		, Mulcpu(InOther.Mulcpu), MaxWorkers(InOther.MaxWorkers), Capacity(InOther.Capacity), Config(InOther.Config), Stats(InOther.Stats)
		, bVerbose(InOther.bVerbose), bLog(InOther.bLog), bNoCustoMalloc(InOther.bNoCustoMalloc), bStoreRaw(InOther.bStoreRaw), bSendRaw(InOther.bSendRaw)
		, SendSize(InOther.SendSize), Named(InOther.Named), bNoPoll(InOther.bNoPoll), bNoStore(InOther.bNoStore), bResetStore(InOther.bResetStore)
		, bQuiet(InOther.bQuiet), MaxIdle(InOther.MaxIdle), bBinaryVersion(InOther.bBinaryVersion), bSummary(InOther.bSummary), EventFile(InOther.EventFile)
		, bSentry(InOther.bSentry), bNoProxy(InOther.bNoProxy), bKillRandom(InOther.bKillRandom), MemWait(InOther.MemWait), MemKill(InOther.MemKill)
		, Crypto(InOther.Crypto), PopulateCas(InOther.PopulateCas)
	{}

	virtual ~FUbaAgentSetting() {};

	FUbaAgentSetting& operator=(const FUbaAgentSetting& InOther)
	{
		if (this != &InOther)
		{
			bEnableAgent = InOther.bEnableAgent; Dir = InOther.Dir;
			ListenPort = InOther.ListenPort;  MaxCpu = InOther.MaxCpu; MaxCon = InOther.MaxCon;
			Mulcpu = InOther.Mulcpu; MaxWorkers = InOther.MaxWorkers; Capacity = InOther.Capacity; Config = InOther.Config; Stats = InOther.Stats;
			bVerbose = InOther.bVerbose; bLog = InOther.bLog; bNoCustoMalloc = InOther.bNoCustoMalloc; bStoreRaw = InOther.bStoreRaw; bSendRaw = InOther.bSendRaw;
			SendSize = InOther.SendSize; Named = InOther.Named; bNoPoll = InOther.bNoPoll; bNoStore = InOther.bNoStore; bResetStore = InOther.bResetStore;
			bQuiet = InOther.bQuiet; MaxIdle = InOther.MaxIdle; bBinaryVersion = InOther.bBinaryVersion; bSummary = InOther.bSummary; EventFile = InOther.EventFile;
			bSentry = InOther.bSentry; bNoProxy = InOther.bNoProxy; bKillRandom = InOther.bKillRandom; MemWait = InOther.MemWait; MemKill = InOther.MemKill;
			Crypto = InOther.Crypto; PopulateCas = InOther.PopulateCas;
		}
		return *this;
	}

	bool operator==(const FUbaAgentSetting& Other) const
	{
		bSame = ListenPort == Other.ListenPort && MaxCpu == Other.MaxCpu && MaxCon == Other.MaxCon && IsEqual(Other);
		return bSame;
	}

	bool IsEqual(const FUbaAgentSetting& Other) const
	{
		bSame = bEnableAgent == Other.bEnableAgent && Dir == Other.Dir && Mulcpu == Other.Mulcpu
			 && MaxWorkers == Other.MaxWorkers && Capacity == Other.Capacity && Config == Other.Config
			 && Stats == Other.Stats && bVerbose == Other.bVerbose && bLog == Other.bLog
			 && bNoCustoMalloc == Other.bNoCustoMalloc && bStoreRaw == Other.bStoreRaw && bSendRaw == Other.bSendRaw && SendSize == Other.SendSize
			 && Named == Other.Named && bNoPoll == Other.bNoPoll && bNoStore == Other.bNoStore && bResetStore == Other.bResetStore
			 && bQuiet == Other.bQuiet && MaxIdle == Other.MaxIdle && bBinaryVersion == Other.bBinaryVersion && bSummary == Other.bSummary
			 && EventFile == Other.EventFile && bSentry == Other.bSentry && bNoProxy == Other.bNoProxy && bKillRandom == Other.bKillRandom
			 && MemWait == Other.MemWait && MemKill == Other.MemKill && Crypto == Other.Crypto && PopulateCas == Other.PopulateCas;
		return bSame;
	}

	mutable bool bSame = true;

	bool bEnableAgent = true;
	FString Dir = GetDefaultDataDir(TEXT("UbaAgent"));// The directory used to store data
	uint32 ListenPort = 1345;		// Agent will listen for connections on port (default: %u) and help when connected
	uint32 MaxCpu = FPlatformMisc::NumberOfCoresIncludingHyperthreads(); // Max number of processes that can be started.
	uint32 Mulcpu = 1;				// This value multiplies with number of cpu to figure out max cpu. Defaults to 1.0
	uint32 MaxCon = 8;				// Max number of connections that can be started by agent. Defaults to \"%u\" (amount up to max will depend on ping)
	uint32 MaxWorkers = FPlatformMisc::NumberOfCoresIncludingHyperthreads(); // Max number of workers is started by agent.
	uint32 Capacity = 20;			// Capacity of local store. Defaults to %u gigabytes
	FString Config;					// Config file that contains options for various systems
	FString Name;					// The identifier of this agent. Defaults to \"%s\" on this machine
	uint32 Stats = 0;				// Print stats for each process if higher than threshold
	bool bVerbose = false;			// Print debug informvation to console
	bool bLog = false;				// Log all processes detouring information to file (only works with debug builds)
	bool bNoCustoMalloc = true;		// Disable custom allocator for processes. If you see odd crashes this can be tested
	bool bStoreRaw = false;			// Disable compression of storage. This will use more storage and might improve performance
	bool bSendRaw = false;			// Disable compression of send. This will use more bandwidth but less cpu
	uint32 SendSize = 256 * 1024;	// Max size of messages being sent from client to server (does not affect server to client)
	FString Named;					// Use named events and file mappings by providing the base name in this option
	bool bNoPoll = false;			// Does not keep polling for work; attempts to connect once then exits
	bool bNoStore = false;			// Does not use storage to store files (with a few exceptions such as binaries)
	bool bResetStore = false;		// Delete all cas
	bool bQuiet = false;			// Does not output any logging in console
	uint32 MaxIdle = ~0u;			// Max time agent will idle before disconnecting. Ignored if -nopoll is not set
	bool bBinaryVersion = false;	// Will use binaries as version. This will cause updates everytime binaries change on host side
	bool bSummary = false;			// Print summary at the end of a session
	FString EventFile;				// File containing external events to agent. Things like machine is about to be terminated etc
	bool bSentry = false;			// Enable sentry
	bool bNoProxy = false;			// Does not allow this agent to be a storage proxy for other agents
	bool bKillRandom = false;		// Kills random process and exit session
	uint32 MemWait = 95;			// The amount of memory needed to spawn a process. Set this to 100 to disable. Defaults to 95%%
	uint32 MemKill = 100;			// The amount of memory needed before processes starts to be killed. Set this to 100 to disable. Defaults to 95%%"
	FString Crypto;					// 32 character (16 bytes) crypto key used for secure network transfer
	FString PopulateCas;			// Prepopulate cas database with files in dir. If files needed exists on machine this can be an optimization

	BEGIN_JSON_SERIALIZER
		JSON_MCI_VALUE(bEnableAgent);
		JSON_MCI_VALUE(Dir);
		JSON_MCI_VALUE(ListenPort);
		JSON_MCI_VALUE(MaxCpu);
		JSON_MCI_VALUE(Mulcpu);
		JSON_MCI_VALUE(MaxCon);
		JSON_MCI_VALUE(MaxWorkers);
		JSON_MCI_VALUE(Capacity);
		JSON_MCI_VALUE(Config);
		JSON_MCI_VALUE(Name);
		JSON_MCI_VALUE(Stats);
		JSON_MCI_VALUE(bVerbose);
		JSON_MCI_VALUE(bLog);
		JSON_MCI_VALUE(bNoCustoMalloc);
		JSON_MCI_VALUE(bStoreRaw);
		JSON_MCI_VALUE(bSendRaw);
		JSON_MCI_VALUE(SendSize);
		JSON_MCI_VALUE(Named);
		JSON_MCI_VALUE(bNoPoll);
		JSON_MCI_VALUE(bNoStore);
		JSON_MCI_VALUE(bResetStore);
		JSON_MCI_VALUE(bQuiet);
		JSON_MCI_VALUE(MaxIdle);
		JSON_MCI_VALUE(bBinaryVersion);
		JSON_MCI_VALUE(bSummary);
		JSON_MCI_VALUE(EventFile);
		JSON_MCI_VALUE(bSentry);
		JSON_MCI_VALUE(bNoProxy);
		JSON_MCI_VALUE(bKillRandom);
		JSON_MCI_VALUE(MemWait);
		JSON_MCI_VALUE(MemKill);
		JSON_MCI_VALUE(Crypto);
		JSON_MCI_VALUE(PopulateCas);
	END_JSON_SERIALIZER
};

#undef JSON_MCI_VALUE