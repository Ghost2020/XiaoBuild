#pragma once

#include "UbaTraceReader.h"
#include "Insights/ViewModels/TooltipDrawState.h"

namespace Xiao
{
	struct FTimer
	{
		FTimer(FTimer& o) { time.store(o.time.load()); count.store(o.count.load()); }
		FTimer() = default;
		FTimer(uint64 t, uint32 c) : time(t), count(c) {}
		FTimer(FTimer&& o) noexcept : time(o.time.load()), count(o.count.load()) {}
		void operator=(const FTimer& o) { time.store(o.time.load()); count.store(o.count.load()); }
		std::atomic<uint64> time;
		std::atomic<uint32> count;
		void operator+=(const FTimer& o) { time += o.time; count += o.count; }
		bool operator==(const FTimer& o) const { return time == o.time && count == o.count; }
	};

	struct AtomicU64 : std::atomic<uint64>
	{
		AtomicU64(uint64 initialValue = 0) : std::atomic<uint64>(initialValue) {}
		AtomicU64(AtomicU64&& o) noexcept : std::atomic<uint64>(o.load()) {}
		void operator=(uint64 o) { store(o); }
		void operator=(const AtomicU64& o) { store(o); }
	};

	struct FExtendedTimer : FTimer
	{
		AtomicU64 longest;
	};

	struct FTimeAndBytes : FExtendedTimer
	{
		void operator+=(const FTimeAndBytes& o) { time += o.time; count += o.count; bytes += o.bytes; }
		AtomicU64 bytes;
	};

	const TCHAR EmptyCharArray[31] = TEXT("                   ");
	
	template<typename T>
	void LogStat(FTooltipDrawState& InToolTip, const char* name, const T&, const uint64 frequency) 
	{}

	static void LogStat(FTooltipDrawState& InToolTip, const char* name, const FTimer& timer, const uint64 frequency)
	{
		if (!timer.count)
			return;
		const FString NameStr = UTF8_TO_TCHAR(name);
		InToolTip.AddTextLine(FString::Printf(TEXT("  %c%s %s %8u %9s"), FChar::ToUpper(NameStr[0]), *NameStr.Mid(1), EmptyCharArray + NameStr.Len() + 1, timer.count.load(), *FPlatformTime::PrettyTime(FPlatformTime::ToSeconds64(timer.time))), FLinearColor::White);
	}

	static void LogStat(FTooltipDrawState& InToolTip, const char* name, const FExtendedTimer& timer, const uint64 frequency)
	{
		LogStat(InToolTip, name, (const FTimer&)timer, frequency);
	}

	static void LogStat(FTooltipDrawState& InToolTip, const char* name, const FTimeAndBytes& timer, const uint64 frequency)
	{
		if (!timer.count)
			return;
		const FString NameStr = UTF8_TO_TCHAR(name);
		InToolTip.AddTextLine(FString::Printf(TEXT("  %c%s %s %8u %9s"), FChar::ToUpper(NameStr[0]), *NameStr.Mid(1), EmptyCharArray + NameStr.Len() + 1, timer.count.load(), *FPlatformTime::PrettyTime(FPlatformTime::ToSeconds64(timer.time))), FLinearColor::White);
		if (timer.bytes)
			InToolTip.AddTextLine(FString::Printf(TEXT("     Bytes                    %9s"), *BytesToText(timer.bytes)), FLinearColor::White);
	}

