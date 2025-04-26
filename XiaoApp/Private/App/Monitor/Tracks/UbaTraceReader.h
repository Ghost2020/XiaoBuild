#pragma once

#include "CoreMinimal.h"
#include "HAL/PlatformProcess.h"
#include "UbaCommon.h"

class IMappedFileRegion;
class IMappedFileHandle;
#if PLATFORM_WINDOWS
struct FPlatformMemory::FSharedMemoryRegion;
#endif


namespace Xiao
{
	class FNetworkTrace;
	struct FBinaryReader;

	static FORCEINLINE FString BytesToText(const uint64 bytes)
	{
		if (bytes < 1000)
			return FString::Printf(TEXT("%ub"), uint64(bytes));
		if (bytes < 1000 * 1000)
			return FString::Printf(TEXT("%.1fkb"), double(bytes) / 1000ull);
		if (bytes < 1000ull * 1000 * 1000)
			return FString::Printf(TEXT("%.1fmb"), double(bytes) / (1000ull * 1000));
		if (bytes < 1000ull * 1000 * 1000 * 1000)
			return FString::Printf(TEXT("%.1fgb"), double(bytes) / (1000ull * 1000 * 1000));
		else
			return FString::Printf(TEXT("%.1ftb"), double(bytes) / (1000ull * 1000 * 1000 * 1000));
	}

	class FTraceView
	{
	public:
		struct FProcess
		{
			uint32 id = 0;
			uint32 exitCode = ~0u;
			uint64 start = 0;
			uint64 stop = 0;
			FString description;
			FString returnedReason;
			FString breadcrumbs;
			uint32 bitmapOffset = 0;
			bool bitmapDirty = true;
			bool cacheFetch = false;
			bool isRemote = false;
			uint64 createFilesTime = 0;
			uint64 writeFilesTime = 0;
			TArray<uint8> stats;
			TArray<FProcessLogLine> logLines;

			FProcess& operator=(const FProcess& Other)
			{
				if (this != &Other)
				{
					id = Other.id;
					exitCode = Other.exitCode;
					start = Other.start;
					stop = Other.stop;
					description = Other.description;
					returnedReason = Other.returnedReason;
					breadcrumbs = Other.breadcrumbs;
					bitmapOffset = Other.bitmapOffset;
					bitmapDirty = Other.bitmapDirty;
					isRemote = Other.isRemote;
					createFilesTime = Other.createFilesTime;
					writeFilesTime = Other.writeFilesTime;
					stats = Other.stats;
					logLines = Other.logLines;
				}
				return *this;
			}
		};

		struct FProcessor
		{
			TArray<FProcess> processes;
		};

		struct FSessionUpdate
		{
			uint64 time;
			uint64 send;
			uint64 recv;
			uint64 ping;
			uint64 memAvail;
			float cpuLoad;
			uint8 connectionCount;
		};

		struct FWorkRecord
		{
			const TCHAR* description = nullptr;
			uint64 start = 0;
			uint64 stop = 0;
			uint32 bitmapOffset = 0;
		};

		struct FWorkTrack
		{
			TArray<FWorkRecord> records;
		};

		struct FFileTransfer
		{
			FCasKey key;
			uint64 size = 0;
			FString hint;
			uint64 start = 0;
			uint64 stop = 0;

			FFileTransfer& operator=(const FFileTransfer& Other)
			{
				if (this != &Other)
				{
					key = Other.key;
					size = Other.size;
					hint = Other.hint;
					start = Other.start;
					stop = Other.stop;
				}
				return *this;
			}
		};

		struct FStatusUpdate
		{
			FString text;
			ELogEntryType type;
			FString link;
		};

		struct FSession
		{
			FString name;
			FString fullName;
			Guid clientUid;
			TArray<FProcessor> processors;
			TArray<FSessionUpdate> updates;
			TArray<FString> summary;
			TArray<FFileTransfer> fetchedFiles;
			TArray<FFileTransfer> storedFiles;
			FString notification;
			uint64 fetchedFilesBytes = 0;
			uint64 storedFilesBytes = 0;
			uint32 maxVisibleFiles = 0;

			float highestSendPerS = 0;
			float highestRecvPerS = 0;

			bool isReset = true;
			uint64 disconnectTime = ~uint64(0);
			uint64 prevUpdateTime = 0;
			uint64 prevSend = 0;
			uint64 prevRecv = 0;
			uint64 memTotal = 0;
			uint32 processActiveCount = 0;
			uint32 processExitedCount = 0;

			FString proxyName;
			bool proxyCreated = false;

