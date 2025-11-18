#include "LinuxAgentStatsMonitor.h"
#include "HAL/PlatformMisc.h"
#include "HAL/PlatformTime.h"
#include "XiaoLog.h"

FLinuxAgentStatsMonitor::FLinuxAgentStatsMonitor()
{
	
}

FLinuxAgentStatsMonitor::~FLinuxAgentStatsMonitor()
{
	Deinitialize();
}

void FLinuxAgentStatsMonitor::Initialize()
{
	XIAO_LOG(Verbose, TEXT("AgentStatsMonitor Initialize Begin."));
	GLog->Flush();

	bIsInitialized = true;
}

void FLinuxAgentStatsMonitor::Deinitialize()
{
}

bool FLinuxAgentStatsMonitor::CollectQueryData()
{
	return false;
}

float FLinuxAgentStatsMonitor::GetCpuUtilization()
{
	if (!IsInitialized())
	{
		return 0.0f;
	}

	return SumUtilization / CoreCpuUtilization.Num();
}

float FLinuxAgentStatsMonitor::GetCpuTemperature()
{
	return 0.0f;
}

float FLinuxAgentStatsMonitor::GetNetworkUtilization()
{
	return FMath::Abs((FMath::Abs(LastRecvSpeed)+FMath::Abs(LastSendSpeed))/2.0f/LastBandWidth);
}

float FLinuxAgentStatsMonitor::GetNetworkSpeed()
{
	return (FMath::Abs(LastRecvSpeed) + FMath::Abs(LastSendSpeed)) / 2.0f;
}

float FLinuxAgentStatsMonitor::GetGpuUtilization()
{
	return 0.0f;
}

float FLinuxAgentStatsMonitor::GetGpuTemperature()
{
	return 0.0f;
}

bool FLinuxAgentStatsMonitor::GetHelperCache(TTuple<int16, int16>& OutGpuUtilization)
{
	return false;
}

bool FLinuxAgentStatsMonitor::GetUpDownTime(TTuple<bool, FString>& OutUpDownTime)
{
	return false;
}

bool FLinuxAgentStatsMonitor::QueryUpdatedUtilization()
{
	return true;
}
