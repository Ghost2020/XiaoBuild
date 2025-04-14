/**
  * @author cxx2020@outlook.com
  * @date 10:24 PM
 */

#pragma once

#include "CoreMinimal.h"
#include "XiaoInstall.h"

static FString GTaskFile;

static bool GAgentAutoSelectPort = false;
static FString GAgentBuildGroup = TEXT("default");
static bool GAgentInstallAddsOn = false;
static EAgentType GAgentType = AT_Helper;
static EInitiatorType GInitiatorType = InitiatorType_Fixed;
static EHelperType GHelperType = ET_Fixed;
static uint32 GHelperCores = 0;
static FString GAgentDesc = TEXT("");
static bool GAgentBuildCache = false;
static FString GAgentDbDir = TEXT("");
static FString GAgentCacheDir = TEXT("");
static uint32 GAgentMaxFileCacheSize = 4096;

// Coordinator
static FString GCoordiDatabaseDir = TEXT("");

static bool GbWatchDogThread = false;
static bool GbNoUncSync = true;
static bool GbStopOnerror = false;
static bool GbOnlyFallLocally = false;
static bool GbShowCmd = false; 
static bool GbOpenMonitor = false; 
static bool GbUseIdeMonitor = false;
static bool GbEnableAgent = true;
static bool GbResetAgentCache = true;
static uint32 SPPID = 0;


static bool OnInstall();
static bool UnInstall();
static bool OnUpdate();