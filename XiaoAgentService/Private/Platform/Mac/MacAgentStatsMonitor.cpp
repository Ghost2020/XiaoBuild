#include "MacAgentStatsMonitor.h"
#include "HAL/PlatformMisc.h"
#include "HAL/PlatformTime.h"
#include "XiaoLog.h"
#include <mach/mach.h>
#include <unistd.h>
#include <ifaddrs.h>
#include <net/if.h>
#include <net/if_dl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


FMacAgentStatsMonitor::FMacAgentStatsMonitor()
{
	XIAO_LOG(Display, TEXT("MacAgentStatsMonitor called!"));
}

FMacAgentStatsMonitor::~FMacAgentStatsMonitor()
{
	Deinitialize();
}

void FMacAgentStatsMonitor::Initialize()
{
	mach_port_t masterPort;
	IOMasterPort(MACH_PORT_NULL, &masterPort);

	CpuService = IOServiceGetMatchingService(masterPort, IOServiceMatching("AppleSMC"));
	if (!CpuService)
	{
		XIAO_LOG(Error, TEXT("No AppleSMC device found."));
		return;
	}
	IORegistryEntryCreateCFProperties(CpuService, &CpuProperties, kCFAllocatorDefault, kNilOptions);

	io_iterator_t iterator;
	kern_return_t result = IOServiceGetMatchingServices(masterPort, IOServiceMatching("IOPCIDevice"), &iterator);
	if (result == KERN_SUCCESS)
	{
		bool find = false;
		while ((GpuService = IOIteratorNext(iterator)))
		{
			CFTypeRef classCode = IORegistryEntryCreateCFProperty(GpuService, CFSTR("class-code"), kCFAllocatorDefault, 0);
			if (classCode && CFGetTypeID(classCode) == CFDataGetTypeID())
			{
				const UInt8* code = CFDataGetBytePtr((CFDataRef)classCode);
				if (code[0] == 0x03 && code[1] == 0x00 && code[2] == 0x00)
				{
					find = true;
				}
			}
			if (classCode)
			{
				CFRelease(classCode);
			}
			if (find)
			{
				break;
			}
			IOObjectRelease(GpuService);
		}
		IOObjectRelease(iterator);
	}
	if (!GpuService)
	{
		XIAO_LOG(Error, TEXT("Failed to find Gpu Service!"));
		return;
	}
	IORegistryEntryCreateCFProperties(GpuService, &GpuProperties, kCFAllocatorDefault, kNilOptions);

	bIsInitialized = true;
	XIAO_LOG(Verbose, TEXT("AgentStatsMonitor Initialize Finish."));
}

void FMacAgentStatsMonitor::Deinitialize()
{
	XIAO_LOG(Display, TEXT("Deinitialize called!"));
	if (CpuProperties)
	{
		CFRelease(CpuProperties);
	}
	if (CpuService)
	{
		IOObjectRelease(CpuService);
	}
	if (GpuProperties)
	{
		CFRelease(GpuProperties);
	}
	if (GpuService)
	{
		IOObjectRelease(GpuService);
	}
}

bool FMacAgentStatsMonitor::CollectQueryData()
{
	return true;
}

float FMacAgentStatsMonitor::GetCpuUtilization()
{
	mach_msg_type_number_t count = HOST_CPU_LOAD_INFO_COUNT;
	host_cpu_load_info_data_t cpuLoad;
	const kern_return_t kr = host_statistics(mach_host_self(), HOST_CPU_LOAD_INFO, (host_info_t)&cpuLoad, &count);
	if (kr != KERN_SUCCESS)
	{
		XIAO_LOG(Error, TEXT("Failed to get CPU load info"));
		return 100.0f;
	}

	uint64_t totalTicks = 0;
	const uint64_t totalIdleTicks = cpuLoad.cpu_ticks[CPU_STATE_IDLE];
	for (int i = 0; i < CPU_STATE_MAX; i++)
	{
		totalTicks += cpuLoad.cpu_ticks[i];
	}

	const float IdleTicks = static_cast<float>(totalIdleTicks - LastIdleTicks);
	const float Ticks = static_cast<float>(totalTicks - LastTotalTicks);
	const float Utilization = (1.0f - (IdleTicks / Ticks)) * 100.0f;

	LastTotalTicks = totalTicks;
	LastIdleTicks = totalIdleTicks;

	return Utilization;
}