	#define UBA_TRACE_TYPES \
		UBA_TRACE_TYPE(SessionAdded) \
		UBA_TRACE_TYPE(SessionUpdate) \
		UBA_TRACE_TYPE(ProcessAdded) \
		UBA_TRACE_TYPE(ProcessExited) \
		UBA_TRACE_TYPE(ProcessReturned) \
		UBA_TRACE_TYPE(FileBeginFetch) \
		UBA_TRACE_TYPE(FileEndFetch) \
		UBA_TRACE_TYPE(FileBeginStore) \
		UBA_TRACE_TYPE(FileEndStore) \
		UBA_TRACE_TYPE(Summary) \
		UBA_TRACE_TYPE(BeginWork) \
		UBA_TRACE_TYPE(EndWork) \
		UBA_TRACE_TYPE(String) \
		UBA_TRACE_TYPE(SessionSummary) \
		UBA_TRACE_TYPE(ProcessEnvironmentUpdated) \
		UBA_TRACE_TYPE(SessionDisconnect) \
		UBA_TRACE_TYPE(ProxyCreated) \
		UBA_TRACE_TYPE(ProxyUsed) \
		UBA_TRACE_TYPE(FileFetchLight) \
		UBA_TRACE_TYPE(FileStoreLight) \
		UBA_TRACE_TYPE(StatusUpdate) \
		UBA_TRACE_TYPE(SessionNotification) \
		UBA_TRACE_TYPE(CacheBeginFetch) \
		UBA_TRACE_TYPE(CacheEndFetch) \
		UBA_TRACE_TYPE(CacheBeginWrite) \
		UBA_TRACE_TYPE(CacheEndWrite) \
		UBA_TRACE_TYPE(ProgressUpdate) \
		UBA_TRACE_TYPE(RemoteExecutionDisabled) \
		UBA_TRACE_TYPE(FileFetchSize) \
		UBA_TRACE_TYPE(ProcessBreadcrumbs) \
		UBA_TRACE_TYPE(WorkHint) \
		UBA_TRACE_TYPE(DriveUpdate) \

	enum TraceType : uint8
	{
		#define UBA_TRACE_TYPE(name) TraceType_##name,
		UBA_TRACE_TYPES
		#undef UBA_TRACE_TYPE
	};

	static void Read(FBinaryReader& reader, uint64& v, const uint32 version)
	{
		v = reader.Read7BitEncoded();
	}
	static void Read(FBinaryReader& reader, uint32& v, const uint32 version)
	{
		v = uint32(reader.Read7BitEncoded());
	}
	static void Read(FBinaryReader& reader, FTimer& timer, const uint32 version)
	{
		timer.time = reader.Read7BitEncoded();
		timer.count = (uint32)reader.Read7BitEncoded();
	}
	static void Read(FBinaryReader& reader, AtomicU64& v, const uint32 version)
	{
		v = reader.Read7BitEncoded();
	}
	static void Read(FBinaryReader& reader, FTimeAndBytes& timer, const uint32 version)
	{
		timer.time = reader.Read7BitEncoded();
		timer.count = (uint32)reader.Read7BitEncoded();
		if (version >= 30)
			timer.bytes = reader.Read7BitEncoded();
	}

	struct ProcessStats
	{
		FTimer waitOnResponse;

		#define UBA_PROCESS_STATS \
			UBA_PROCESS_STAT(attach, 0) \
			UBA_PROCESS_STAT(detach, 0) \
			UBA_PROCESS_STAT(init, 0) \
			UBA_PROCESS_STAT(createFile, 0) \
			UBA_PROCESS_STAT(closeFile, 0) \
			UBA_PROCESS_STAT(getFullFileName, 0) \
			UBA_PROCESS_STAT(deleteFile, 0) \
			UBA_PROCESS_STAT(moveFile, 0) \
			UBA_PROCESS_STAT(chmod, 17) \
			UBA_PROCESS_STAT(copyFile, 0) \
			UBA_PROCESS_STAT(createProcess, 0) \
			UBA_PROCESS_STAT(updateTables, 0) \
			UBA_PROCESS_STAT(listDirectory, 0) \
			UBA_PROCESS_STAT(createTempFile, 0) \
			UBA_PROCESS_STAT(openTempFile, 0) \
			UBA_PROCESS_STAT(virtualAllocFailed, 0) \
			UBA_PROCESS_STAT(log, 0) \
			UBA_PROCESS_STAT(sendFiles, 0) \
			UBA_PROCESS_STAT(writeFiles, 19) \
			UBA_PROCESS_STAT(queryCache, 24) \
			UBA_PROCESS_STAT(waitDecompress, 30) \
			UBA_PROCESS_STAT(preparseObjFiles, 30) \
			UBA_PROCESS_STAT(fileTable, 30) \
			UBA_PROCESS_STAT(dirTable, 30) \
			UBA_PROCESS_STAT(longPathName, 31) \


