/**
  * @author cxx2020@outlook.com
  * @date 5:54 PM
 */

#pragma once

#include "CoreMinimal.h"
#include "HAL/PlatformTime.h"
#include "XiaoAgent.h"
#include "Database/Users.h"
#include "App/Monitor/Tracks/UbaTraceReader.h"
#include "HAL/PlatformFileManager.h"

inline FDateTime GDateTime;

inline FAgentSettings GAgentSettings;

inline XiaoDB::FUserDesc GCurrentUser;

inline bool GCanUpdate = true;

// 显示系统相关数据标志
struct FSystemGraphShowFlags
{
	// Processor
	bool bCpuUsage = false;
	bool bKernelTime = false;
	bool bUserTime = false;
	bool bDpcTime = false;
	bool bInterruptTime = false;
	bool bInterruptCount = false;

	// Memory
	bool bPhysicalTotal = false;
	bool bPhysicalAvailable = false;
	bool bVirtualTotal = false;
	bool bVirtualLimit = false;
	bool bPageFaults = false;

	// Distribution
	bool bAssignedCpus = false;
	bool bUtilizedCpus = false;
	bool bAssignedProcessionPower = false;
	bool bUtilizedProcessingPower = false;
	bool bReadyTask = false;
	bool bActiveTask = false;
	bool bNeededHelpers = false;
	
	// Synchronization
	bool bFilesSynchronized = false;
	bool bDirsSynchronized = false;
	bool bKeysSynchronized = false;
	bool bDirsScanned = false;
	bool bBytesToDownload = false;
	bool bFilesToDownload = false;
	bool bBytesToWrite = false;
};

// 构建历史信息记录
struct FBuildHistoryDesc
{
	explicit FBuildHistoryDesc()
	{}

	explicit FBuildHistoryDesc(const Xiao::FTraceView& InTraceView, const FString& InFilePath)
		: FilePath(InFilePath)
	{
		uint64 LastTime = 0;
		for (const auto& Session : InTraceView.sessions)
		{
			for (const auto& Processor : Session.processors)
			{
				if (Processor.processes.Num() > 0)
				{
					const auto& LastProcess = Processor.processes.Last();
					LastTime = LastProcess.stop > LastTime ? LastProcess.stop : LastTime;
				}

				for (const auto& Process : Processor.processes)
				{
					if (Process.exitCode != 0)
					{
						++ErrorNum;
					}

					for (const auto& Log : Process.logLines)
					{
						if (Log.type == Xiao::ELogEntryType::LogEntryType_Warning)
						{
							++WarningNum;
						}
					}
				}
			}
		}
		BuildStatus = ErrorNum > 0 ? -1 : (WarningNum > 0 ? 1 : 0);
		StartTime = FPlatformFileManager::Get().GetPlatformFile().GetTimeStamp(*InFilePath);
		Duration = LastTime;
		Version = FString::FromInt(InTraceView.version);
	}

	int BuildStatus = 0; // -1::失败 0::成功 1::成功但有警告 2::取消

	bool bBuildOrRebuild = true;

	FDateTime StartTime;
	FTimespan Duration;

	uint16 ErrorNum = 0;
	uint16 WarningNum = 0;

	uint16 AutoRecoveryNum = 0;

	FString Version;

	EProjectType Type = EProjectType::Type_Unknwon;
	FString SolutionFilename = TEXT("XiaoBuild");

	FString FilePath;
};

// 系统显示条目描述
struct FGraphItemRow
{
	explicit FGraphItemRow(const bool bInReal, const bool InbEnable = false, const FText& InName = FText::GetEmpty(), const FLinearColor InColor = FLinearColor(1.0f, 1.0f, 1.0f, 0.1), const FText& InScale = FText::GetEmpty())
		: bReal(bInReal)
		, bEnable(InbEnable)
		, Name(InName)
		, Color(InColor)
		, Scale(InScale)
	{}

	bool bReal;
	bool bEnable;
	FText Name;
	FLinearColor Color;
	FText Scale;
};

// Simple stopwatch.
struct FStopwatch
{
	uint64 AccumulatedTime = 0;
	uint64 StartTime = 0;
	bool bIsStarted = false;

	void Start()
	{
		if (!bIsStarted)
		{
			bIsStarted = true;
			StartTime = FPlatformTime::Cycles64();
		}
	}

	void Stop()
	{
		if (bIsStarted)
		{
			bIsStarted = false;
			AccumulatedTime += FPlatformTime::Cycles64() - StartTime;
		}
	}

	void Update()
	{
		if (bIsStarted)
		{
			const uint64 CrtTime = FPlatformTime::Cycles64();
			AccumulatedTime += CrtTime - StartTime;
			StartTime = CrtTime;
		}
	}

	void Restart()
	{
		AccumulatedTime = 0;
		bIsStarted = true;
		StartTime = FPlatformTime::Cycles64();
	}

