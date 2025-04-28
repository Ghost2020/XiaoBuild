/**
  * @author cxx2020@outlook.com
  * @date 10:24 PM
 */

#pragma once

#include "CoreMinimal.h"
#include "XiaoInstall.h"

inline FString GTaskFile = TEXT("");

inline bool GAgentAutoSelectPort = false;
inline FString GAgentBuildGroup = TEXT("default");
inline bool GAgentInstallAddsOn = false;
inline EAgentType GAgentType = AT_Helper;
inline EInitiatorType GInitiatorType = InitiatorType_Fixed;
inline EHelperType GHelperType = ET_Fixed;
inline uint32 GHelperCores = 0;
inline FString GAgentDesc = TEXT("");
inline bool GAgentBuildCache = false;
inline FString GAgentDbDir = TEXT("");
inline FString GAgentCacheDir = TEXT("");
inline uint32 GAgentMaxFileCacheSize = 4096;

// Coordinator
inline FString GCoordiDatabaseDir = TEXT("");

inline bool GbWatchDogThread = false;
inline bool GbNoUncSync = true;
inline bool GbStopOnerror = false;
inline bool GbOnlyFallLocally = false;
inline bool GbShowCmd = false; 
inline bool GbOpenMonitor = false; 
inline bool GbUseIdeMonitor = false;
inline bool GbEnableAgent = true;
inline bool GbResetAgentCache = true;
inline uint32 SPPID = 0;


bool OnInstall();
bool UnInstall();
bool OnUpdate();