		#define UBA_PROCESS_STAT(T, ver) FTimer T;
		UBA_PROCESS_STATS
		#undef UBA_PROCESS_STAT

		enum
		{
			#define UBA_PROCESS_STAT(var, ver) Bit_##var,
			UBA_PROCESS_STATS
			#undef UBA_PROCESS_STAT
		};

		AtomicU64 startupTime;
		AtomicU64 exitTime;

		// Don't add in GetTotalTime()
		AtomicU64 wallTime;
		AtomicU64 cpuTime;

		AtomicU64 detoursMemory;
		AtomicU64 peakMemory;

		AtomicU64 iopsRead;
		AtomicU64 iopsWrite;
		AtomicU64 iopsOther;

		AtomicU64 usedMemory;

		AtomicU64 hostTotalTime;

		void Read(FBinaryReader& reader, const uint32 version)
		{
			Xiao::Read(reader, waitOnResponse, version);

			if (version < 30)
			{
				#define UBA_PROCESS_STAT(T, ver) if (ver <= version) Xiao::Read(reader, T, version);
				UBA_PROCESS_STATS
				#undef UBA_PROCESS_STAT
			}
			else
			{
				uint64 bits = reader.Read7BitEncoded();
				#define UBA_PROCESS_STAT(var, ver) if (bits & (1 << Bit_##var)) Xiao::Read(reader, var, version);
				UBA_PROCESS_STATS
				#undef UBA_PROCESS_STAT
			}

			if (version >= 37)
			{
				startupTime = reader.Read7BitEncoded();
				exitTime = reader.Read7BitEncoded();
				wallTime = reader.Read7BitEncoded();
				cpuTime = reader.Read7BitEncoded();
				detoursMemory = reader.Read7BitEncoded();
				peakMemory = reader.Read7BitEncoded();
				if (version >= 39)
				{
					iopsRead = reader.Read7BitEncoded();
					iopsWrite = reader.Read7BitEncoded();
					iopsOther = reader.Read7BitEncoded();
				}
				hostTotalTime = reader.Read7BitEncoded();
			}
			else
			{
				startupTime = reader.ReadU64();
				exitTime = reader.ReadU64();
				wallTime = reader.ReadU64();
				cpuTime = reader.ReadU64();
				detoursMemory = reader.ReadU32();
				hostTotalTime = reader.ReadU64();
			}
		}

		uint64 GetTotalTime() const
		{
			return
			#define UBA_PROCESS_STAT(T, ver) + T.time
				UBA_PROCESS_STATS
			#undef UBA_PROCESS_STAT
				;
		}

		uint32 GetTotalCount() const
		{
			return
			#define UBA_PROCESS_STAT(T, ver) + T.count
				UBA_PROCESS_STATS
			#undef UBA_PROCESS_STAT
				;
		}