	void Reset()
	{
		AccumulatedTime = 0;
		StartTime = 0;
		bIsStarted = false;
	}

	double GetAccumulatedTime() const
	{
		return FStopwatch::Cycles64ToSeconds(AccumulatedTime);
	}

	uint64 GetAccumulatedTimeMs() const
	{
		return FStopwatch::Cycles64ToMilliseconds(AccumulatedTime);
	}

	static double Cycles64ToSeconds(const uint64 Cycles64)
	{
		return static_cast<double>(Cycles64) * FPlatformTime::GetSecondsPerCycle64();
	}

	static uint64 Cycles64ToMilliseconds(const uint64 Cycles64)
	{
		const double Milliseconds = FMath::RoundToDouble(static_cast<double>(Cycles64 * 1000) * FPlatformTime::GetSecondsPerCycle64());
		return static_cast<uint64>(Milliseconds);
	}
};

#define INSIGHTS_DECLARE_NAMESPACE_RTTI(Namespace, ClassName, BaseClassName) \
	public: \
		static const FName& GetStaticTypeName() { return Namespace::ClassName::TypeName; } \
		virtual const FName& GetTypeName() const override { return Namespace::ClassName::GetStaticTypeName(); } \
		virtual bool IsKindOf(const FName& InTypeName) const override { return InTypeName == Namespace::ClassName::GetStaticTypeName() || BaseClassName::IsKindOf(InTypeName); } \
	private: \
		static const FName TypeName;

