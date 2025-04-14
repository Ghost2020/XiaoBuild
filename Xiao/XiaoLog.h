/**
* @author cxx2020@outlook.com
  * @date 10:24 PM
 */
#pragma once

#include "CoreMinimal.h"
#include "Misc/OutputDeviceRedirector.h"


DEFINE_LOG_CATEGORY_STATIC(LogXiao, Log, All);


static bool GNeedFlush = false;

#define XIAO_LOG(Verbosity, FMT, ...) \
	UE_LOG(LogXiao, Verbosity, (FMT), ##__VA_ARGS__); \
	if(GNeedFlush) \
	{ \
		GLog->Flush(); \
	}
//	if(NgLogQueue.IsValid() && (ELogVerbosity::Verbosity == ELogVerbosity::Fatal || ELogVerbosity::Verbosity == ELogVerbosity::Error)) \
//	{ \
//		const boost::posix_time::ptime Timeout = boost::posix_time::microsec_clock::universal_time() + boost::posix_time::microseconds(5); \
//		const std::string Buffer = "Error"; \
//		NgLogQueue->timed_send(Buffer.c_str(), Buffer.size(), 0, Timeout); \
//	}