float FMacAgentStatsMonitor::GetCpuTemperature()
{
	float temperature = -1.0f;
	if (CpuProperties)
	{
		CFNumberRef temp = (CFNumberRef)CFDictionaryGetValue(CpuProperties, CFSTR("CPU0Thermistor"));
		if (temp)
		{
			CFNumberGetValue(temp, kCFNumberFloatType, &temperature);
			return temperature;
		}
	}

	return temperature;
}

float FMacAgentStatsMonitor::GetNetworkUtilization()
{
	struct ifaddrs* ifap, * ifa;

	if (getifaddrs(&ifap) == -1)
	{
		perror("getifaddrs");
		return 0.0f;
	}

	double SendBytes = 0.0f;
	double RecvBytes = 0.0f;
	for (ifa = ifap; ifa; ifa = ifa->ifa_next)
	{
		if (ifa->ifa_data == nullptr)
		{
			continue;
		}

		struct if_data* ifd = (struct if_data*)ifa->ifa_data;
		SendBytes += ifd->ifi_obytes / 1024.0f;
		RecvBytes += ifd->ifi_ibytes / 1024.0f;
	}

	SendSpeed = (SendBytes - LastSendBytes) / 1024.f / 1024.0f;
	RecvSpeed = (RecvBytes - LastRecvBytes) / 1024.f / 1024.0f;

	if (SendSpeed > MaxSpeed)
	{
		MaxSpeed = SendSpeed;
	}
	if (RecvSpeed > MaxSpeed)
	{
		MaxSpeed = RecvSpeed;
	}

	LastSendBytes = SendBytes;
	LastRecvBytes = RecvBytes;

	freeifaddrs(ifap);
	return FMath::Abs((FMath::Abs(RecvSpeed) + FMath::Abs(SendSpeed)) / 2.0f / MaxSpeed);
}

float FMacAgentStatsMonitor::GetNetworkSpeed()
{
	return (FMath::Abs(RecvSpeed) + FMath::Abs(SendSpeed)) / 2.0f;
}

float FMacAgentStatsMonitor::GetGpuUtilization()
{
	if (GpuProperties)
	{
		CFTypeRef gpuLoad = CFDictionaryGetValue(GpuProperties, CFSTR("GPULoad"));
		if (gpuLoad && CFGetTypeID(gpuLoad) == CFNumberGetTypeID())
		{
			double utiliValue;
			CFNumberGetValue((CFNumberRef)gpuLoad, kCFNumberDoubleType, &utiliValue);
			return utiliValue;
		}
	}
	return -1.f;
}

float FMacAgentStatsMonitor::GetGpuTemperature()
{
	if (GpuProperties)
	{
		CFTypeRef temperature = CFDictionaryGetValue(GpuProperties, CFSTR("temperature"));
		if (temperature && CFGetTypeID(temperature) == CFNumberGetTypeID())
		{
			double tempValue;
			CFNumberGetValue((CFNumberRef)temperature, kCFNumberDoubleType, &tempValue);
			return tempValue;
		}
	}
	return -1.f;
}

bool FMacAgentStatsMonitor::GetHelperCache(TTuple<int16, int16>& OutGpuUtilization)
{
	return false;
}

bool FMacAgentStatsMonitor::GetUpDownTime(TTuple<bool, FString>& OutUpDownTime)
{
	XIAO_LOG(Display, TEXT("GetUpDownTime Not implement!"));
	return false;
}