		void Print(FTooltipDrawState& InToolTip, const uint64 frequency) const
		{
			InToolTip.AddTextLine(FString::Printf(TEXT("  Total              %8u %9s"), GetTotalCount(), *FPlatformTime::PrettyTime(FPlatformTime::ToSeconds64(GetTotalTime()))), FLinearColor::White);
			InToolTip.AddTextLine(FString::Printf(TEXT("  WaitOnResponse     %8u %9s"), waitOnResponse.count.load(), *FPlatformTime::PrettyTime(FPlatformTime::ToSeconds64(waitOnResponse.time))), FLinearColor::White);
			InToolTip.AddTextLine(FString::Printf(TEXT("  Host                %17s"), *FPlatformTime::PrettyTime(FPlatformTime::ToSeconds64(hostTotalTime))), FLinearColor::White);
			InToolTip.AddTextLine(TEXT(""), FLinearColor::Transparent);

			struct Stat { const char* name; uint64 nameLen; const FTimer& timer; };
			Stat stats[] =
			{
				#define UBA_PROCESS_STAT(T, ver) { #T, sizeof(#T), T },
				UBA_PROCESS_STATS
				#undef UBA_PROCESS_STAT
			};

			for (Stat& s : stats)
				if (s.timer.count)
					InToolTip.AddTextLine(FString::Printf(TEXT("  %c%hs %s %8u %9s"), FChar::ToUpper(s.name[0]), s.name + 1, EmptyCharArray + s.nameLen, s.timer.count.load(), *FPlatformTime::PrettyTime(FPlatformTime::ToSeconds64(s.timer.time))), FLinearColor::White);

			InToolTip.AddTextLine(TEXT(""), FLinearColor::Transparent);

			InToolTip.AddTextLine(FString::Printf(TEXT("  HighestMem                  %9s"), *BytesToText(usedMemory)), FLinearColor::White);
			InToolTip.AddTextLine(FString::Printf(TEXT("  Startup Time                %9s"), *FPlatformTime::PrettyTime(FPlatformTime::ToSeconds64(startupTime))), FLinearColor::White);
			InToolTip.AddTextLine(FString::Printf(TEXT("  Exit Time                   %9s"), *FPlatformTime::PrettyTime(FPlatformTime::ToSeconds64(exitTime))), FLinearColor::White);
			InToolTip.AddTextLine(FString::Printf(TEXT("  CPU Time                    %9s"), *FPlatformTime::PrettyTime(FPlatformTime::ToSeconds64(cpuTime))), FLinearColor::White);
			InToolTip.AddTextLine(FString::Printf(TEXT("  Wall Time                   %9s"), *FPlatformTime::PrettyTime(FPlatformTime::ToSeconds64(wallTime))), FLinearColor::White);
		}
	};

	struct SessionStats
	{
		#define UBA_SESSION_STATS \
			UBA_SESSION_STAT(FTimer, getFileMsg, 0) \
			UBA_SESSION_STAT(FTimer, getBinaryMsg, 0) \
			UBA_SESSION_STAT(FTimer, sendFileMsg, 0) \
			UBA_SESSION_STAT(FTimer, listDirMsg, 0) \
			UBA_SESSION_STAT(FTimer, getDirsMsg, 0) \
			UBA_SESSION_STAT(FTimer, getHashesMsg, 8) \
			UBA_SESSION_STAT(FTimer, deleteFileMsg, 0) \
			UBA_SESSION_STAT(FTimer, copyFileMsg, 16) \
			UBA_SESSION_STAT(FTimer, createDirMsg, 0) \
			UBA_SESSION_STAT(FTimer, waitGetFileMsg, 10) \
			UBA_SESSION_STAT(FTimer, createMmapFromFile, 12) \
			UBA_SESSION_STAT(FTimer, waitMmapFromFile, 12) \
			UBA_SESSION_STAT(FTimer, getLongNameMsg, 31) \
			UBA_SESSION_STAT(FTimer, waitBottleneck, 40) \

		#define UBA_SESSION_STAT(type, var, ver) type var;
		UBA_SESSION_STATS
		#undef UBA_SESSION_STAT

		void Read(FBinaryReader& reader, const uint32 version)
		{
			if (version < 30)
			{
				#define UBA_SESSION_STAT(type, var, ver) if (ver <= version) Xiao::Read(reader, var, version);
				UBA_SESSION_STATS
				#undef UBA_SESSION_STAT
				return;
			}

			const uint16 bits = reader.ReadU16();
			#define UBA_SESSION_STAT(type, var, ver) if (bits & (1 << Bit_##var)) Xiao::Read(reader, var, version);
			UBA_SESSION_STATS
			#undef UBA_SESSION_STAT
		};

		enum
		{
			#define UBA_SESSION_STAT(type, var, ver) Bit_##var,
			UBA_SESSION_STATS
			#undef UBA_SESSION_STAT
		};

		void Print(FTooltipDrawState& InToolTip, const uint64 frequency) const
		{
			#define UBA_SESSION_STAT(type, var, ver) LogStat(InToolTip, #var, var, frequency);
			UBA_SESSION_STATS
			#undef UBA_SESSION_STAT
		}

		bool IsEmpty() const
		{
			#define UBA_SESSION_STAT(type, var, ver) if (var.count) return false;
			UBA_SESSION_STATS
			#undef UBA_SESSION_STAT
			return true;
		}
	};

