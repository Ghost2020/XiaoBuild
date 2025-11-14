#include "WindowsAgentStatsMonitor.h"
#include "HAL/PlatformMisc.h"
#include "HAL/PlatformTime.h"
#include "XiaoLog.h"

static constexpr float SMegabyteNum = 1024 * 1024;

FWindowsAgentStatsMonitor::FWindowsAgentStatsMonitor()
{
	
}

FWindowsAgentStatsMonitor::~FWindowsAgentStatsMonitor()
{
	Deinitialize();
}

void FWindowsAgentStatsMonitor::Initialize()
{
	XIAO_LOG(Verbose, TEXT("AgentStatsMonitor Initialize Begin."));
	GLog->Flush();

	PDH_STATUS PdhStatus = ::PdhOpenQuery(nullptr, 0, &QueryHandle);
	if (PdhStatus != ERROR_SUCCESS || !QueryHandle)
	{
		XIAO_LOG(Error, TEXT("PdhOpenQuery failed. Error code: %d"), PdhStatus);
	}

	// Processor Time
	const int32 NumCores = FPlatformMisc::NumberOfCores();
	CoreCpuUtilization.SetNum(1);
	const FString TotalCounterName = TEXT("\\Processor(_Total)\\% Processor Time");
	PdhStatus = ::PdhAddEnglishCounter(QueryHandle, *TotalCounterName, 0, &ProcessorCounterHandle);
	if (PdhStatus != ERROR_SUCCESS)
	{
		XIAO_LOG(Warning, TEXT("PdhAddEnglishCounter %s failed. Error code: %d"), *TotalCounterName, PdhStatus);
		return;
	}

	// \\Network Interface(*)\\Current Bandwidth
	FString CounterName = TEXT("\\Network Interface(*)\\Current Bandwidth");
	PdhStatus = ::PdhAddEnglishCounter(QueryHandle, *CounterName, 0, &NetworkBandWidthHandle);
	if (PdhStatus != ERROR_SUCCESS)
	{
		XIAO_LOG(Warning, TEXT("PdhAddEnglishCounter %s failed. Error code: %d"), *CounterName, PdhStatus);
		return;
	}

	// \\Network Interface(*)\\Bytes Received/sec
	CounterName = TEXT("\\Network Interface(*)\\Bytes Received/sec");
	PdhStatus = ::PdhAddEnglishCounter(QueryHandle, *CounterName, 0, &NetworkRecvHandle);
	if (PdhStatus != ERROR_SUCCESS)
	{
		XIAO_LOG(Warning, TEXT("PdhAddEnglishCounter %s failed. Error code: %d"), *CounterName, PdhStatus);
		return;
	}

	// \\Network Interface(*)\\Bytes Sent/sec
	CounterName = TEXT("\\Network Interface(*)\\Bytes Sent/sec");
	PdhStatus = ::PdhAddEnglishCounter(QueryHandle, *CounterName, 0, &NetworkSendHandle);
	if (PdhStatus != ERROR_SUCCESS)
	{
		XIAO_LOG(Warning, TEXT("PdhAddEnglishCounter %s failed. Error code: %d"), *CounterName, PdhStatus);
		return;
	}

	// \\GPU Engine(*)\\Utilization Percentage
	CounterName = TEXT("\\GPU Engine(*_3D)\\Utilization Percentage");
	PdhStatus = ::PdhAddEnglishCounter(QueryHandle, *CounterName, 0, &GpuUtilizeHandle);
	if (PdhStatus != ERROR_SUCCESS)
	{
		XIAO_LOG(Warning, TEXT("PdhAddEnglishCounter %s failed. Error code: %d"), *CounterName, PdhStatus);
		return;
	}

	// Initial condition: ensures we've collected twice before the first attempt to get values.
	PdhStatus = ::PdhCollectQueryData(QueryHandle);
	if (PdhStatus != ERROR_SUCCESS)
	{
		XIAO_LOG(Warning, TEXT("First PdhCollectQueryData failed. Error code: %d"), PdhStatus);
		return;
	}

	XIAO_LOG(Verbose, TEXT("AgentStatsMonitor Initialize Finish."));
	GLog->Flush();
	bIsInitialized = true;
}

