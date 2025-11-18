/**
  * @author cxx2020@outlook.com
  * @datetime 2023 -10:13 PM
 */
#pragma once

#include "Containers/Array.h"
#include "Templates/Tuple.h"
#include "XiaoLog.h"
#include "agent.pb.h"


static constexpr float SMegabyteNum = 1024.0f * 1024.0f * 1024.0f;


class FGenericAgentStatsMonitor
{
public:
	virtual ~FGenericAgentStatsMonitor()
	{
	}

	virtual void Initialize() 
	{

	}

	virtual void Deinitialize()
	{

	}

	virtual bool IsInitialized() const
	{
		return bIsInitialized;
	}

	virtual bool CollectQueryData()
	{
		return false;
	}

	const TArray<int8>& GetCpusUtilization() const
	{
		return CoreCpuUtilization;
	}

	virtual float GetCpuUtilization()
	{
		return 0.0f;
	}

	virtual float GetCpuTemperature()
	{
		return false;
	}

	virtual float GetDiskUtilization()
	{
		return false;
	}

	virtual float GetNetworkUtilization()
	{
		return 0.0f;
	}

	virtual FString GetGPUDesc()
	{
		return TEXT("");
	}

	virtual float GetGpuUtilization()
	{
		return false;
	}

	virtual float GetGpuTemperature()
	{
		return false;
	}

	virtual bool GetGpuMemoryUsageUtility(TTuple<float, float>& OutMemoryUsageUtililty)
	{
		return false;
	}

	virtual bool GetHelperCache(TTuple<int16, int16>& OutGpuUtilization)
	{
		return false;
	}

	static bool GetAvailableMemory(TTuple<uint64, uint32>& OutAvailableMemory);

	static bool GetAvailableSpace(TTuple<uint64, uint64>& OutDiskSpace, const FString& InTargetDrive);

	virtual bool GetUpDownTime(TTuple<bool, FString>& OutUpDownTime)
	{
		return false;
	}

	virtual float GetNetworkSpeed()
	{
		return 0.0f;
	}

	void UpdateAgentStats(FAgentProto& OutAgentProto, const FString& InTargetDrive)
	{
		if (!CollectQueryData())
		{
			XIAO_LOG(Error, TEXT("Collect Query Data failed"));
			return;
		}

		OutAgentProto.set_cpuava(100 - GetCpuUtilization());
		TTuple<uint64, uint32> AvaMem;
		if (GetAvailableMemory(AvaMem))
		{
			OutAgentProto.set_usememory(AvaMem.Key);
			OutAgentProto.set_totalmemory(AvaMem.Value);
		}
		
		TTuple<uint64, uint64> AvaSpace;
		if (GetAvailableSpace(AvaSpace, InTargetDrive))
		{
			OutAgentProto.set_usehardspace(AvaSpace.Key);
			OutAgentProto.set_totalhardspace(AvaSpace.Value);
		}

		TTuple<float, float> GpuMemUtility;
		if (GetGpuMemoryUsageUtility(GpuMemUtility))
		{
			// XIAO_LOG(Log, TEXT("DedicatedMem::%fGB SharedMem::%fGB"), GpuMemUtility.Key, GpuMemUtility.Value);
		}
		
		TTuple<int16, int16> AvaCache;// #TODO
		if (GetHelperCache(AvaCache))
		{
			OutAgentProto.set_usehelpcache(AvaCache.Key);
			OutAgentProto.set_totalhelpcache(AvaCache.Value);
		}
		else
		{
			// XIAO_LOG(Warning, TEXT("GetHelperCache Failed!"));
		}

		OutAgentProto.set_avadisk(GetDiskUtilization());
		OutAgentProto.set_avalnet(GetNetworkUtilization());
		OutAgentProto.set_networkspeed(GetNetworkSpeed());
		OutAgentProto.set_avagpu(GetGpuUtilization());
	}

protected:
	bool bIsInitialized = false;

	TArray<int8> CoreCpuUtilization;
	TArray<int32> CpuTemperature;
	TArray<int8> CoreGpuUtilization;
	TArray<int32> GpuTemperature;
};

#if PLATFORM_WINDOWS || PLATFORM_MAC || PLATFORM_UNIX
#include COMPILED_PLATFORM_HEADER(AgentStatsMonitor.h)
#else
typedef FGenericAgentStatsMonitor FAgentStatsMonitor;
#endif
