/**
  * @author cxx2020@outlook.com
  * @date 10:24 PM
 */

#pragma once

#include "CoreMinimal.h"
#include "XiaoInstall.h"

static FString GTaskFile;

extern bool GAgentAutoSelectPort = false;
extern FString GAgentBuildGroup = TEXT("default");
extern bool GAgentInstallAddsOn = false;
extern EAgentType GAgentType = AT_Helper;
extern EInitiatorType GInitiatorType = InitiatorType_Fixed;
extern EHelperType GHelperType = ET_Fixed;
extern uint32 GHelperCores = 0;
extern FString GAgentDesc = TEXT("");
extern bool GAgentBuildCache = false;
extern FString GAgentDbDir = TEXT("");
extern FString GAgentCacheDir = TEXT("");
extern uint32 GAgentMaxFileCacheSize = 4096;

// Coordinator
extern FString GCoordiDatabaseDir = TEXT("");

extern bool GbWatchDogThread = false;
extern bool GbNoUncSync = true;
extern bool GbStopOnerror = false;
extern bool GbOnlyFallLocally = false;
extern bool GbShowCmd = false; 
extern bool GbOpenMonitor = false; 
extern bool GbUseIdeMonitor = false;
extern bool GbEnableAgent = true;
extern bool GbResetAgentCache = true;
extern uint32 SPPID = 0;


bool OnInstall();
bool UnInstall();
bool OnUpdate();