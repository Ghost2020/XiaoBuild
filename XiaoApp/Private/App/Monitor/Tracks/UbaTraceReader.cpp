#include "UbaTraceReader.h"
#include "BinaryReader.h"
#include "HAL/PlatformFileManager.h"
#include "HAL/PlatformMemory.h"
#include "Async/MappedFileHandle.h"
#include "UbaStats.h"
#include "UbaNetworkTrace.h"
#include "XiaoLog.h"
#if PLATFORM_MAC
#endif

#if PLATFORM_WINDOWS
#include "Windows/WindowsHWrapper.h"
#define UBA_CHAR TCHAR
#else
#include <sys/time.h>
#define UBA_CHAR TCHAR
#endif

namespace Xiao
{
	static constexpr uint32 TraceVersion = 34;
	static constexpr uint32 TraceReadCompatibilityVersion = 6;

    const FTraceView::FProcess& FTraceView::GetProcess(const FProcessLocation& loc)
    {
		static FTraceView::FProcess emptyProcess;
		if (loc.sessionIndex >= uint32(sessions.Num()))
			return emptyProcess;
		auto& session = sessions[loc.sessionIndex];
		if (loc.processorIndex >= uint32(session.processors.Num()))
			return emptyProcess;
		auto& processor = session.processors[loc.processorIndex];
		if (loc.processIndex >= uint32(processor.processes.Num()))
			return emptyProcess;
		return processor.processes[loc.processIndex];
    }

	const FTraceView::FSession& FTraceView::GetSession(const FProcessLocation& loc)
	{
		if (loc.sessionIndex < uint32(sessions.Num()))
			return sessions[loc.sessionIndex];
		static FTraceView::FSession emptySession;
		return emptySession;
	}

	void FTraceView::Clear()
	{
		sessions.Empty();
		workTracks.Empty();
		strings.Empty();
		statusMap.Empty();
		cacheWrites.Empty();
		startTime = 0;
		totalProcessActiveCount = 0;
		totalProcessExitedCount = 0;
		activeSessionCount = 0;
		progressProcessesTotal = 0;
		progressProcessesDone = 0;
		progressErrorCount = 0;
		remoteExecutionDisabled = false;
		finished = true;
	}

	FTraceReader::FTraceReader()
	{
		FPlatformTime::InitTiming();
	}

	FTraceReader::~FTraceReader()
	{
		Unmap();
	}

	inline static uint64 GetFrequency()
	{
		static uint64 frequency = static_cast<uint64>(1.0f / FPlatformTime::GetSecondsPerCycle64());
		return frequency;
	}

	inline static uint64 ConvertTime(const FTraceView& view, const uint64 time)
	{
		return time * GetFrequency() / view.frequency;
	}

	inline static uint64 UsToTime(const uint64 us) { return us * GetFrequency() / 1'000'000; }
	inline static float TimeToS(const uint64 time, const uint64 frequency) { return float(double(time) / double(frequency)); }
	inline static float TimeToS(const uint64 time) { return TimeToS(time, GetFrequency()); }

	static FORCEINLINE uint64 GetSystemTimeUs()
	{
#if PLATFORM_WINDOWS
		constexpr uint64 EPOCH_DIFF = 11'644'473'600ull; // Seconds from 1 Jan. 1601 to 1970 (windows to linux)
		FILETIME st;
		GetSystemTimeAsFileTime(&st);
		return *(uint64*)&st / 10 - (EPOCH_DIFF * 1'000'000ull);
#else
		timeval tv;
		gettimeofday(&tv, NULL); // Returns time in microseconds since 1 Jan 1970
		return uint64(tv.tv_sec) * 1'000'000ull + uint64(tv.tv_usec);
#endif
	}

#if PLATFORM_MAC
	
	static SIZE_T FileMappingAlignment = FPlatformMemory::GetConstants().PageSize;

	class FMacMappedFileRegion final : public IMappedFileRegion
	{
	public:
		class FMacMappedFileHandle* Parent;
		const uint8* AlignedPtr;
		uint64 AlignedSize;

		FMacMappedFileRegion(const uint8* InMappedPtr, const uint8* InAlignedPtr, size_t InMappedSize, uint64 InAlignedSize, const FString& InDebugFilename, size_t InDebugOffsetIntoFile, class FMacMappedFileHandle* InParent)
			: IMappedFileRegion(InMappedPtr, InMappedSize, InDebugFilename, InDebugOffsetIntoFile)
			, Parent(InParent)
			, AlignedPtr(InAlignedPtr)
			, AlignedSize(InAlignedSize)
		{
		}

		~FMacMappedFileRegion();

		virtual void PreloadHint(int64 PreloadOffset = 0, int64 BytesToPreload = MAX_int64) override
		{
			int64 Size = GetMappedSize();
			const uint8* Ptr = GetMappedPtr();
			int32 FoolTheOptimizer = 0;
			while (Size > 0)
			{
				FoolTheOptimizer += Ptr[0];
				Size -= 4096;
				Ptr += 4096;
			}
			if (FoolTheOptimizer == 0xbadf00d)
			{
				FPlatformProcess::Sleep(0.0f); // this will more or less never happen, but we can't let the optimizer strip these reads
			}
		}

	};

	class FMacMappedFileHandle final : public IMappedFileHandle
	{
		const uint8* MappedPtr;
		FString Filename;
		int32 NumOutstandingRegions;
		int FileHandle;

	public:

		FMacMappedFileHandle(int InFileHandle, int64 FileSize, const FString& InFilename)
			: IMappedFileHandle(FileSize)
			, MappedPtr(nullptr)
#if !UE_BUILD_SHIPPING
			, Filename(InFilename)
#endif
			, NumOutstandingRegions(0)
			, FileHandle(InFileHandle)
		{
		}

		~FMacMappedFileHandle()
		{
			check(!NumOutstandingRegions); // can't delete the file before you delete all outstanding regions
			close(FileHandle);
		}

