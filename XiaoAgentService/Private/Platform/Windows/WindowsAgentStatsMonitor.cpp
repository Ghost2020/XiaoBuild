#include "WindowsAgentStatsMonitor.h"
#include "HAL/PlatformMisc.h"
#include "HAL/PlatformTime.h"
#include <dxgi.h>
#include "XiaoLog.h"

static FString PERFM_PATH_CPU_UTILITY(TEXT("\\Processor Information(_Total)\\% Processor Utility"));
static FString PERFM_PATH_DISK_UTILITY(TEXT("\\PhysicalDisk(_Total)\\% Disk Time"));
static FString PERFM_PATH_NETWORK_BAND_WIDTH(TEXT("\\Network Interface(*)\\Current Bandwidth"));
static FString PERFM_PATH_NETWORK_RECV_RATE(TEXT("\\Network Interface(*)\\Bytes Received/sec"));
static FString PERFM_PATH_NETWORK_SENT_RATE(TEXT("\\Network Interface(*)\\Bytes Sent/sec"));
static FString PERFM_PATH_GPU_3D_UTILITY(TEXT("\\GPU Engine(*_3D)\\Utilization Percentage"));
static FString PERFM_PATH_GPU_MEMORY_DEDICATED_USAGE_UTILITY(TEXT("\\GPU Adapter Memory(*)\\Dedicated Usage"));
static FString PERFM_PATH_GPU_MEMORY_SHARE_USAGE_UTILITY(TEXT("\\GPU Adapter Memory(*)\\Shared Usage"));

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
	if (!AddCounter(PERFM_PATH_CPU_UTILITY))
	{
		return;
	}
	if (!AddCounter(PERFM_PATH_DISK_UTILITY))
	{
		return;
	}
	if (!AddCounter(PERFM_PATH_NETWORK_BAND_WIDTH))
	{
		return;
	}
	if (!AddCounter(PERFM_PATH_NETWORK_RECV_RATE))
	{
		return;
	}
	if (!AddCounter(PERFM_PATH_NETWORK_SENT_RATE))
	{
		return;
	}
	if (!AddCounter(PERFM_PATH_GPU_3D_UTILITY))
	{
		return;
	}
	if (!AddCounter(PERFM_PATH_GPU_MEMORY_DEDICATED_USAGE_UTILITY))
	{
		return;
	}
	if (!AddCounter(PERFM_PATH_GPU_MEMORY_SHARE_USAGE_UTILITY))
	{
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

float FWindowsAgentStatsMonitor::GetDiskUtilization()
{
	PDH_FMT_COUNTERVALUE DiskUtilityValue;
	GetFormattedCounterArray(Counter2Handle[PERFM_PATH_DISK_UTILITY], PDH_FMT_DOUBLE, &DiskUtilityValue);
	return 100.0f - DiskUtilityValue.doubleValue;
}

float FWindowsAgentStatsMonitor::GetNetworkUtilization()
{
	PDH_FMT_COUNTERVALUE CounterBandWidth;
	GetFormattedCounterArray(Counter2Handle[PERFM_PATH_NETWORK_BAND_WIDTH], PDH_FMT_DOUBLE, &CounterBandWidth);
	LastBandWidth = CounterBandWidth.doubleValue;

	return (1.0f - (LastRecvSpeed+LastSendSpeed)*8.0f/LastBandWidth)*100.0f;
}

float FWindowsAgentStatsMonitor::GetNetworkSpeed()
{
	PDH_FMT_COUNTERVALUE RecvValue = { 0 };
	GetFormattedCounterArray(Counter2Handle[PERFM_PATH_NETWORK_RECV_RATE], PDH_FMT_DOUBLE, &RecvValue);
	LastRecvSpeed = RecvValue.doubleValue;

	PDH_FMT_COUNTERVALUE SentValue = { 0 };
	GetFormattedCounterArray(Counter2Handle[PERFM_PATH_NETWORK_SENT_RATE], PDH_FMT_DOUBLE, &SentValue);
	LastSendSpeed = SentValue.doubleValue;

	return (FMath::Abs(LastRecvSpeed) + FMath::Abs(LastSendSpeed)) / 2.0f;
}

FString FWindowsAgentStatsMonitor::GetGPUDesc()
{
	TRefCountPtr<IDXGIFactory1> DXGIFactory1;
	if (CreateDXGIFactory1(__uuidof(IDXGIFactory1), (void**)&DXGIFactory1) != S_OK || !DXGIFactory1)
	{
		return {};
	}

	DXGI_ADAPTER_DESC BestDesc = {};
	TRefCountPtr<IDXGIAdapter> TempAdapter;
	for (uint32 AdapterIndex = 0; DXGIFactory1->EnumAdapters(AdapterIndex, TempAdapter.GetInitReference()) != DXGI_ERROR_NOT_FOUND; ++AdapterIndex)
	{
		if (TempAdapter)
		{
			DXGI_ADAPTER_DESC Desc;
			TempAdapter->GetDesc(&Desc);

			if (Desc.DedicatedVideoMemory > BestDesc.DedicatedVideoMemory || AdapterIndex == 0)
			{
				BestDesc = Desc;
			}
		}
	}

	if (GPUSystemMemory == 0)
	{
		GPUSystemMemory = BestDesc.DedicatedVideoMemory;
		// XIAO_LOG(Log, TEXT("GPUSystemMemory::%f"), GPUSystemMemory / SMegabyteNum);
	}
	return BestDesc.Description;
}

float FWindowsAgentStatsMonitor::GetGpuUtilization()
{
	PDH_FMT_COUNTERVALUE Value = { 0 };
	GetFormattedCounterArray(Counter2Handle[PERFM_PATH_GPU_3D_UTILITY], PDH_FMT_DOUBLE, &Value);
	return (1.0f-Value.doubleValue)*100.0f;
}

float FWindowsAgentStatsMonitor::GetGpuTemperature()
{
	return 0.0f;
}

bool FWindowsAgentStatsMonitor::GetGpuMemoryUsageUtility(TTuple<float, float>& OutMemoryUsageUtililty)
{
	PDH_FMT_COUNTERVALUE Value = { 0 };
	GetFormattedCounterArray(Counter2Handle[PERFM_PATH_GPU_MEMORY_DEDICATED_USAGE_UTILITY], PDH_FMT_DOUBLE, &Value);
	OutMemoryUsageUtililty.Key = Value.doubleValue / SMegabyteNum;
	GetFormattedCounterArray(Counter2Handle[PERFM_PATH_GPU_MEMORY_SHARE_USAGE_UTILITY], PDH_FMT_DOUBLE, &Value);
	OutMemoryUsageUtililty.Value = Value.doubleValue/ SMegabyteNum;
	return true;
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
	const PDH_STATUS PdhStatus = ::PdhGetFormattedCounterValue(Counter2Handle[PERFM_PATH_CPU_UTILITY], PDH_FMT_DOUBLE, nullptr, &CounterValue);
	if (PdhStatus != ERROR_SUCCESS)
	{
		XIAO_LOG(Warning, TEXT("PdhGetFormattedCounterValue failed. Error code: %d"), PdhStatus);
	}

	CoreCpuUtilization[0] = FMath::Clamp(FMath::RoundToInt(CounterValue.doubleValue), 0, 100);

	LastQueryTime = FPlatformTime::Seconds();
	return true;
}

bool FWindowsAgentStatsMonitor::AddCounter(const FString& InCounterName)
{
	PDH_HCOUNTER CounterHandle;
	const PDH_STATUS PdhStatus = ::PdhAddEnglishCounter(QueryHandle, *InCounterName, 0, &CounterHandle);
	if (PdhStatus != ERROR_SUCCESS)
	{
		XIAO_LOG(Warning, TEXT("PdhAddEnglishCounter %s failed. Error code: %d"), *InCounterName, PdhStatus);
		return false;
	}
	Counter2Handle.Add(InCounterName, CounterHandle);
	return true;
}
