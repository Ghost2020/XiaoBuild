#pragma once

#ifndef _WIN32_WINDOWS
#define _WIN32_WINDOWS 0x0410
#endif

#include "../../Agent/GenericAgentStatsMonitor.h"

#include "Windows/AllowWindowsPlatformTypes.h"
#include <Windows.h>
#include <Pdh.h>
#include <PdhMsg.h>
#include "Windows/HideWindowsPlatformTypes.h"

class FWindowsAgentStatsMonitor final : public FGenericAgentStatsMonitor
{
public:
	FWindowsAgentStatsMonitor();
	
	virtual ~FWindowsAgentStatsMonitor() override;
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
	bool QueryUpdatedUtilization();

	HQUERY QueryHandle = INVALID_HANDLE_VALUE;
	double LastQueryTime = DBL_MIN;

	// Processors
	PDH_HCOUNTER ProcessorCounterHandle;
	// Network Recv,Send 
	PDH_HCOUNTER NetworkBandWidthHandle, NetworkRecvHandle, NetworkSendHandle;
	// 网络带宽
	double LastBandWidth = 0.0f;
	// 发送速度
	double LastSendSpeed = 0.0f;
	// 接收速度
	double LastRecvSpeed = 0.0f;
	//GPU Utiliz
	PDH_HCOUNTER GpuUtilizeHandle;

};

typedef FWindowsAgentStatsMonitor FAgentStatsMonitor;