		virtual IMappedFileRegion* MapRegion(int64 Offset = 0, int64 BytesToMap = MAX_int64, bool bPreloadHint = false) override
		{
			LLM_PLATFORM_SCOPE(ELLMTag::PlatformMMIO);
			check(Offset < GetFileSize()); // don't map zero bytes and don't map off the end of the file
			BytesToMap = FMath::Min<int64>(BytesToMap, GetFileSize() - Offset);
			check(BytesToMap > 0); // don't map zero bytes


			// const uint8* MapPtr = (const uint8 *)mmap(NULL, BytesToMap, PROT_READ, MAP_PRIVATE | (bPreloadHint ? MAP_POPULATE : 0), FileHandle, Offset);
			// const uint8* MapPtr = (const uint8 *)mmap(NULL, BytesToMap, PROT_READ, MAP_PRIVATE, FileHandle, Offset);
			//		const uint8* MapPtr = (const uint8 *)mmap(NULL, BytesToMap, PROT_READ, MAP_SHARED, FileHandle, Offset);

			const int64 AlignedOffset = AlignDown(Offset, FileMappingAlignment);
			//File mapping can extend beyond file size. It's OK, kernel will just fill any leftover page data with zeros
			const int64 AlignedSize = Align(BytesToMap + Offset - AlignedOffset, FileMappingAlignment);

			const uint8* AlignedMapPtr = (const uint8*)mmap(NULL, AlignedSize, PROT_READ, MAP_PRIVATE, FileHandle, AlignedOffset);
			if (AlignedMapPtr == (const uint8*)-1 || AlignedMapPtr == nullptr)
			{
				XIAO_LOG(Warning, TEXT("Failed to map memory %s, error is %d"), *Filename, errno);
				return nullptr;
			}
			LLM_IF_ENABLED(FLowLevelMemTracker::Get().OnLowLevelAlloc(ELLMTracker::Platform, AlignedMapPtr, AlignedSize));

			// create a mapping for this range
			const uint8* MapPtr = AlignedMapPtr + Offset - AlignedOffset;
			FMacMappedFileRegion* Result = new FMacMappedFileRegion(MapPtr, AlignedMapPtr, BytesToMap, AlignedSize, Filename, Offset, this);
			NumOutstandingRegions++;
			return Result;
		}

		void UnMap(FMacMappedFileRegion* Region)
		{
			LLM_PLATFORM_SCOPE(ELLMTag::PlatformMMIO);
			check(NumOutstandingRegions > 0);
			NumOutstandingRegions--;

			LLM_IF_ENABLED(FLowLevelMemTracker::Get().OnLowLevelFree(ELLMTracker::Platform, (void*)Region->AlignedPtr));
			int Res = munmap((void*)Region->AlignedPtr, Region->AlignedSize);
			checkf(Res == 0, TEXT("Failed to unmap, error is %d, errno is %d [params: %x, %d]"), Res, errno, MappedPtr, GetFileSize());
		}
	};
	
	static IMappedFileHandle* OpenMapped(const TCHAR* fileName)
	{
		FString FinalPath(fileName);
		FILE* FP;
		FP = fopen(TCHAR_TO_UTF8(*FinalPath), "r");
		if (FP == nullptr)
		{
			return nullptr;
		}

		int32 Handle = fileno(FP);

		if (Handle != -1)
		{
			struct stat FileInfo;
			FileInfo.st_size = -1;
			// check the read path
			if (fstat(Handle, &FileInfo) == -1)
			{
				return nullptr;
			}
			const uint64 FileSize = FileInfo.st_size;

			return new FMacMappedFileHandle(Handle, FileSize, FinalPath);
		}
		
		return nullptr;
	}

	FMacMappedFileRegion::~FMacMappedFileRegion()
	{
		Parent->UnMap(this);
	}
#endif

	bool FTraceReader::ReadFile(FTraceView& out, const TCHAR* fileName, bool replay)
	{
		Reset(out);

		m_fileHandle.Reset(
#if !PLATFORM_MAC
			FPlatformFileManager::Get().GetPlatformFile().OpenMapped(fileName)
#else
			OpenMapped(fileName)
#endif
		);

		if (!m_fileHandle)
		{
			XIAO_LOG(Error, TEXT("Can't open mapped::%s."), fileName);
			return false;
		}

		m_mappedFile.Reset(m_fileHandle->MapRegion());
		if (!m_mappedFile)
		{
			XIAO_LOG(Error, TEXT("Can't map region::%s."), fileName);
			return false;
		}

		m_memoryBegin = const_cast<uint8*>(m_mappedFile->GetMappedPtr());
		const auto mappedSize = m_mappedFile->GetMappedSize();
		m_memoryEnd = m_memoryBegin + mappedSize;

		FBinaryReader reader(m_memoryBegin);

		const uint32 traceSize = reader.ReadU32(); (void)traceSize;
		const uint32 version = reader.ReadU32();
		if (version < TraceReadCompatibilityVersion || version > TraceVersion)
		{
			Unmap();
			XIAO_LOG(Error, TEXT("Incompatible trace version (%u). Current executable supports version %u to %u."), version, TraceReadCompatibilityVersion, TraceVersion);
			return false;
		}

		out.version = version;
		reader.ReadU32(); // ProcessId
		uint64 traceSystemStartTimeUs = 0;
		if (version >= 18)
			traceSystemStartTimeUs = reader.Read7BitEncoded();
		if (version >= 18)
			out.frequency = reader.Read7BitEncoded();
		else
			out.frequency = GetFrequency();

		out.realStartTime = reader.Read7BitEncoded();

		out.startTime = out.realStartTime;
		if (replay)
			out.startTime = FPlatformTime::Cycles64();
		else if (traceSystemStartTimeUs)
		{
			out.startTime = FPlatformTime::Cycles64() - UsToTime(GetSystemTimeUs() - traceSystemStartTimeUs);
		}

		m_memoryPos += reader.GetPosition();
		out.finished = false;

		while (reader.GetPosition() < uint64(mappedSize))
		{
			if (!ReadTrace(out, reader, ~uint64(0)))
			{
				Unmap();
				return false;
			}
		}
		out.finished = true;
		Unmap();
		return true;
	}