void FWindowsAgentStatsMonitor::Deinitialize()
{
	if (QueryHandle != INVALID_HANDLE_VALUE)
	{
		const PDH_STATUS PdhStatus = ::PdhCloseQuery(QueryHandle);
	}
}

bool FWindowsAgentStatsMonitor::CollectQueryData()
{
	if (QueryHandle == INVALID_HANDLE_VALUE)
	{
		::PdhOpenQuery(nullptr, 0, &QueryHandle);
	}

	const bool Rtn = ::PdhCollectQueryData(QueryHandle) == ERROR_SUCCESS;
	if (!Rtn)
	{
		const uint32 ErrorCode = FPlatformMisc::GetLastError();
		if (ErrorCode == 0)
		{
			return true;
		}
		TCHAR ErrorBuffer[2048];
		FPlatformMisc::GetSystemErrorMessage(ErrorBuffer, 2048, ErrorCode);
		XIAO_LOG(Warning, TEXT("Failed to PdhCollectQueryData: %u (%s)"), ErrorCode, ErrorBuffer);
	}
	return Rtn;
}

static bool GetFormattedCounterArray(const PDH_HCOUNTER& InCounter, DWORD dwFormat, PPDH_FMT_COUNTERVALUE pValue)
{
	PPDH_FMT_COUNTERVALUE_ITEM pAryValue = NULL;
	PDH_STATUS status = ERROR_SUCCESS;
	DWORD dwBufferSize = 0;
	DWORD dwItemCount = 0;

	do
	{
		status = ::PdhGetFormattedCounterArray(
			InCounter,
			dwFormat,
			&dwBufferSize,
			&dwItemCount,
			NULL
		);

		if (PDH_MORE_DATA != status)
		{
			break;
		}

		pAryValue = (PPDH_FMT_COUNTERVALUE_ITEM)::HeapAlloc(::GetProcessHeap(), 0, dwBufferSize);
		if (NULL == pAryValue)
		{
			break;
		}

		status = ::PdhGetFormattedCounterArray(
			InCounter,
			dwFormat,
			&dwBufferSize,
			&dwItemCount,
			pAryValue
		);

		PDH_FMT_COUNTERVALUE value = { 0 };
		for (DWORD i = 0; i < dwItemCount; i++)
		{
			if (PDH_FMT_DOUBLE == dwFormat)
			{
				value.doubleValue += pAryValue[i].FmtValue.doubleValue;
			}
			if (PDH_FMT_LARGE == dwFormat)
			{
				value.largeValue += pAryValue[i].FmtValue.largeValue;
			}
			if (PDH_FMT_LONG == dwFormat)
			{
				value.longValue += pAryValue[i].FmtValue.longValue;
			}
		}

		if (pValue)
		{
			if (PDH_FMT_DOUBLE == dwFormat)
			{
				value.doubleValue = value.doubleValue / dwItemCount;
			}

			if (PDH_FMT_LARGE == dwFormat)
			{
				value.largeValue = value.largeValue / dwItemCount;
			}

			if (PDH_FMT_LONG == dwFormat)
			{
				value.longValue = value.longValue / dwItemCount;
			}

			*pValue = value;
		}

	} while (false);

	if (pAryValue)
	{
		::HeapFree(::GetProcessHeap(), 0, pAryValue);
	}

	return ERROR_SUCCESS == status;
}

float FWindowsAgentStatsMonitor::GetCpuUtilization()
{
	if (!IsInitialized())
	{
		return 0.0f;
	}

	// Rate limiting. MSDN recommends <= 1 Hz.
	constexpr double MinQueryIntervalSec = 1.0;
	const double TimeNow = FPlatformTime::Seconds();
	if (TimeNow - LastQueryTime >= MinQueryIntervalSec)
	{
		if (!QueryUpdatedUtilization())
		{
			return 0.0f;
		}
	}

	return CoreCpuUtilization[0];
}

float FWindowsAgentStatsMonitor::GetCpuTemperature()
{
	return 0.0f;
}