	struct StorageStats
	{
		#define UBA_STORAGE_STATS \
			UBA_STORAGE_STAT(FTimer, calculateCasKey, 0) \
			UBA_STORAGE_STAT(FTimer, copyOrLink, 0) \
			UBA_STORAGE_STAT(FTimer, copyOrLinkWait, 0) \
			UBA_STORAGE_STAT(FTimer, ensureCas, 0) \
			UBA_STORAGE_STAT(FTimer, sendCas, 0) \
			UBA_STORAGE_STAT(FTimer, recvCas, 0) \
			UBA_STORAGE_STAT(FTimer, compressWrite, 0) \
			UBA_STORAGE_STAT(FTimer, compressSend, 0) \
			UBA_STORAGE_STAT(FTimer, decompressRecv, 0) \
			UBA_STORAGE_STAT(FTimer, decompressToMem, 0) \
			UBA_STORAGE_STAT(FTimer, memoryCopy, 30) \
			UBA_STORAGE_STAT(FTimer, handleOverflow, 0) \
			UBA_STORAGE_STAT(AtomicU64, sendCasBytesRaw, 0) \
			UBA_STORAGE_STAT(AtomicU64, sendCasBytesComp, 0) \
			UBA_STORAGE_STAT(AtomicU64, recvCasBytesRaw, 0) \
			UBA_STORAGE_STAT(AtomicU64, recvCasBytesComp, 0) \
			UBA_STORAGE_STAT(FTimer, createCas, 0) \
			UBA_STORAGE_STAT(AtomicU64, createCasBytesRaw, 0) \
			UBA_STORAGE_STAT(AtomicU64, createCasBytesComp, 0) \

		#define UBA_STORAGE_STAT(type, var, ver) type var;
		UBA_STORAGE_STATS
		#undef UBA_STORAGE_STAT

		void Read(FBinaryReader& reader, const uint32 version)
		{
			if (version < 30)
			{
				#define UBA_STORAGE_STAT(type, var, ver) if (ver <= version) Xiao::Read(reader, var, version);
				UBA_STORAGE_STATS
				#undef UBA_STORAGE_STAT
				return;
			}

			uint64 bits = reader.Read7BitEncoded();
			#define UBA_STORAGE_STAT(type, var, ver) if (bits & (1 << Bit_##var)) Xiao::Read(reader, var, version);
			UBA_STORAGE_STATS
			#undef UBA_STORAGE_STAT
		}

		enum
		{
			#define UBA_STORAGE_STAT(type, var, ver) Bit_##var,
			UBA_STORAGE_STATS
			#undef UBA_STORAGE_STAT
		};

