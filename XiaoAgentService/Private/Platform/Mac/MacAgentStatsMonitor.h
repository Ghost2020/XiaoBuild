#pragma once

#include "../../Agent/GenericAgentStatsMonitor.h"
#include <IOKit/IOKitLib.h>
#include <CoreFoundation/CoreFoundation.h>

class FMacAgentStatsMonitor final : public FGenericAgentStatsMonitor
{
public:
	FMacAgentStatsMonitor();
	
	virtual ~FMacAgentStatsMonitor() override;
	virtual void Initialize() override;
	virtual void Deinitialize() override;
	virtual bool CollectQueryData() override;
	virtual float GetCpuUtilization() override;
	virtual float GetCpuTemperature() override;
	virtual float GetNetworkUtilization() override;
	virtual float GetNetworkSpeed() override;
	virtual float GetGpuUtilization() override;
	virtual float GetGpuTemperature() override;
	virtual bool GetHelperCache(TTuple<int16, int16>& OutGpuUtilization) override;
	virtual bool GetUpDownTime(TTuple<bool, FString>& OutUpDownTime) override;

private:
	io_service_t CpuService;
	CFMutableDictionaryRef CpuProperties = nullptr;
	io_service_t GpuService;
	CFMutableDictionaryRef GpuProperties = nullptr;

	double LastQueryTime = DBL_MIN;

	uint64_t LastTotalTicks = 0;
	uint64_t LastIdleTicks = 0;

	double MaxSpeed = 0.0f;
	
	double SendSpeed = 0.0f;
	double LastSendBytes = 0.0f;
	
	double RecvSpeed = 0.0f;
	double LastRecvBytes = 0.0f;
};

// typedef FMacAgentStatsMonitor FAgentStatsMonitor;