float FWindowsAgentStatsMonitor::GetNetworkUtilization()
{
	PDH_FMT_COUNTERVALUE CounterRecvValue;
	PDH_STATUS PdhStatus = ::PdhGetFormattedCounterValue(NetworkRecvHandle, PDH_FMT_DOUBLE, nullptr, &CounterRecvValue);
	if (PdhStatus != ERROR_SUCCESS)
	{
		static bool CallOnce = true;
		if (CallOnce)
		{
			CallOnce = false;
			XIAO_LOG(Warning, TEXT("PdhGetFormattedCounterValue \"Network Recv\" failed. Error code: %d"), PdhStatus);
		}
		return 100.0f;
	}
	LastRecvSpeed = CounterRecvValue.doubleValue;

	PDH_FMT_COUNTERVALUE CounterSendValue;
	PdhStatus = ::PdhGetFormattedCounterValue(NetworkSendHandle, PDH_FMT_DOUBLE, nullptr, &CounterSendValue);
	if (PdhStatus != ERROR_SUCCESS)
	{
		static bool CallOnce = true;
		if (CallOnce)
		{
			CallOnce = false;
			XIAO_LOG(Warning, TEXT("PdhGetFormattedCounterValue \"Network Send\" failed. Error code: %d"), PdhStatus);
		}
		return 100.0f;
	}
	LastSendSpeed = CounterSendValue.doubleValue;

	PDH_FMT_COUNTERVALUE CounterBandWidthValue;
	PdhStatus = ::PdhGetFormattedCounterValue(NetworkBandWidthHandle, PDH_FMT_DOUBLE, nullptr, &CounterBandWidthValue);
	if (PdhStatus != ERROR_SUCCESS)
	{
		static bool CallOnce = true;
		if (CallOnce)
		{
			CallOnce = false;
			XIAO_LOG(Warning, TEXT("PdhGetFormattedCounterValue \"Network Bandwidth\" failed. Error code: %d"), PdhStatus);
		}
		return 0.0f;
	}
	LastBandWidth = CounterBandWidthValue.doubleValue;

	return FMath::Abs((FMath::Abs(LastRecvSpeed)+FMath::Abs(LastSendSpeed))/2.0f/LastBandWidth);
}

float FWindowsAgentStatsMonitor::GetNetworkSpeed()
{
	PDH_FMT_COUNTERVALUE RecvValue = { 0 };
	GetFormattedCounterArray(NetworkRecvHandle, PDH_FMT_DOUBLE, &RecvValue);
	LastRecvSpeed = RecvValue.doubleValue;

	PDH_FMT_COUNTERVALUE SentValue = { 0 };
	GetFormattedCounterArray(NetworkRecvHandle, PDH_FMT_DOUBLE, &SentValue);
	LastSendSpeed = SentValue.doubleValue;

	return (FMath::Abs(LastRecvSpeed) + FMath::Abs(LastSendSpeed)) / 2.0f;
}

float FWindowsAgentStatsMonitor::GetGpuUtilization()
{
	PDH_FMT_COUNTERVALUE Value = { 0 };
	GetFormattedCounterArray(GpuUtilizeHandle, PDH_FMT_DOUBLE, &Value);
	return (1.0f-Value.doubleValue)*100.0f;
}

float FWindowsAgentStatsMonitor::GetGpuTemperature()
{
	return 0.0f;
}

bool FWindowsAgentStatsMonitor::GetHelperCache(TTuple<int16, int16>& OutGpuUtilization)
{
	return false;
}

bool FWindowsAgentStatsMonitor::GetUpDownTime(TTuple<bool, FString>& OutUpDownTime)
{
	return false;
}

bool FWindowsAgentStatsMonitor::QueryUpdatedUtilization()
{
	PDH_FMT_COUNTERVALUE CounterValue;
	const PDH_STATUS PdhStatus = ::PdhGetFormattedCounterValue(ProcessorCounterHandle, PDH_FMT_DOUBLE, nullptr, &CounterValue);
	if (PdhStatus != ERROR_SUCCESS)
	{
		XIAO_LOG(Warning, TEXT("PdhGetFormattedCounterValue failed. Error code: %d"), PdhStatus);
	}

	CoreCpuUtilization[0] = FMath::RoundToInt(CounterValue.doubleValue);

	LastQueryTime = FPlatformTime::Seconds();
	return true;
}