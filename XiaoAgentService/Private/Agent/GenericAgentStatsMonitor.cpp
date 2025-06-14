/**
  * @author cxx2020@outlook.com
  * @datetime 2023 -10:13 PM
 */
#include "GenericAgentStatsMonitor.h"
#include "HAL/PlatformProcess.h"
#include "HAL/PlatformMemory.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/Paths.h"

bool FGenericAgentStatsMonitor::GetAvailableMemory(TTuple<uint64, uint32>& OutAvailableMemory)
{
	const FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
	OutAvailableMemory.Key = MemStats.TotalPhysicalGB - (MemStats.AvailablePhysical / 1024 / 1024 / 1024);
	OutAvailableMemory.Value = MemStats.TotalPhysicalGB;
	return true;
}

bool FGenericAgentStatsMonitor::GetAvailableSpace(TTuple<uint64, uint64>& OutDiskSpace, const FString& InTargetDrive)
{
	FText Text;
	if (FPaths::ValidatePath(InTargetDrive, &Text) && !FPaths::DirectoryExists(InTargetDrive))
	{
		IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
		if (PlatformFile.CreateDirectoryTree(*InTargetDrive))
		{
			XIAO_LOG(Log, TEXT("Directory create success: %s"), *InTargetDrive);
		}
		else
		{
			XIAO_LOG(Error, TEXT("Directory create failed: %s"), *InTargetDrive);
		}
	}

	uint64 TotalNumberOfBytes, NumberOfFreeBytes;
	if (FPlatformMisc::GetDiskTotalAndFreeSpace(InTargetDrive, TotalNumberOfBytes, NumberOfFreeBytes))
	{
		OutDiskSpace.Value = static_cast<uint32>(TotalNumberOfBytes / static_cast<uint64>(1024 * 1024 * 1024));
		OutDiskSpace.Key = OutDiskSpace.Value - (static_cast<uint32>(NumberOfFreeBytes / static_cast<uint64>(1024 * 1024 * 1024)));
		return true;
	}
	const uint32 ErrorCode = FPlatformMisc::GetLastError();
	TCHAR ErrorBuffer[2048];
	FPlatformMisc::GetSystemErrorMessage(ErrorBuffer, 2048, ErrorCode);
	XIAO_LOG(Error, TEXT("GetDiskTotalAndFreeSpace TargetDrive::%s failed with error code::%u (%s)"), *InTargetDrive, ErrorCode, ErrorBuffer);
	return false;
}