	bool FTraceReader::UpdateReadFile(FTraceView& out, const uint64 maxTime, bool& outChanged)
	{
		FBinaryReader traceReader(m_memoryPos);
		uint64 left = uint64(m_memoryEnd - m_memoryPos);
		while (traceReader.GetPosition() < left)
		{
			uint64 pos = traceReader.GetPosition();
			if (!ReadTrace(out, traceReader, maxTime))
				return false;
			if (pos == traceReader.GetPosition())
			{
				if (!traceReader.GetLeft())
					out.finished = true;
				break;
			}
		}
		outChanged = traceReader.GetPosition() != 0 || !m_activeProcesses.empty();
		m_memoryPos += traceReader.GetPosition();
		return true;
	}

	bool FTraceReader::StartReadClient(FTraceView& out, FNetworkTrace& client)
	{
		const uint32 traceMemSize = 128 * 1024 * 1024;
		const FGuid Guid = FGuid::NewGuid();
		m_sharedMemeoryRegion.Reset(FPlatformMemory::MapNamedSharedMemoryRegion(Guid.ToString(EGuidFormats::DigitsWithHyphensInBraces), true, (FPlatformMemory::ESharedMemoryAccess::Read | FPlatformMemory::ESharedMemoryAccess::Write), traceMemSize));
		if (!m_sharedMemeoryRegion)
		{
			XIAO_LOG(Error, TEXT("MapNamedSharedMemoryRegion failed!"));
			return false;
		}

		m_memoryBegin = static_cast<uint8*>(m_sharedMemeoryRegion->GetAddress());
		m_memoryPos = m_memoryBegin;
		m_memoryEnd = m_memoryBegin;

		out.finished = false;
		out.sessions.AddDefaulted();
		out.sessions.Last().name = TEXT("LOCAL");
		bool changed;
		return UpdateReadClient(out, client, changed);
	}

	bool FTraceReader::UpdateReadClient(FTraceView& out, FNetworkTrace& client, bool& outChanged)
	{
		outChanged = false;
		if (!m_sharedMemeoryRegion.IsValid())
			return true;

		while (true)
		{
			uint32 pos = uint32(m_memoryEnd - m_memoryBegin);
			TArray<uint8> ReceiveBuffer;
			if (!client.SendTrace(pos, ReceiveBuffer))
			{
				for (auto& session : out.sessions)
				{
					for (auto& processor : session.processors)
					{
						if (processor.processes.Num() > 0)
						{
							auto& process = processor.processes.Last();
							if (process.stop == ~uint64(0))
							{
								process.stop = FPlatformTime::Cycles64() - out.startTime;
								process.exitCode = -1; // This is wrong but since we didn't get the final result we can't tell if it was success or not
								process.bitmapDirty = true;
								process.returnedReason = FString::Printf(TEXT("Exception occured::can\'t [%d] pos trace."), pos);
							}
						}
					}
				}
				out.finished = true;
				return false;
			}
			FBinaryReader reader(ReceiveBuffer.GetData(), 0, ReceiveBuffer.Num());
			const uint32 remotePos = reader.ReadU32();
			const uint32 left = uint32(reader.GetLeft());
			reader.ReadBytes(m_memoryEnd, left);
			pos += left;
			m_memoryEnd += left;
			if (remotePos == pos)
				break;
		}
		outChanged = !m_activeProcesses.empty() || m_memoryPos != m_memoryEnd;
		return ReadMemory(out, false, ~uint64(0));
	}

	bool FTraceReader::StartReadNamed(FTraceView& out, const TCHAR* namedTrace, bool silentFail, bool replay)
	{
		Reset(out);

		if (!namedTrace)
			namedTrace = TEXT("");

		if (*namedTrace && m_namedTrace != namedTrace)
		{
			m_sharedMemeoryRegion.Reset(FPlatformMemory::MapNamedSharedMemoryRegion(namedTrace, false, (FPlatformMemory::ESharedMemoryAccess::Read | FPlatformMemory::ESharedMemoryAccess::Write), 0));
			m_memoryBegin = static_cast<uint8*>(m_sharedMemeoryRegion->GetAddress());
			if (!m_memoryBegin)
				return false;
		}

		m_namedTrace = namedTrace;

		m_memoryPos = m_memoryBegin;
		m_memoryEnd = m_memoryBegin;
		out.finished = false;
		out.sessions.AddDefaulted();
		bool changed;
		return UpdateReadNamed(out, replay ? 0ull : ~uint64(0), changed);
	}

	bool FTraceReader::UpdateReadNamed(FTraceView& out, const uint64 maxTime, bool& outChanged)
	{
		outChanged = false;
		if (!m_memoryBegin)
			return true;

		m_memoryEnd = m_memoryBegin + *(uint32*)m_memoryBegin;
		outChanged = !m_activeProcesses.empty() || m_memoryPos != m_memoryEnd;
		const bool res = ReadMemory(out, true, maxTime);

		if (m_hostProcess.IsValid() && !FPlatformProcess::IsProcRunning(m_hostProcess))
			StopAllActive(out, FPlatformTime::Cycles64() - out.realStartTime);

		if (res || m_namedTrace.IsEmpty())
			return true;

		// Move memory to local mapping in case we want to replay
		const uint64 traceMemSize = m_memoryEnd - m_memoryBegin;
		uint8* memoryBegin = nullptr;
		auto sharedMemeoryRegion = FPlatformMemory::MapNamedSharedMemoryRegion(TEXT(""), true, (FPlatformMemory::ESharedMemoryAccess::Read | FPlatformMemory::ESharedMemoryAccess::Write), traceMemSize);
		if (sharedMemeoryRegion)
		{
			memoryBegin = static_cast<uint8*>(sharedMemeoryRegion->GetAddress());
			if (memoryBegin)
			{
				FMemory::Memcpy(memoryBegin, m_memoryBegin, traceMemSize);
			}
		}

		const uint64 pos = m_memoryPos - m_memoryBegin;
		const uint64 end = m_memoryEnd - m_memoryBegin;
		Unmap();
		m_sharedMemeoryRegion.Reset(sharedMemeoryRegion);
		m_memoryPos = memoryBegin + pos;
		m_memoryEnd = memoryBegin + end;
		m_memoryBegin = memoryBegin;

		return res;
	}

