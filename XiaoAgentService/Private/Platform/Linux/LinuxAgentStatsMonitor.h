#pragma once

#include "../../Agent/GenericAgentStatsMonitor.h"

class FLinuxAgentStatsMonitor final : public FGenericAgentStatsMonitor
{
public:
	FLinuxAgentStatsMonitor();
	
	virtual ~FLinuxAgentStatsMonitor() override;
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
	double LastQueryTime = DBL_MIN;

	// 网络带宽
	double LastBandWidth = 0.0f;
	// 发送速度
	double LastSendSpeed = 0.0f;
	// 接收速度
	double LastRecvSpeed = 0.0f;

};

typedef FLinuxAgentStatsMonitor FAgentStatsMonitor;