		void Print(FTooltipDrawState& InToolTip, const uint64 frequency) const
		{
			if (calculateCasKey.count)
				InToolTip.AddTextLine(FString::Printf(TEXT("  CalculateCasKeys     %6u %9s"), calculateCasKey.count.load(), *FPlatformTime::PrettyTime(FPlatformTime::ToSeconds64(calculateCasKey.time))), FLinearColor::White);
			if (ensureCas.count)
				InToolTip.AddTextLine(FString::Printf(TEXT("  EnsureCas            %6u %9s"), ensureCas.count.load(), *FPlatformTime::PrettyTime(FPlatformTime::ToSeconds64(ensureCas.time))), FLinearColor::White);
			if (recvCas.count)
			{
				InToolTip.AddTextLine(FString::Printf(TEXT("  ReceiveCas           %6u %9s"), recvCas.count.load(), *FPlatformTime::PrettyTime(FPlatformTime::ToSeconds64(recvCas.time))), FLinearColor::White);
				InToolTip.AddTextLine(FString::Printf(TEXT("     Bytes Raw/Comp %9s %9s"), *BytesToText(recvCasBytesRaw), *BytesToText(recvCasBytesComp)), FLinearColor::White);
				if (decompressRecv.count)
					InToolTip.AddTextLine(FString::Printf(TEXT("     Decompress        %6u %9s"), decompressRecv.count.load(), *FPlatformTime::PrettyTime(FPlatformTime::ToSeconds64(decompressRecv.time))), FLinearColor::White);
			}
			if (sendCas.count)
			{
				InToolTip.AddTextLine(FString::Printf(TEXT("  SendCas              %6u %9s"), sendCas.count.load(), *FPlatformTime::PrettyTime(FPlatformTime::ToSeconds64(sendCas.time))), FLinearColor::White);
				InToolTip.AddTextLine(FString::Printf(TEXT("     Bytes Raw/Comp %9s %9s"), *BytesToText(sendCasBytesRaw), *BytesToText(sendCasBytesComp)), FLinearColor::White);
				InToolTip.AddTextLine(FString::Printf(TEXT("     Compress          %6u %9s"), compressSend.count.load(), *FPlatformTime::PrettyTime(FPlatformTime::ToSeconds64(compressSend.time))), FLinearColor::White);
			}
			if (createCas.count)
			{
				InToolTip.AddTextLine(FString::Printf(TEXT("  CreateCas            %6u %9s"), createCas.count.load(), *FPlatformTime::PrettyTime(FPlatformTime::ToSeconds64(createCas.time))), FLinearColor::White);
				InToolTip.AddTextLine(FString::Printf(TEXT("     Bytes Raw/Comp %9s %9s"), *BytesToText(createCasBytesRaw), *BytesToText(createCasBytesComp)), FLinearColor::White);
				InToolTip.AddTextLine(FString::Printf(TEXT("     Compress          %6u %9s"), compressWrite.count.load(), *FPlatformTime::PrettyTime(FPlatformTime::ToSeconds64(compressWrite.time))), FLinearColor::White);
			}
			if (copyOrLink.count)
				InToolTip.AddTextLine(FString::Printf(TEXT("  CopyOrLink           %6u %9s"), copyOrLink.count.load(), *FPlatformTime::PrettyTime(FPlatformTime::ToSeconds64(copyOrLink.time))), FLinearColor::White);
			if (copyOrLinkWait.count)
				InToolTip.AddTextLine(FString::Printf(TEXT("  CopyOrLinkWait       %6u %9s"), copyOrLinkWait.count.load(), *FPlatformTime::PrettyTime(FPlatformTime::ToSeconds64(copyOrLinkWait.time))), FLinearColor::White);
			if (compressWrite.count)
				InToolTip.AddTextLine(FString::Printf(TEXT("  CompressToMem        %6u %9s"), compressWrite.count.load(), *FPlatformTime::PrettyTime(FPlatformTime::ToSeconds64(compressWrite.time))), FLinearColor::White);
			if (decompressToMem.count)
				InToolTip.AddTextLine(FString::Printf(TEXT("  DecompressToMem      %6u %9s"), decompressToMem.count.load(), *FPlatformTime::PrettyTime(FPlatformTime::ToSeconds64(decompressToMem.time))), FLinearColor::White);

			if (memoryCopy.count)
				InToolTip.AddTextLine(FString::Printf(TEXT("  MemoryCopy           %6u %9s"), memoryCopy.count.load(), *FPlatformTime::PrettyTime(FPlatformTime::ToSeconds64(memoryCopy.time))), FLinearColor::White);
		}

		bool IsEmpty() const
		{
			#define UBA_STORAGE_STAT(type, var, ver) if (var != type()) return false;
			UBA_STORAGE_STATS
			#undef UBA_STORAGE_STAT
			return true;
		}
	};

	struct KernelStats
	{
		#define UBA_KERNEL_STATS \
			UBA_KERNEL_STAT(FExtendedTimer, createFile, 0) \
			UBA_KERNEL_STAT(FExtendedTimer, closeFile, 0) \
			UBA_KERNEL_STAT(FTimeAndBytes, writeFile, 0) \
			UBA_KERNEL_STAT(FTimeAndBytes, memoryCopy, 30) \
			UBA_KERNEL_STAT(FTimeAndBytes, readFile, 0) \
			UBA_KERNEL_STAT(FExtendedTimer, setFileInfo, 0) \
			UBA_KERNEL_STAT(FExtendedTimer, getFileInfo, 29) \
			UBA_KERNEL_STAT(FExtendedTimer, createFileMapping, 0) \
			UBA_KERNEL_STAT(FExtendedTimer, mapViewOfFile, 0) \
			UBA_KERNEL_STAT(FExtendedTimer, unmapViewOfFile, 0) \
			UBA_KERNEL_STAT(FExtendedTimer, getFileTime, 0) \
			UBA_KERNEL_STAT(FExtendedTimer, closeHandle, 0) \
			UBA_KERNEL_STAT(FExtendedTimer, traverseDir, 27) \
			UBA_KERNEL_STAT(FExtendedTimer, virtualAlloc, 30) \
			UBA_KERNEL_STAT(FTimeAndBytes, memoryCompress, 41) \