	bool FTraceReader::ReadMemory(FTraceView& out, bool trackHost, const uint64 maxTime)
	{
		if (m_memoryEnd == m_memoryPos)
			return true;

		const uint64 toRead = m_memoryEnd - m_memoryPos;
		FBinaryReader traceReader(m_memoryPos);

		if (m_memoryPos == m_memoryBegin)
		{
			if (toRead < 128)
				return true;

			const uint32 traceSize = traceReader.ReadU32(); (void)traceSize;
			const uint32 version = traceReader.ReadU32();
			
			if (version < TraceReadCompatibilityVersion || version > TraceVersion)
			{
				XIAO_LOG(Error, TEXT("Incompatible trace version (%u). Current executable supports version %u to %u."), version, TraceReadCompatibilityVersion, TraceVersion);
				return false;
			}

			const bool replay = maxTime != ~uint64(0);

			out.version = version;
			const uint32 hostProcessId = traceReader.ReadU32();
			m_hostProcess.Reset();
			if (trackHost && !replay)
				m_hostProcess = FPlatformProcess::OpenProcess(hostProcessId);
			uint64 traceSystemStartTimeUs = 0;
			if (version >= 18)
				traceSystemStartTimeUs = traceReader.Read7BitEncoded();
			if (version >= 18)
				out.frequency = traceReader.Read7BitEncoded();
			else
				out.frequency = GetFrequency();

			out.realStartTime = traceReader.Read7BitEncoded();
			if (traceSystemStartTimeUs)
			{
				out.realStartTime = FPlatformTime::Cycles64() - UsToTime(GetSystemTimeUs() - traceSystemStartTimeUs);
			}

			out.startTime = out.realStartTime;
			if (replay)
			{
				out.startTime = FPlatformTime::Cycles64();
			}
		}

		uint64 lastPos = traceReader.GetPosition();
		while (lastPos != toRead)
		{
			if (!ReadTrace(out, traceReader, maxTime))
				return false;
			const uint64 pos = traceReader.GetPosition();
			if (pos == lastPos)
				break;
			check(pos <= toRead);
			lastPos = pos;
		}
		m_memoryPos += lastPos;
		return true;
	}

