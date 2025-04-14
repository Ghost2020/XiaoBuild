/**
  * @author cxx2020@outlook.com
  * @date 10:24 PM
 */
#pragma once

#include "CoreMinimal.h"

class FGenericXiaoUtilities
{
public:
	static bool SetProcessAffinity(const FProcHandle& InProcHandle, const uint64 InAffinity)
	{
		return false;
	}
};


#if PLATFORM_WINDOWS or PLATFORM_APPLE or PLATFORM_MAC
#include COMPILED_PLATFORM_HEADER(XiaoUtilities.h)
#else
typedef FGenericXiaoUtilities FXiaoUtilities;
#endif