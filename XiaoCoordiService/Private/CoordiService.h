/**
  * @author cxx2020@outlook.com
  * @datetime 2023 -8:00 PM
 */

#pragma once

#include "CoreMinimal.h"

namespace sw::redis
{
	class ConnectionOptions;
}
using namespace sw::redis;

struct FProcHandle;

class FCoordiService final
{
public:
	static bool OnInitialize(const FString& InParams);
	static void OnDeinitialize();
	static void OnTick();

private:
	static bool TryRunServer(const FString& InServerName, const FString& InParams, FProcHandle& OutProcHandle, void*& ReadPipe, void*& WritePipe);
	static bool TryRunRedisServer();
	static void TryBecomeMaster();
	static void BecomeSlave();
	static void UpdateRedisCluster();
	static void UpdateAgentStatus();
	static void UpdateNetworkStatus();

public:
	static float SSleepTime;
};