			FSession& operator=(const FSession& Other)
			{
				if (this != &Other)
				{
					name = Other.name;
					fullName = Other.fullName;
					clientUid = Other.clientUid;
					notification = Other.notification;
					fetchedFilesBytes = Other.fetchedFilesBytes;
					storedFilesBytes = Other.storedFilesBytes;
					maxVisibleFiles = Other.maxVisibleFiles;
					highestSendPerS = Other.highestRecvPerS;
					highestRecvPerS = Other.highestRecvPerS;
					isReset = Other.isReset;
					disconnectTime = Other.disconnectTime;
					prevUpdateTime = Other.prevUpdateTime;
					prevSend = Other.prevSend;
					prevRecv = Other.prevRecv;
					memTotal = Other.memTotal;
					processActiveCount = Other.processActiveCount;
					processExitedCount = Other.processExitedCount;
					proxyName = Other.proxyName;
					proxyCreated = Other.proxyCreated;
				}
				return *this;
			}
		};

		struct FProcessLocation
		{
			uint32 sessionIndex = 0;
			uint32 processorIndex = 0;
			uint32 processIndex = 0;
			bool operator==(const FProcessLocation& o) const 
			{ 
				return sessionIndex == o.sessionIndex && processorIndex == o.processorIndex && processIndex == o.processIndex; 
			}
		};

		struct FCacheWrite
		{
			uint64 start = 0;
			uint64 end = 0;
			uint64 bytesSent = 0;
			bool success = false;
		};

		const FProcess& GetProcess(const FProcessLocation& loc);
		const FSession& GetSession(const FProcessLocation& loc);
		void Clear();

		TArray<FSession> sessions;
		TArray<FWorkTrack> workTracks;
		TArray<FString> strings;
		TMap<uint64, FStatusUpdate> statusMap;
		TMap<uint32, FCacheWrite> cacheWrites;
		uint64 realStartTime = 0;
		uint64 startTime = 0;
		uint64 frequency = 0;
		uint32 totalProcessActiveCount = 0;
		uint32 totalProcessExitedCount = 0;
		uint32 activeSessionCount = 0;
		uint32 version = 0;
		uint32 progressProcessesTotal = 0;
		uint32 progressProcessesDone = 0;
		uint32 progressErrorCount = 0;
		bool remoteExecutionDisabled = false;
		bool finished = true;
	};


	class FTraceReader
	{
	public:
		FTraceReader();
		~FTraceReader();

		// Use for file read
		bool ReadFile(FTraceView& out, const TCHAR* fileName, bool replay);
		bool UpdateReadFile(FTraceView& out, const uint64 maxTime, bool& outChanged);

		// Use for network
		bool StartReadClient(FTraceView& out, FNetworkTrace& client);
		bool UpdateReadClient(FTraceView& out, FNetworkTrace& client, bool& outChanged);

		// Use for local
		bool StartReadNamed(FTraceView& out, const TCHAR* namedTrace, bool silentFail = false, bool replay = false);
		bool UpdateReadNamed(FTraceView& out, const uint64 maxTime, bool& outChanged);

		bool ReadMemory(FTraceView& out, bool trackHost, const uint64 maxTime);
		bool ReadTrace(FTraceView& out, FBinaryReader& reader, const uint64 maxTime);
		void StopAllActive(FTraceView& out, const uint64 stopTime);
		void Reset(FTraceView& out);
		void Unmap();

		Guid ReadClientId(FTraceView& out, FBinaryReader& reader);
		FTraceView::FSession& GetSession(FTraceView& out, const uint32 sessionIndex);
		FTraceView::FSession* GetSession(FTraceView& out, const Guid& clientUid);

		FTraceView::FProcess* ProcessBegin(FTraceView& out, const uint32 sessionIndex, const uint32 id, const uint64 time, const TCHAR* description);
		FTraceView::FProcess* ProcessEnd(FTraceView& out, uint32& outSessionIndex, const uint32 id, const uint64 time);

		std::unordered_map<uint32, FTraceView::FProcessLocation> m_activeProcesses;

		struct FWorkRecordLocation { uint32 track; uint32 index; };
		std::unordered_map<uint32, FWorkRecordLocation> m_activeWorkRecords;

		TArray<uint32> m_sessionIndexToSession;

		// TraceChannel m_channel;
		FRWLock m_memoryLock;
		TUniquePtr<IMappedFileHandle> m_fileHandle = nullptr;
		TUniquePtr<IMappedFileRegion> m_mappedFile = nullptr;
		TUniquePtr<FPlatformMemory::FSharedMemoryRegion> m_sharedMemeoryRegion = nullptr;

		FString m_namedTrace;
		uint8* m_memoryBegin = nullptr;
		uint8* m_memoryPos = nullptr;
		uint8* m_memoryEnd = nullptr;
		FProcHandle m_hostProcess;
	};
}