#define INSIGHTS_IMPLEMENT_NAMESPACE_RTTI(Namespace, ClassName) \
	const FName ClassName::TypeName = TEXT(#ClassName);


enum class ESeriesType : uint8
{
	Percent,
	StatsCounter,
	Interrupts,
	Memory,
	Faults,
	CPUs,
	GHz,
	Tasks,
	Secs,
	Files,
	TransferSpeed,
};

struct FSystemGraphDesc
{
	explicit FSystemGraphDesc(const FString& InDesc, const FColor& InColor, const FLinearColor& InBorderColor, const ESeriesType InType, const double InMaxVal = 100.0f)
		: Desc(InDesc)
		, Color(InColor)
		, BorderColor(InBorderColor)
		, Type(InType)
		, MaxVal(InMaxVal)
	{
	}

	FString Desc;
	FColor Color;
	FLinearColor BorderColor;
	ESeriesType Type;
	double MaxVal;
};

static const TMap<FString, FSystemGraphDesc> SGraphMap = {
	{SCpuUsage, FSystemGraphDesc(TEXT("Cpu使用率"), FColor(255, 192, 255), FLinearColor(0.3f, 0.3f, 1.0f, 1.0f), ESeriesType::Percent, 1.1f) },
	{SSend, FSystemGraphDesc(TEXT("发送"), FColor(0, 35, 245), FLinearColor(0.3f, 0.3f, 1.0f, 1.0f), ESeriesType::TransferSpeed) },
	{SReceive, FSystemGraphDesc(TEXT("接受"), FColor(255, 242, 0), FLinearColor(0.3f, 0.3f, 1.0f, 1.0f), ESeriesType::TransferSpeed) },
	{SPing, FSystemGraphDesc(TEXT("Ping"), FColor(34, 177, 76), FLinearColor(0.3f, 0.3f, 1.0f, 1.0f), ESeriesType::StatsCounter, 500000.0f) },
	{SMemAvail, FSystemGraphDesc(TEXT("内存可用"), FColor(0, 162, 232), FLinearColor(0.3f, 0.3f, 1.0f, 1.0f), ESeriesType::Memory) },
	{SConnectionCount, FSystemGraphDesc(TEXT("连接数量"), FColor(163, 73, 164), FLinearColor(0.3f, 0.3f, 1.0f, 1.0f), ESeriesType::StatsCounter, 50.0f) },
	{SPhysicalAvailable, FSystemGraphDesc(TEXT("可用物理"), FColor(195, 195, 195), FLinearColor(0.3f, 0.3f, 1.0f, 1.0f), ESeriesType::Memory) },
	{SVirtualTotal, FSystemGraphDesc(TEXT("虚拟内存"), FColor(185, 122, 87), FLinearColor(0.3f, 0.3f, 1.0f, 1.0f), ESeriesType::Memory) },
	{SVirtualLimit, FSystemGraphDesc(TEXT("虚拟Limit"), FColor(255, 174, 201), FLinearColor(0.3f, 0.3f, 1.0f, 1.0f), ESeriesType::Memory) },
	{SSoftPageFaults, FSystemGraphDesc(TEXT("内存缺页次数"), FColor(255, 201, 14), FLinearColor(0.3f, 0.3f, 1.0f, 1.0f), ESeriesType::Faults) },
	{SHardPageFaults, FSystemGraphDesc(TEXT("硬盘缺页次数"), FColor(255, 201, 14), FLinearColor(0.3f, 0.3f, 1.0f, 1.0f), ESeriesType::Faults) },
	{STotalPageFaults, FSystemGraphDesc(TEXT("总共缺页次数"), FColor(255, 201, 14), FLinearColor(0.3f, 0.3f, 1.0f, 1.0f), ESeriesType::Faults) },
	{SAssignedCpus, FSystemGraphDesc(TEXT("已分配的CPU"), FColor(239, 228, 176), FLinearColor(0.3f, 0.3f, 1.0f, 1.0f), ESeriesType::CPUs) },
	{SUtilizedCpus, FSystemGraphDesc(TEXT("已利用的CPU"), FColor(181, 230, 29), FLinearColor(0.3f, 0.3f, 1.0f, 1.0f), ESeriesType::CPUs) },
	{SAssignedPower, FSystemGraphDesc(TEXT("已分配的功率"), FColor(153, 217, 234), FLinearColor(0.3f, 0.3f, 1.0f, 1.0f), ESeriesType::GHz) },
	{SUtilizedPower, FSystemGraphDesc(TEXT("已利用的功率"), FColor(112, 146, 190), FLinearColor(0.3f, 0.3f, 1.0f, 1.0f), ESeriesType::GHz) },
	{SReadyTasks, FSystemGraphDesc(TEXT("预备的任务"), FColor(200, 191, 231), FLinearColor(0.3f, 0.3f, 1.0f, 1.0f), ESeriesType::Tasks) },
	{SActiveTasks, FSystemGraphDesc(TEXT("活跃的任务"), FColor(161, 251, 142), FLinearColor(0.3f, 0.3f, 1.0f, 1.0f), ESeriesType::Tasks) },
	{SNeededHelpers, FSystemGraphDesc(TEXT("需要的的协助者"), FColor(58, 8, 62), FLinearColor(0.3f, 0.3f, 1.0f, 1.0f), ESeriesType::Tasks) },
	{SFilesSynchronized, FSystemGraphDesc(TEXT("文件同步"), FColor(117, 250, 141), FLinearColor(0.3f, 0.3f, 1.0f, 1.0f), ESeriesType::Secs) },
	{SDirsSynchronized, FSystemGraphDesc(TEXT("目录同步"), FColor(117, 250, 141), FLinearColor(0.3f, 0.3f, 1.0f, 1.0f), ESeriesType::Secs) },
	{SKeysSynchronized, FSystemGraphDesc(TEXT("Keys同步"), FColor(119, 67, 66), FLinearColor(0.3f, 0.3f, 1.0f, 1.0f), ESeriesType::Secs) },
	{SDirsScanned, FSystemGraphDesc(TEXT("扫描目录"), FColor(115, 251, 253), FLinearColor(0.3f, 0.3f, 1.0f, 1.0f), ESeriesType::Secs) },
	{SBytesToDownload, FSystemGraphDesc(TEXT("字节下载量"), FColor(0, 35, 245), FLinearColor(0.3f, 0.3f, 1.0f, 1.0f), ESeriesType::Memory) },
	{SFilesToDownload, FSystemGraphDesc(TEXT("文件下载量"), FColor(117, 249, 77), FLinearColor(0.3f, 0.3f, 1.0f, 1.0f), ESeriesType::Files) },
	{SBytesToWrite, FSystemGraphDesc(TEXT("写入字节量"), FColor(234, 54, 128), FLinearColor(0.3f, 0.3f, 1.0f, 1.0f), ESeriesType::Memory) }
};


static const TArray<FString> AllGraphNames = { SSend, SReceive, SPing, SMemAvail, SCpuUsage, SConnectionCount};

#ifndef JSON_MCI_VALUE
#define JSON_MCI_VALUE(var) JSON_SERIALIZE(#var, var)
#endif

struct FMonitorSettings final : FJsonSerializable
{
	FString SavePath = FPaths::ConvertRelativePathToFull(FPaths::Combine(FPaths::ProjectConfigDir(), TEXT("XiaoBuildMonitor.json")));

	int EnabledOptions = 410;
	int VisibleOptions = 126;

	TMap<FString, int32> ShowMap;

	void Save() const
	{
		FFileHelper::SaveStringToFile(ToJson(), *SavePath);
	}

	void Load()
	{
		FString Result;
		if (FFileHelper::LoadFileToString(Result, *SavePath))
		{
			FromJson(Result);
		}
	}

	BEGIN_JSON_SERIALIZER
		JSON_MCI_VALUE(EnabledOptions);
		JSON_MCI_VALUE(VisibleOptions);
		JSON_SERIALIZE_MAP("ShowMap", ShowMap);
	END_JSON_SERIALIZER
};

inline FMonitorSettings GMonitorSettings;