	bool FTraceReader::ReadTrace(FTraceView& out, FBinaryReader& reader, const uint64 maxTime)
	{
		const uint64 readPos = reader.GetPosition();
		const uint8 traceType = reader.ReadByte();
		uint64 time = 0;
		if (out.version >= 15 && traceType != TraceType_String)
		{
			time = ConvertTime(out, reader.Read7BitEncoded());
			if (time > maxTime)
			{
				reader.SetPosition(readPos);
				return true;
			}
		}

		switch (traceType)
		{
		case TraceType_SessionAdded:
		{
			TStringBuilderWithBuffer<UBA_CHAR, 128> sessionName;
			reader.ReadString(sessionName);
			TStringBuilderWithBuffer<UBA_CHAR, 512> sessionInfo;
			reader.ReadString(sessionInfo);
			const Guid clientUid = ReadClientId(out, reader);
			const uint32 sessionIndex = reader.ReadU32();

			TStringBuilderWithBuffer<UBA_CHAR, 512> fullName;
			fullName.Append(sessionName.GetData()).Append(TEXT(" (")).Append(sessionInfo.GetData()).Append(TEXT(")"));
			const FString SessionFullName = fullName.ToString();

			// Check if we can re-use existing session (same machine was disconnected and then reconnected)
			uint32 virtualSessionIndex = sessionIndex;
			for (uint32 i = 0; i != out.sessions.Num(); ++i)
			{
				auto& oldSession = out.sessions[i];
				if (oldSession.fullName != SessionFullName)
					continue;
				if (oldSession.disconnectTime == ~uint64(0))
					break;
				oldSession.isReset = true;
				oldSession.disconnectTime = ~uint64(0);
				oldSession.proxyName.Empty();
				oldSession.proxyCreated = false;
				oldSession.notification.Empty();
				//oldSession.fetchedFiles.clear();
				//oldSession.storedFiles.clear();
				virtualSessionIndex = i;
				break;
			}


			if (out.sessions.Num() <= int32(virtualSessionIndex))
			{
				out.sessions.SetNum(virtualSessionIndex + 1);
			}

			if (m_sessionIndexToSession.Num() <= int32(sessionIndex))
				m_sessionIndexToSession.SetNum(sessionIndex + 1);

			m_sessionIndexToSession[sessionIndex] = virtualSessionIndex;

			FTraceView::FSession& session = GetSession(out, sessionIndex);
			session.name = sessionName.GetData();
			session.fullName = SessionFullName;
			session.clientUid = clientUid;

			++out.activeSessionCount;
			break;
		}
		case TraceType_SessionUpdate:
		{
			if (out.version < 15)
			{
				time = reader.Read7BitEncoded();
				if (time > maxTime)
				{
					reader.SetPosition(readPos);
					return true;
				}
			}

			uint32 sessionIndex;
			uint8 connectionCount = 0;
			uint64 totalSend;
			uint64 totalRecv;
			uint64 lastPing;

			if (out.version >= 14)
			{
				sessionIndex = uint32(reader.Read7BitEncoded());
				connectionCount = uint8(reader.Read7BitEncoded());
				totalSend = reader.Read7BitEncoded();
				totalRecv = reader.Read7BitEncoded();
				lastPing = reader.Read7BitEncoded();
			}
			else
			{
				sessionIndex = reader.ReadU32();
				totalSend = reader.ReadU64();
				totalRecv = reader.ReadU64();
				lastPing = reader.Read7BitEncoded();
			}

			uint64 memAvail = 0;
			uint64 memTotal = 0;
			if (out.version >= 9)
			{
				memAvail = reader.Read7BitEncoded();
				memTotal = reader.Read7BitEncoded();
			}

			float cpuLoad = 0;
			if (out.version >= 13)
			{
				uint32 value = reader.ReadU32();
				cpuLoad = *(float*)&value;
			}
			FTraceView::FSession& session = GetSession(out, sessionIndex);
			if (session.isReset)
			{
				session.isReset = false;
				session.prevUpdateTime = 0;
				session.prevSend = 0;
				session.prevRecv = 0;
				session.memTotal = 0;
				if (!session.updates.IsEmpty())
					session.updates.Add({ time, 0, 0, lastPing, memAvail, cpuLoad, connectionCount });
			}
			else
			{
				checkf(session.updates.Num() > 0, TEXT("session.updates Num must >= 0"));
				auto& prevUpdate = session.updates.Last();
				session.prevSend = prevUpdate.send;
				session.prevRecv = prevUpdate.recv;
				session.prevUpdateTime = session.updates.Last().time;
			}

			//session.lastPing = lastPing;
			//session.memAvail = memAvail;
			session.memTotal = memTotal;
			session.updates.Add({ time, totalSend, totalRecv, lastPing, memAvail, cpuLoad, connectionCount });

			const uint64 send = totalSend - session.prevSend;
			const uint64 recv = totalRecv - session.prevRecv;
			session.highestSendPerS = FMath::Max(session.highestSendPerS, float(send) / TimeToS(time - session.prevUpdateTime));
			session.highestRecvPerS = FMath::Max(session.highestRecvPerS, float(recv) / TimeToS(time - session.prevUpdateTime));
			break;
		}
		case TraceType_SessionDisconnect:
		{
			const uint32 sessionIndex = reader.ReadU32();
			if (out.version < 15)
			{
				time = reader.Read7BitEncoded();
				if (time > maxTime)
				{
					reader.SetPosition(readPos);
					return true;
				}
			}

			FTraceView::FSession& session = GetSession(out, sessionIndex);
			session.disconnectTime = time;
			session.maxVisibleFiles = 0;
			for (auto& it : session.fetchedFiles)
				if (it.stop == ~uint64(0))
					it.stop = time;
			for (auto& it : session.storedFiles)
				if (it.stop == ~uint64(0))
					it.stop = time;

			--out.activeSessionCount;
			break;
		}
		case TraceType_SessionNotification:
		{
			const uint32 sessionIndex = reader.ReadU32();
			FTraceView::FSession& session = GetSession(out, sessionIndex);
			session.notification = reader.ReadString();
			break;
		}
		case TraceType_SessionSummary:
		{
			const uint32 sessionIndex = reader.ReadU32();
			const uint32 lineCount = reader.ReadU32();

			FTraceView::FSession& session = GetSession(out, sessionIndex);
			session.summary.SetNum(lineCount);
			for (uint32 i = 0; i != lineCount; ++i)
			{
				const FString Output = reader.ReadString();
				session.summary[i] = Output;
			}
			break;
		}
		case TraceType_ProcessAdded:
		{
			const uint32 sessionIndex = reader.ReadU32();
			const uint32 id = reader.ReadU32();
			TStringBuilderWithBuffer<UBA_CHAR, 512> desc;
			reader.ReadString(desc);

			if (out.version < 15)
			{
				time = reader.Read7BitEncoded();
				if (time > maxTime)
				{
					reader.SetPosition(readPos);
					return true;
				}
			}

			ProcessBegin(out, sessionIndex, id, time, desc.GetData());
			break;
		}
		case TraceType_ProcessExited:
		{
			const uint32 id = reader.ReadU32();
			const uint32 exitCode = reader.ReadU32();
			if (out.version < 15)
			{
				time = reader.Read7BitEncoded();
				if (time > maxTime)
				{
					reader.SetPosition(readPos);
					return true;
				}
			}

			uint32 sessionIndex;
			FTraceView::FProcess& process = *ProcessEnd(out, sessionIndex, id, time);

			process.exitCode = exitCode;

			ProcessStats processStats;
			SessionStats sessionStats;
			StorageStats storageStats;
			KernelStats kernelStats;

			const uint8* dataStart = reader.GetPositionData();
			processStats.Read(reader, out.version);

			check(process.isRemote == (sessionIndex != 0));

			if (process.isRemote)
			{
				if (out.version >= 7)
				{
					sessionStats.Read(reader, out.version);
					storageStats.Read(reader, out.version);
					kernelStats.Read(reader, out.version);
				}
			}
			else if (out.version >= 30)
			{
				storageStats.Read(reader, out.version);
				kernelStats.Read(reader, out.version);
			}
			const uint8* dataEnd = reader.GetPositionData();
			process.stats.SetNum(dataEnd - dataStart);
			memcpy(process.stats.GetData(), dataStart, dataEnd - dataStart);

			if (out.version >= 34)
			{
				process.breadcrumbs = reader.ReadString();
			}

			process.createFilesTime = processStats.createFile.time;
			process.writeFilesTime = FMath::Max(processStats.writeFiles.time.load(), processStats.sendFiles.time.load());

			if (out.version >= 22)
			{
				while (true)
				{
					const auto type = (ELogEntryType)reader.ReadByte();
					if (type == 255)
						break;
					process.logLines.Add(FPocessLogLine(reader.ReadString(), type));
				}
			}
			else if (out.version >= 20)
			{
				uint64 logLineCount = reader.Read7BitEncoded();
				if (logLineCount >= 101)
					logLineCount = 101;
				process.logLines.SetNum(logLineCount);
				while (logLineCount--)
				{
					const auto type = (ELogEntryType)reader.ReadByte();
					process.logLines.Add(FPocessLogLine(reader.ReadString(), type));
				}
			}

			break;
		}
		case TraceType_ProcessEnvironmentUpdated:
		{
			const uint32 processId = reader.ReadU32();
			auto findIt = m_activeProcesses.find(processId);
			if (findIt == m_activeProcesses.end())
				return false;
			TStringBuilderWithBuffer<UBA_CHAR, 512> reason;
			reader.ReadString(reason);
			if (out.version < 15)
				time = reader.Read7BitEncoded();
			FTraceView::FProcessLocation active = findIt->second;
			auto& session = GetSession(out, active.sessionIndex);

			auto& processes = session.processors[active.processorIndex].processes;
			FTraceView::FProcess& process = processes[active.processIndex];

			const uint8* dataStart = reader.GetPositionData();

			ProcessStats processStats;
			SessionStats sessionStats;
			StorageStats storageStats;
			KernelStats kernelStats;
			processStats.Read(reader, out.version);
			sessionStats.Read(reader, out.version);
			storageStats.Read(reader, out.version);
			kernelStats.Read(reader, out.version);

			const uint8* dataEnd = reader.GetPositionData();
			process.stats.SetNum(dataEnd - dataStart);
			memcpy(process.stats.GetData(), dataStart, dataEnd - dataStart);

			process.exitCode = 0u;
			process.stop = time;
			process.bitmapDirty = true;
			bool isRemote = process.isRemote;

			processes.AddDefaulted();
			auto& process2 = processes.Last();
			m_activeProcesses[processId].processIndex = uint32(processes.Num() - 1);
			process2.id = processId;
			process2.description = reason.GetData();
			process2.start = time;
			process2.stop = ~uint64(0);
			process2.exitCode = ~0u;
			process2.isRemote = isRemote;

			++session.processExitedCount;
			++out.totalProcessExitedCount;
			break;
		}
		case TraceType_ProcessReturned:
		{
			const uint32 id = reader.ReadU32();
			if (out.version < 15)
			{
				time = reader.Read7BitEncoded();
				if (time > maxTime)
				{
					reader.SetPosition(readPos);
					return true;
				}
			}
			FString reason;
			if (out.version >= 33)
				reason = reader.ReadString();
			if (reason.IsEmpty())
				reason = TEXT("Unknown");

			auto findIt = m_activeProcesses.find(id);
			if (findIt == m_activeProcesses.end())
				return false;
			FTraceView::FProcessLocation active = findIt->second;
			m_activeProcesses.erase(findIt);

			auto& session = GetSession(out, active.sessionIndex);
			--session.processActiveCount;
			--out.totalProcessActiveCount;

			FTraceView::FProcess& process = session.processors[active.processorIndex].processes[active.processIndex];
			process.exitCode = 0;
			process.stop = time;
			process.returnedReason = reason;
			process.bitmapDirty = true;
			break;
		}
		case TraceType_FileBeginFetch:
		{
			const Guid clientUid = ReadClientId(out, reader);
			const FCasKey key = reader.ReadCasKey();
			const uint64 size = reader.Read7BitEncoded();
			TStringBuilderWithBuffer<UBA_CHAR, 512> temp;
			const UBA_CHAR* hint;
			if (out.version < 14)
			{
				reader.ReadString(temp);
				hint = temp.GetData();
			}
			else
				hint = out.strings[reader.Read7BitEncoded()].GetCharArray().GetData();

			if (out.version < 15)
			{
				time = reader.Read7BitEncoded();
				if (time > maxTime)
				{
					reader.SetPosition(readPos);
					return true;
				}
			}

			if (auto session = GetSession(out, clientUid))
			{
				session->fetchedFiles.Add({ key, size, hint, time, ~uint64(0) });
				session->fetchedFilesBytes += size;
			}
			break;
		}
		case TraceType_FileFetchLight:
		{
			const Guid clientUid = ReadClientId(out, reader);
			const uint64 fileSize = reader.Read7BitEncoded();
			if (auto session = GetSession(out, clientUid))
			{
				session->fetchedFiles.Add({ CasKeyZero, fileSize, TEXT(""), time, time });
				session->fetchedFilesBytes += fileSize;
			}
			break;
		}
		case TraceType_ProxyCreated:
		{
			const Guid clientUid = ReadClientId(out, reader);
			TStringBuilderWithBuffer<UBA_CHAR, 512> proxyName;
			reader.ReadString(proxyName);
			if (out.version < 15)
			{
				time = reader.Read7BitEncoded();
				if (time > maxTime)
				{
					reader.SetPosition(readPos);
					return true;
				}
			}

			if (auto session = GetSession(out, clientUid))
			{
				session->proxyName = proxyName.GetData();
				session->proxyCreated = true;
			}
			break;
		}
		case TraceType_ProxyUsed:
		{
			const Guid clientUid = ReadClientId(out, reader);
			TStringBuilderWithBuffer<UBA_CHAR, 512> proxyName;
			reader.ReadString(proxyName);

			if (out.version < 15)
			{
				time = reader.Read7BitEncoded();
				if (time > maxTime)
				{
					reader.SetPosition(readPos);
					return true;
				}
			}

			if (auto session = GetSession(out, clientUid))
				session->proxyName = proxyName.GetData();
			break;
		}
		case TraceType_FileEndFetch:
		{
			const Guid clientUid = ReadClientId(out, reader);
			FCasKey key = reader.ReadCasKey();

			if (out.version < 15)
			{
				time = reader.Read7BitEncoded();
				if (time > maxTime)
				{
					reader.SetPosition(readPos);
					return true;
				}
			}

			if (auto session = GetSession(out, clientUid))
			{
				// for (auto rit = session->fetchedFiles.rbegin(), rend = session->fetchedFiles.rend(); rit != rend; ++rit)
				for(int i = session->fetchedFiles.Num() -1; i >= 0; --i)
				{
					auto& rit = session->fetchedFiles[i];
					if (rit.key != key)
						continue;
					rit.stop = time;
					break;
				}
			}
			break;
		}
		case TraceType_FileBeginStore:
		{
			const Guid clientUid = ReadClientId(out, reader);
			const FCasKey key = reader.ReadCasKey();
			const uint64 size = reader.Read7BitEncoded();
			TStringBuilderWithBuffer<UBA_CHAR, 512> temp;
			const UBA_CHAR* hint;
			if (out.version < 14)
			{
				reader.ReadString(temp);
				hint = temp.GetData();
			}
			else
				hint = out.strings[reader.Read7BitEncoded()].GetCharArray().GetData();
			if (out.version < 15)
			{
				time = reader.Read7BitEncoded();
				if (time > maxTime)
				{
					reader.SetPosition(readPos);
					return true;
				}
			}

			if (auto session = GetSession(out, clientUid))
			{
				session->storedFiles.Add({ key, size, hint, time, ~uint64(0) });
				session->storedFilesBytes += size;
			}
			break;
		}
		case TraceType_FileStoreLight:
		{
			const Guid clientUid = ReadClientId(out, reader);
			const uint64 fileSize = reader.Read7BitEncoded();
			if (auto session = GetSession(out, clientUid))
			{
				session->storedFiles.Add({ CasKeyZero, fileSize, TEXT(""), time, time });
				session->storedFilesBytes += fileSize;
			}
			break;
		}
		case TraceType_FileEndStore:
		{
			const Guid clientUid = ReadClientId(out, reader);
			const FCasKey key = reader.ReadCasKey();

			if (out.version < 15)
			{
				time = reader.Read7BitEncoded();
				if (time > maxTime)
				{
					reader.SetPosition(readPos);
					return true;
				}
			}

			if (auto session = GetSession(out, clientUid))
			{
				// for (auto rit = session->storedFiles.rbegin(), rend = session->storedFiles.rend(); rit != rend; ++rit)
				for (int i = session->storedFiles.Num() - 1; i >= 0; --i)
				{
					auto& rit = session->storedFiles[i];
					if (rit.key != key)
						continue;
					rit.stop = time;
					break;
				}
			}
			break;
		}
		case TraceType_Summary:
		{
			if (out.version < 15)
				time = reader.Read7BitEncoded();
			StopAllActive(out, time);
			return false;
		}
		case TraceType_BeginWork:
		{
			uint32 workIndex;
			if (out.version < 14)
				workIndex = reader.ReadU32();
			else
				workIndex = uint32(reader.Read7BitEncoded());

			uint32 trackIndex = 0;
			FTraceView::FWorkTrack* workTrack = nullptr;
			for (uint32 i = 0, e = uint32(out.workTracks.Num()); i != e; ++i)
			{
				auto& it = out.workTracks[i];
				if (it.records.IsEmpty() || it.records.Last().stop == ~uint64(0))
				{
					continue;
				}
				trackIndex = i;
				workTrack = &it;
				break;
			}
			if (!workTrack)
			{
				trackIndex = uint32(out.workTracks.Num());
				out.workTracks.AddDefaulted();
				workTrack = &out.workTracks.Last();
			}

			workTrack->records.AddDefaulted();
			auto& record = workTrack->records.Last();

			m_activeWorkRecords.try_emplace(workIndex, FWorkRecordLocation{ trackIndex, uint32(workTrack->records.Num() - 1) });

			uint64 stringIndex;
			if (out.version < 14)
				stringIndex = reader.ReadU32();
			else
				stringIndex = reader.Read7BitEncoded();

			record.description = out.strings[stringIndex].GetCharArray().GetData();
			if (out.version < 15)
				record.start = reader.Read7BitEncoded();
			else
				record.start = time;
			record.stop = ~uint64(0);
			break;
		}
		case TraceType_EndWork:
		{
			uint32 workIndex;
			if (out.version < 14)
				workIndex = reader.ReadU32();
			else
				workIndex = uint32(reader.Read7BitEncoded());

			auto findIt = m_activeWorkRecords.find(workIndex);
			if (findIt == m_activeWorkRecords.end())
				return false;
			FWorkRecordLocation active = findIt->second;
			m_activeWorkRecords.erase(findIt);

			FTraceView::FWorkRecord& record = out.workTracks[active.track].records[active.index];
			if (out.version < 15)
				record.stop = reader.Read7BitEncoded();
			else
				record.stop = time;
			break;
		}
		case TraceType_ProgressUpdate:
		{
			out.progressProcessesTotal = uint32(reader.Read7BitEncoded());
			out.progressProcessesDone = uint32(reader.Read7BitEncoded());
			out.progressErrorCount = uint32(reader.Read7BitEncoded());
			break;
		}
		case TraceType_StatusUpdate:
		{
			if (out.version < 32)
			{
				reader.Read7BitEncoded();
				reader.Read7BitEncoded();
				reader.ReadString();
				reader.Read7BitEncoded();
				reader.ReadString();
				reader.ReadByte();
			}
			else
			{
				const uint64 row = reader.Read7BitEncoded();
				const uint64 column = reader.Read7BitEncoded();
				const uint64 key = (row << 32) | column;
				
				Xiao::FTraceView::FStatusUpdate status;
				status.text = reader.ReadString();
				status.type = (ELogEntryType)reader.ReadByte();
				status.link = reader.ReadString();
				out.statusMap.Add(key, status);
			}

			break;
		}
		case TraceType_RemoteExecutionDisabled:
		{
			out.remoteExecutionDisabled = true;
			break;
		}
		case TraceType_String:
		{
			out.strings.Add(reader.ReadString());
			break;
		}
		case TraceType_CacheBeginFetch:
		{
			const uint32 id = uint32(reader.Read7BitEncoded());
			TStringBuilderWithBuffer<UBA_CHAR, 512> desc;
			reader.ReadString(desc);
			ProcessBegin(out, 0, id, time, desc.GetData())->cacheFetch = true;
			break;
		}
		case TraceType_CacheEndFetch:
		{
			const uint32 id = uint32(reader.Read7BitEncoded());
			bool success = reader.ReadBool();

			uint32 sessionIndex;
			FTraceView::FProcess& process = *ProcessEnd(out, sessionIndex, id, time);
			process.exitCode = 0;//success ? 0 : -1;
			if (!success)
				process.returnedReason = TEXT("M");

			CacheStats cacheStats;
			KernelStats kernelStats;
			StorageStats storageStats;
			const uint8* dataStart = reader.GetPositionData();
			cacheStats.Read(reader, out.version);
			if (success || out.version >= 29)
			{
				storageStats.Read(reader, out.version);
				kernelStats.Read(reader, out.version);
			}
			const uint8* dataEnd = reader.GetPositionData();
			process.stats.SetNum(dataEnd - dataStart);
			memcpy(process.stats.GetData(), dataStart, dataEnd - dataStart);
			break;
		}
		case TraceType_CacheBeginWrite:
		{
			const uint32 processId = uint32(reader.Read7BitEncoded());
			out.cacheWrites[processId].start = time;
			break;
		}
		case TraceType_CacheEndWrite:
		{
			const uint32 processId = uint32(reader.Read7BitEncoded());
			FTraceView::FCacheWrite& write = out.cacheWrites[processId];
			write.success = reader.ReadBool();
			write.bytesSent = reader.Read7BitEncoded();
			write.end = time;
		}
		}
		return true;
	}

