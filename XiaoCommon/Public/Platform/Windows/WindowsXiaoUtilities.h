/**
  * @author cxx2020@outlook.com
  * @datetime 2024 -12:18 AM
 */

#pragma once

#include "../../XiaoGenericUtilities.h"
#include "../../XiaoLog.h"

#if PLATFORM_WINDOWS
#include "Windows/AllowWindowsPlatformTypes.h"
#endif

class FWindowsXiaoUtilities : public FGenericXiaoUtilities
{
public:
	static bool SetProcessAffinity(const FProcHandle& InProcHandle, const uint64 InAffinity)
	{
		if(!InProcHandle.IsValid())
		{
			return false;
		}
		
		if(!SetProcessAffinityMask(InProcHandle.Get(), InAffinity))
		{
			return false;
		}
		return true;
	}
};

typedef FWindowsXiaoUtilities FXiaoUtilities;

#if PLATFORM_WINDOWS
#include "Windows/HideWindowsPlatformTypes.h"
#endif