		#define UBA_KERNEL_STAT(type, var, ver) type var;
		UBA_KERNEL_STATS
		#undef UBA_KERNEL_STAT

		void Read(FBinaryReader& reader, const uint32 version)
		{
			if (version < 30)
			{
				#define UBA_KERNEL_STAT(type, var, ver) if (ver <= version) Xiao::Read(reader, var, version);
				UBA_KERNEL_STATS
				#undef UBA_KERNEL_STAT
				return;
			}

			uint16 bits = reader.ReadU16();
			#define UBA_KERNEL_STAT(type, var, ver) if (bits & (1 << Bit_##var)) Xiao::Read(reader, var, version);
			UBA_KERNEL_STATS
			#undef UBA_KERNEL_STAT
		}

		enum
		{
			#define UBA_KERNEL_STAT(type, var, ver) Bit_##var,
			UBA_KERNEL_STATS
			#undef UBA_KERNEL_STAT
		};

		void Print(FTooltipDrawState& InToolTip, const bool writeHeader, uint64 frequency) const
		{
			if (writeHeader)
				InToolTip.AddTitle(TEXT("  ------- Kernel stats summary --------"));

			#define UBA_KERNEL_STAT(type, var, ver) LogStat(InToolTip, #var, var, frequency);
				UBA_KERNEL_STATS
			#undef UBA_KERNEL_STAT

			if (writeHeader)
				InToolTip.AddTextLine(TEXT(""), FLinearColor::White);
		}

		bool IsEmpty() const
		{
			#define UBA_KERNEL_STAT(type, var, ver) if (var.count) return false;
			UBA_KERNEL_STATS
			#undef UBA_KERNEL_STAT
			return true;
		}
	};

	struct CacheStats
	{
		#define UBA_CACHE_STATS \
			UBA_CACHE_STAT(FTimer, fetchEntries, 0) \
			UBA_CACHE_STAT(FTimer, fetchCasTable, 0) \
			UBA_CACHE_STAT(FTimer, normalizeFile, 28) \
			UBA_CACHE_STAT(FTimer, testEntry, 0) \
			UBA_CACHE_STAT(FTimer, fetchOutput, 0) \
			UBA_CACHE_STAT(AtomicU64, fetchBytesRaw, 26) \
			UBA_CACHE_STAT(AtomicU64, fetchBytesComp, 26) \

		#define UBA_CACHE_STAT(type, var, ver) type var;
		UBA_CACHE_STATS
		#undef UBA_CACHE_STAT

		void Read(FBinaryReader& reader, const uint32 version)
		{
			#define UBA_CACHE_STAT(type, var, ver) if (ver <= version) Xiao::Read(reader, var, version);
			UBA_CACHE_STATS
			#undef UBA_CACHE_STAT
		}

		void Print(FTooltipDrawState& InToolTip, const uint64 frequency) const
		{
			#define UBA_CACHE_STAT(type, var, ver) LogStat(InToolTip, #var, var, frequency);
			UBA_CACHE_STATS
			#undef UBA_CACHE_STAT
			if (fetchBytesComp)
				InToolTip.AddTextLine(FString::Printf(TEXT("   Bytes   Raw/Comp %9s %9s"), *BytesToText(fetchBytesRaw), *BytesToText(fetchBytesComp)), FLinearColor::White);
		}

		bool IsEmpty() const
		{
			#define UBA_CACHE_STAT(type, var, ver) if (var != type()) return false;
			UBA_CACHE_STATS
			#undef UBA_CACHE_STAT
			return true;
		}
	};
}