	void FTraceReader::StopAllActive(FTraceView& out, const uint64 stopTime)
	{
		for (auto& pair : m_activeProcesses)
		{
			auto& active = pair.second;
			FTraceView::FProcess& process = GetSession(out, active.sessionIndex).processors[active.processorIndex].processes[active.processIndex];
			process.exitCode = ~0u;
			process.stop = stopTime;
			process.bitmapDirty = true;
		}

		m_activeProcesses.clear();
		out.finished = true;
	}

	void FTraceReader::Reset(FTraceView& out)
	{
		out.Clear();
		m_activeProcesses.clear();
		m_activeWorkRecords.clear();
		m_sessionIndexToSession.Empty();
	}

	void FTraceReader::Unmap()
	{
		if (m_hostProcess.IsValid())
			FPlatformProcess::CloseProc(m_hostProcess);

		m_memoryBegin = nullptr;

		if (m_mappedFile.IsValid())
		{
			m_mappedFile = nullptr;
		}

		if (m_fileHandle.IsValid())
		{
			m_fileHandle = nullptr;
		}

		if (m_sharedMemeoryRegion.IsValid())
		{
			m_sharedMemeoryRegion = nullptr;
		}

		m_namedTrace.Empty();
	}

	Guid FTraceReader::ReadClientId(FTraceView& out, FBinaryReader& reader)
	{
		if (out.version < 15)
			return reader.ReadGuid();
		Guid clientUid;
		clientUid.data1 = uint32(reader.Read7BitEncoded());
		return clientUid;
	}

