/**
  * @author cxx2020@outlook.com
  * @datetime 2023 -9:32 PM
 */

#pragma once

#include "CoreMinimal.h"

class FAgentService final
{
public:
	static bool OnInitialize(const FString& InParams);
	static void OnDeinitialize();
	static void OnTick(const float InDeltaTime);

protected:
	static bool UpdateClusterConfig();
	static bool UpdateAgentStats(const bool bInit = false);
	static void UpdateAgentSettings();
	static void UpdateAgentProtobuf(const bool bInit);
	static bool TryRunUbaAgent();
	static void TryRunIPerfServer();
};