	FTraceView::FSession& FTraceReader::GetSession(FTraceView& out, const uint32 sessionIndex)
	{
		return out.sessions[m_sessionIndexToSession[sessionIndex]];
	}

	FTraceView::FSession* FTraceReader::GetSession(FTraceView& out, const Guid& clientUid)
	{
		for (auto& session : out.sessions)
			if (session.clientUid == clientUid)
				return &session;
		// First file can be retrieved here before session is connected... haven't figured out how this can happen but let's ignore that for now :-)
		//UBA_ASSERTF(false, L"Failed to get session for clientUid %ls", GuidToString(clientUid).str);
		return nullptr;
	}

	FTraceView::FProcess* FTraceReader::ProcessBegin(FTraceView& out, const uint32 sessionIndex, const uint32 id, const uint64 time, const TCHAR* description)
	{
		FTraceView::FProcessor* processor = nullptr;
		FTraceView::FSession& session = GetSession(out, sessionIndex);

		uint32 processorIndex = 0;
		for (uint32 i = 0, e = uint32(session.processors.Num()); i != e; ++i)
		{
			auto& it = session.processors[i];
			if (it.processes.IsEmpty() || it.processes.Last().stop == ~uint64(0))
				continue;
			processorIndex = i;
			processor = &it;
			break;
		}
		if (!processor)
		{
			processorIndex = uint32(session.processors.Num());
			session.processors.AddDefaulted();
			processor = &session.processors.Last();
		}

		processor->processes.AddDefaulted();
		auto& process = processor->processes.Last();

		m_activeProcesses.try_emplace(id, FTraceView::FProcessLocation{ sessionIndex, processorIndex, uint32(processor->processes.Num() - 1) });

		++session.processActiveCount;
		++out.totalProcessActiveCount;

		process.id = id;
		process.description = description;
		process.start = time;
		process.stop = ~uint64(0);
		process.exitCode = ~0u;
		process.isRemote = sessionIndex != 0;
		return &process;
	}

	FTraceView::FProcess* FTraceReader::ProcessEnd(FTraceView& out, uint32& outSessionIndex, const uint32 id, const uint64 time)
	{
		auto findIt = m_activeProcesses.find(id);
		if (findIt == m_activeProcesses.end())
			return nullptr;
		FTraceView::FProcessLocation active = findIt->second;
		m_activeProcesses.erase(findIt);
		outSessionIndex = active.sessionIndex;
		auto& session = GetSession(out, active.sessionIndex);
		++session.processExitedCount;
		--session.processActiveCount;

		++out.totalProcessExitedCount;
		--out.totalProcessActiveCount;

		FTraceView::FProcess& process = session.processors[active.processorIndex].processes[active.processIndex];

		process.stop = time;
		process.bitmapDirty = true;
		return &process;
	}
}

#undef UBA_CHAR