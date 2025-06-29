/**
  * @author cxx2020@outlook.com
  * @date 10:24 PM
 */
#pragma once

#include "CoreMinimal.h"
#include "HAL/PlatformProcess.h"
#include "HAL/PlatformFileManager.h"
#include "Async/Async.h"

#ifdef DESKTOPPLATFORM_API
#include "Framework/Application/SlateApplication.h"
#include "DesktopPlatformModule.h"
#endif
#include "Misc/Paths.h"
#include "Misc/AES.h"
#include "Misc/Base64.h"
#include "Misc/KeyChainUtilities.h"
#include "Misc/FileHelper.h"
#include "XiaoShareField.h"
#include "XiaoLog.h"

#ifdef WITH_UNREALPNG
#include "IImageWrapperModule.h"
#include "RenderUtils.h"
#endif

#if PLATFORM_WINDOWS
#ifdef DESKTOPPLATFORM_API
	#include "Windows/WindowsApplication.h"
	#include <winternl.h>
	#pragma comment(lib, "ntdll.lib")
#endif
#include "Windows/WindowsHWrapper.h"
#include "Windows/PreWindowsApi.h"
#include <shellapi.h>
#include "Windows.h"
#include "WinUser.h"
#include <tlhelp32.h>
#include <Psapi.h>

#pragma comment(lib, "Secur32.lib")
#else
#include <unistd.h>
#endif

#define LOCTEXT_NAMESPACE "XiaoShare"

// Windows版本枚举
enum struct EWinVersion : uint8
{
	Win_None = 0,
	Win_XP,
	Win_2003,
	Win_Vista,
	Win_2008,
	Win_7,
	Win_8,
	Win_8_1,
	Win_10,
	Win_11,
};

// 机器硬件配置信息
struct FMachineConfiguration
{
	// CPU 个数
	uint32 CpuCount;
	// 总共逻辑核心数
	uint32 LogicCoreCount;
	// 物理地址
	FString MacAddress;
	// IP地址
	FString IpAddress;
	// 主机名称
	FString HostName;
	// 内存大小/GB
	uint32 MemorySize;
	// 显存大小/GB
	uint32 VideoMemorySize;
	// 是否开启超频
	bool bSupportHyperThread;

	explicit FMachineConfiguration()
		: CpuCount(1)
		, LogicCoreCount(1)
		, MemorySize(1)
		, VideoMemorySize(1)
		, bSupportHyperThread(false)
	{}

	explicit FMachineConfiguration(
		const uint32 InCpuCount,
		const uint32 InLogicCoreCount,
		const FString InMacAddress,
		const FString InIpAddress,
		const FString InHostName)
		: CpuCount(InCpuCount)
		, LogicCoreCount(InLogicCoreCount)
		, MacAddress(InMacAddress)
		, IpAddress(InIpAddress)
		, HostName(InHostName)
		, MemorySize(0)
		, VideoMemorySize(0)
		, bSupportHyperThread(false)
	{
	}
};

// 机器当前的性能指标
struct FMachinePerformance
{
	// CPU使用百分比
	float CpuPercentage;
	// 内存使用百分比
	float MemoryPercentage;
	// 现存使用百分比
	float VideoMemoryPercentage;
	// IO读写百分比
	float DiskPercentage;
	// 网络上传下载百分比
	float NetworkPercentage;

	explicit FMachinePerformance()
		: CpuPercentage(0.0)
		, MemoryPercentage(0.0)
		, VideoMemoryPercentage(0.0)
		, DiskPercentage(0.0)
		, NetworkPercentage(0.0)
	{}
	
	explicit FMachinePerformance(
		const float InCpuPercentage,
		const float InMemoryPercentage,
		const float InVideoMemoryPercentage,
		const float DiskPercentage,
		const float NetworkPercentage)
		: CpuPercentage(InCpuPercentage)
		, MemoryPercentage(InMemoryPercentage)
		, VideoMemoryPercentage(InVideoMemoryPercentage)
		, DiskPercentage(DiskPercentage)
		, NetworkPercentage(NetworkPercentage)
	{}
};

// 服务描述
struct FServiceDesc
{
	FString ServiceName; FString DisplayName;
	uint64 ServiceType; uint64 StartType;
	FString Description; FString LoadOrderGroup; uint32 Dependencies;
	FString User; FString Password;

	explicit FServiceDesc()
		:ServiceType(0),StartType(0), Dependencies(0)
	{}

	explicit FServiceDesc(
		const FString& InServiceName, const FString& InDisplayName=TEXT(""),
		const uint64 InServiceType=0, const uint64 InStartType=0,
		const FString& InDescription=TEXT(""), const FString& InLoadOrderGroup=TEXT(""),
		const FString& InUser=TEXT(""), const FString& InPassword=TEXT(""))
		: ServiceName(InServiceName),DisplayName(InDisplayName),
		ServiceType(InServiceType), StartType(InStartType),
		Description(InDescription), LoadOrderGroup(InLoadOrderGroup), Dependencies(0),
		User(InUser), Password(InPassword)
	{}

	FServiceDesc& operator=(const FServiceDesc& InDesc)
	{
		this->ServiceName = InDesc.ServiceName;
		this->DisplayName = InDesc.DisplayName;
		this->ServiceType = InDesc.ServiceType;
		this->StartType = InDesc.StartType;
		this->LoadOrderGroup = InDesc.LoadOrderGroup;
		this->Description = InDesc.Description;
		this->Dependencies = InDesc.Dependencies;
		this->User = InDesc.User;
		this->Password = InDesc.Password;

		return *this;
	}
};

// 机器状态
enum EAgentStatus : uint8
{
	Status_Ready = 0,
	Status_Initiating = 1,
	Status_Helping = 2 ,
	Status_Updating = 3,
	Status_Offline = 4,
	Status_Stopped = 5,
	Status_UnCondi = 6,
	Status_UnReached = 7,

	Status_Undefined = 255
};

enum EBuildGroup : uint8
{
	Group_Default = 0
};

enum EBuildPriority : uint8
{
	Priority_Idle = 0,
	Priority_BlowNormal,
	Priority_Normal,
	Priority_AboveNormal,
	Priority_High,
	Priority_Realtime
};

enum EAgentHelperPermissions
{
	AllowedToEnableDisableAsHelper,
	NotAllowedToEnableDisable
};

enum ELogLevel : uint8
{
	Level_Minimal = 0,
	Level_Basic,
	Level_Intermeidate,
	Level_Extended,
	Level_Detailed
};

struct FAgentDesc
{
	FAgentDesc(const uint16 InIndex, const FString& InId, const bool InbEnable,
		const FString& InName, const EAgentStatus InStatus, const FString& InDesc, 
		const uint16& InLogicCores)
		: Index(InIndex), Id(InId), bEnable(InbEnable)
		, Name(InName), Description(InDesc), Status(InStatus) 
		, LogicCores(InLogicCores)
	{}

	FAgentDesc(const uint16 InIndex, const FString& InId, const bool InbEnable,
	const bool InbBuildCache, const uint16 InAvaliableCpu,
	const bool InbInitiator, const bool InbHelper,
	const FString InName, const FString InDesc,
	const int InStatus, const FString InGroup,
	const FDateTime InLastLogin,
	const uint16 InAvalibaleMemory, const uint16 InTotalMemory,
	const uint32 InUsedHelpersCache, const uint32 InTotalHepersCache,
	const uint16 InRegisteredHelpCores, const uint16 InFreeDiskSpace,
	const int InBuildPriority,
	const FString InCPUInfo, const FString InGPUInfo,
	const FString InIP, const FString InLoggedOnUser,
	const int InAssignPriority,
	const float InAvaliableDiskSpace, const float InTotalDiskSpace,
	const uint16 InPort, const uint16 InLogicCores,
	const FString InMACAddress, const FString InNetwork,
	const FString InOSSystem, const FString InRoutingIP,
	const uint16 InPhysicalCores, const int InLogLevel,
	const bool InbUp, const FString InUpDownTime)
		: Index(InIndex), Id(InId), bEnable(InbEnable)
		, bBuildCache(InbBuildCache), AvaliableCPU(InAvaliableCpu)
		, bInitiator(InbInitiator), bHelper(InbHelper)
		, Name(InName), Description(InDesc)
		, Status(static_cast<EAgentStatus>(InStatus))
		, Group(InGroup)
		, LastConnected(InLastLogin)
		, AvalibaleMemory(InAvalibaleMemory), TotalMemory(InTotalMemory)
		, UsedHelpersCache(InUsedHelpersCache), TotalHepersCache(InTotalHepersCache)
		, RegisteredHelpCores(InRegisteredHelpCores), FreeDiskSpace(InFreeDiskSpace)
		, BuildPriority(static_cast<EBuildPriority>(InBuildPriority))
		, CPUInfo(InCPUInfo), GPUInfo(InGPUInfo)
		, IP(InIP), LoggedOnUser(InLoggedOnUser)
		, AssignPriority(static_cast<EBuildPriority>(InAssignPriority))
		, AvaliableDiskSpace(InAvaliableDiskSpace), TotalDiskSpace(InTotalDiskSpace)
		, Port(InPort), LogicCores(InLogicCores)
		, MACAddress(InMACAddress), Network(InNetwork)
		, OSSystem(InOSSystem), RoutingIP(InRoutingIP)
		, PhysicalCores(InPhysicalCores)
		, LogLevel(static_cast<ELogLevel>(InLogLevel))
		, bUp(InbUp), UpDownTime(InUpDownTime)
	{}
	
	uint16 Index = 1;
	FString Id;
	bool bEnable = true;
	bool bBuildCache = false;
	uint16 AvaliableCPU = 80;
	bool bInitiator = true;
	bool bHelper = true;

	FString Name = TEXT("Ghost");
	FString Description = TEXT("Master");
	EAgentStatus Status;
	FString Group = "Default";
	
	FDateTime LastConnected = FDateTime::Now();

	uint16 AvalibaleMemory = 10; //单位GB
	uint16 TotalMemory = 32; //单位GB
	
	uint32 UsedHelpersCache = 0; //单位MB
	uint32 TotalHepersCache = 20480; //单位MB

	uint16 RegisteredHelpCores = 1;
	uint16 FreeDiskSpace = 0; // 单位GB
	
	EBuildPriority BuildPriority = EBuildPriority::Priority_Normal;
	FString CPUInfo = TEXT("Interl(R) Core(TM)");
	FString GPUInfo = TEXT("GPU 0 NVIDIA GeForce GTX 1060 6GB");
	
	FString IP = TEXT("127.0.0.1");
	FString LoggedOnUser = TEXT("Admin");
	EBuildPriority AssignPriority = EBuildPriority::Priority_Normal;

	float AvaliableDiskSpace = 1.2f; // 单位GB
	float TotalDiskSpace = 118.3f; // 单位GB

	uint16 Port = 23230;
	uint16 LogicCores = 1;

	FString MACAddress = TEXT("N/A");
	FString Network = TEXT("Local Connection (1000 Gbps)");
	FString OSSystem = "Windows";
	FString RoutingIP = "127.0.01";
	
	uint16 PhysicalCores = 1;
	ELogLevel LogLevel = ELogLevel::Level_Basic;
	
	bool bUp = true;
	FString UpDownTime = "02d 03h 35m"; // 02d 03h 35m || 07h 54m 40s
};

// Online Agent Activity
struct FOnlineAgentActivity
{
	uint16 OnlineMachineNum = 1;
	uint16 BusyNum = 0;
	uint16 IdleNum = 1;

	uint16 BusyCoresNum = 0;
	uint16 HelperNum = 1;
	uint16 InitiatorNum = 0;

	uint32 BuildBum = 0;

	uint16 InitiatorsNum = 10;
	uint16 AssignedNum = 1;
	uint16 AvailableNum = 1;

	uint16 HelperCoresNum = 40;
	uint16 HelperCorePoolNum = 6;
};

// License Usage
struct FLicenseUsage
{
	uint16 InitiatorNum = 10;
	uint16 AssignedNum = 1;
	uint16 AvailableNum = 9;
	
	uint16 HelperCoreNum = 40;
	uint16 HelperCorePoolNum = 6;
};

// Build Groups
struct FBuildGroup
{
	explicit FBuildGroup(const FString& InName, const uint32 InAssignedNum)
		: Name(InName), AssignedNum(InAssignedNum)
	{}
	FString Name = TEXT("Default");
	uint32 AssignedNum = 1;
};

static const FText GPriorityIdle = LOCTEXT("Priority_Idle_Text", "1-Idle");
static const FText GPriorityBlowNormal = LOCTEXT("Priority_BlowNormal_Text", "2-Blow Normal");
static const FText GPriorityNormal = LOCTEXT("Priority_Normal_Text", "3-Normal");
static const FText GPriorityAboveNormal = LOCTEXT("Priority_AboveNormal_Text", "4-Above Normal");
static const FText GPriorityHigh = LOCTEXT("Priority_High_Text", "5-High");
static const FText GPriorityRealtime = LOCTEXT("Priority_Realtime_Text", "6-Realtime");
static const FText GMultiValue = LOCTEXT("Multi_Text", "Multi");

inline FString GLocalization = TEXT("zh-CN");

static FString GetXiaoHomePath()
{
#if PLATFORM_MAC
	return TEXT("/Applications/XiaoApp.app/Contents/UE/Engine");
#else
	return FPlatformMisc::GetEnvironmentVariable(TEXT("XIAO_HOME"));
#endif
}

static bool IsValidDir()
{
	// 所在目录判断
	const FString WorkingDir = FPlatformProcess::GetCurrentWorkingDirectory();
	const FString EngineDir = FPaths::ConvertRelativePathToFull(FPaths::Combine(WorkingDir, TEXT("../../")));
	if (FPaths::GetPathLeaf(EngineDir) != "Engine")
	{
		XIAO_LOG(Error, TEXT("Not valid working directory!::%s"), *EngineDir);
		return false;
	}

	// 必要文件判断
	auto& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	TArray<FString> Files;
	PlatformFile.FindFiles(Files, *WorkingDir, TEXT(""));
	TSet<FString> ExistApps;
	for (const auto& File : Files)
	{
		ExistApps.Add(FPaths::GetBaseFilename(File));
	}

	// 配置文件
	const FString ConfigDir = FPaths::ConvertRelativePathToFull(FPaths::Combine(EngineDir, TEXT("Config")));
	const FString CacheConf = FPaths::Combine(ConfigDir, TEXT("cache.conf"));
	if (!FPaths::FileExists(CacheConf))
	{
		XIAO_LOG(Error, TEXT("cache config file not exist!"));
		GLog->Flush(); 
		return false;
	}
	const FString ConfigConf = FPaths::Combine(ConfigDir, TEXT("config.json"));
	if (!FPaths::FileExists(ConfigConf))
	{
		XIAO_LOG(Error, TEXT("system config file not exist!"));
		GLog->Flush();
		return false;
	}

	return true;
}

static bool IsValidPeriod()
{
	/*const FDateTime Now = FDateTime::Now();
	FDateTime ExpirationTime;
	if (!FDateTime::Parse(AppDetails::GExpiration, ExpirationTime))
	{
		XIAO_LOG(Error, TEXT("FDateTime::Parse failed!"));
		return false;
	}

	return Now <= ExpirationTime;*/
	return true;
}

static bool IsRunAsAdmin()
{
#if PLATFORM_WINDOWS
	BOOL BIsRunAsAdmin = false;
	PSID PAdminSid = nullptr;

	SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
	if (AllocateAndInitializeSid(&NtAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &PAdminSid))
	{
		if (!CheckTokenMembership(nullptr, PAdminSid, &BIsRunAsAdmin))
		{
			BIsRunAsAdmin = -1;
		}

		FreeSid(PAdminSid);
	}

	return BIsRunAsAdmin == 1;
#else
	return geteuid() == 0;
#endif

	return false;
}

static FText Priority2Text(const EBuildPriority InPriority)
{
	switch (InPriority)
	{
	case Priority_Idle:			return GPriorityIdle;
	case Priority_BlowNormal:	return GPriorityBlowNormal;
	case Priority_Normal:		return GPriorityNormal;
	case Priority_AboveNormal:	return GPriorityAboveNormal;
	case Priority_High:			return GPriorityHigh;
	case Priority_Realtime:		return GPriorityRealtime;
	}
	return FText::GetEmpty();
}

static uint32 GetLogicProcessorNum()
{
	uint32 OutLogicalProcessorCount = 0;
#if PLATFORM_WINDOWS
	uint8* BufferPtr = nullptr;
	DWORD BufferBytes = 0;
	
	if (false == GetLogicalProcessorInformationEx(RelationAll, (PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX)BufferPtr, &BufferBytes))
	{
		if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
		{
			BufferPtr = reinterpret_cast<uint8*>(FMemory::Malloc(BufferBytes));

			if (GetLogicalProcessorInformationEx(RelationAll, (PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX)BufferPtr, &BufferBytes))
			{
				uint8* InfoPtr = BufferPtr;

				while (InfoPtr < BufferPtr + BufferBytes)
				{
					PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX ProcessorInfo = (PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX)InfoPtr;

					if (nullptr == ProcessorInfo)
					{
						break;
					}

					if (ProcessorInfo->Relationship == RelationProcessorCore)
					{
						for (int GroupIdx = 0; GroupIdx < ProcessorInfo->Processor.GroupCount; ++GroupIdx)
						{
							OutLogicalProcessorCount += FMath::CountBits(ProcessorInfo->Processor.GroupMask[GroupIdx].Mask);
						}
					}

					InfoPtr += ProcessorInfo->Size;
				}
			}

			FMemory::Free(BufferPtr);
		}
	}
#else
	OutLogicalProcessorCount = FPlatformMisc::NumberOfCoresIncludingHyperthreads();
#endif

	return OutLogicalProcessorCount;
}


static const FText GLogLevelIdle = LOCTEXT("Level_Minimal_Text", "1-Idle");
static const FText GLogLevelBasic = LOCTEXT("Level_Basic_Text", "2-Basic");
static const FText GLogLevelIntermeidate = LOCTEXT("Level_Intermeidate_Text", "3-Intermeidate");
static const FText GLogLevelExtended = LOCTEXT("Level_Extended_Text", "4-Extended");
static const FText GLogLevelDetailed = LOCTEXT("Level_Detailed_Text", "5-Detailed");

static const FText GLogRoleGridAmin = LOCTEXT("Role_GridAmin_Text", "Grid Admin");
static const FText GLogRoleGroupManager = LOCTEXT("Role_GroupManager_Text", "Group Manager");
static const FText GLogRoleViewer = LOCTEXT("Role_Viewer_Text", "Viewer");

static const FText GMaster = LOCTEXT("BackupMaster_Text", "Master");
static const FText GSlave = LOCTEXT("BackupSlave_Text", "Slave");

static const FText GActiveText = LOCTEXT("Active_Text", "Activated");
static const FText GInActiveText = LOCTEXT("InActive_Text", "Not activated");

static FText LogLevel2Text(const ELogLevel InLevel)
{
	switch (InLevel)
	{
		case Level_Minimal:			return GLogLevelIdle;
		case Level_Basic:			return GLogLevelBasic;
		case Level_Intermeidate:    return GLogLevelIntermeidate;
		case Level_Extended:		return GLogLevelExtended;
		case Level_Detailed:		return GLogLevelDetailed;
	}
	return FText::GetEmpty();
}

enum EUserRole
{
	Role_GridAdnim = 0,
	Role_GroupManager,
	Role_Viewer
};

enum EUserStatus
{
	Active = 0,
	Inactive
};

// 工程类型
enum EProjectType : uint8
{
	Type_Unknwon = 0,	// 不知晓的类型
	Type_UE_Engine,		// 引擎编译
	Type_UE_Client,		// 客户端编译
	Type_UE_Server,		// 服务器编译
	Type_UE_Program,	// Program编译
	Type_UE_Material,	// 材质编译
	Type_UE_BakeLight,	// 光照计算
	Type_UE_Animation,	// 动画压缩
	Type_UE_Cook		// 烘培
};

static void SetCurrentProcessPriority(const int32 InPriority)
{
	// 设置为高优先级
	const uint32 ProcessId = FPlatformProcess::GetCurrentProcessId();
	FProcHandle ProcHandle = FPlatformProcess::OpenProcess(ProcessId);
	if (ProcHandle.IsValid())
	{
#if PLATFORM_WINDOWS
		FPlatformProcess::SetProcPriority(ProcHandle, InPriority);
#endif
	}
}

static void SetProcessDisplyName(const FString& InDisplayName)
{
	XIAO_LOG(Warning, TEXT("SetProcessDisplyName Not implement!"));
#if PLATFORM_WINDOWS
	//HMODULE hKernel32 = GetModuleHandle(L"kernel32.dll");
	//if (!hKernel32)
	//{
	//	XIAO_LOG(Error, TEXT("Failed to load kernel32.dll"));
	//	return;
	//}

	/*typedef HRESULT(WINAPI* SetProcDescFunc)(HANDLE, PCWSTR);
	if (auto SetProcessDescription = reinterpret_cast<SetProcDescFunc>(GetProcAddress(hKernel32, "SetProcessDescription")))
	{
		HRESULT hr = SetProcessDescription(GetCurrentProcess(), *InDisplayName);
		if (SUCCEEDED(hr))
		{
			XIAO_LOG(Log, TEXT("Successfully set process name to: %s") ,*InDisplayName);
		}
		else
		{
			XIAO_LOG(Error, TEXT("Failed to set process name. HRESULT: 0x"), hr);
		}
	}
	else
	{
		XIAO_LOG(Error, TEXT("SetProcessDescription API not available on this system."));
	}*/
#endif
}

static FText BuildType2Text(const EProjectType InType)
{
	switch (InType)
	{
		case Type_Unknwon:		return LOCTEXT("Unknwon_Text", "Unknwon");
		case Type_UE_Engine:	return LOCTEXT("UE_Engine_Text", "UE Engine");
		case Type_UE_Client:	return LOCTEXT("UE_Client_Text", "UE Client");
		case Type_UE_Server:	return LOCTEXT("UE_Server_Text", "UE Server");
		case Type_UE_Program:	return LOCTEXT("UE_Program_Text", "UE Program");
		case Type_UE_Material:	return LOCTEXT("UE_Material_Text", "UE Material");
		case Type_UE_BakeLight:	return LOCTEXT("UE_BackeLighst_Text", "UE Lightmass");
		case Type_UE_Animation:	return LOCTEXT("UE_Animation_Text", "UE Animation");
	}
	return FText::GetEmpty();
}

static FString BytesToString(const TArray<uint8>& InBytes)
{
	FUTF8ToTCHAR Convert(reinterpret_cast<const char*>(InBytes.GetData()), InBytes.Num());
	FString Temp(Convert.Get());
	Temp = Temp.Left(InBytes.Num());
	return Temp;
}

static TArray<uint8> StringToBytes(const FString& InString)
{
	FTCHARToUTF8 Convert(*InString);
	const TArray<uint8> Output(reinterpret_cast<const uint8*>(Convert.Get()), Convert.Length());
	return Output;
}

static bool StringContainUpperCase(const FString& InString)
{
	for(const auto& Char : InString)
	{
		if(FChar::IsUpper(Char))
		{
			return true;
		}
	}
	return false;
}

static bool StringContainSpecialCase(const FString& InString)
{
	static const TSet<TCHAR> SpecialChars =
	{
		TCHAR('`'), TCHAR('~'),
		TCHAR('!'), TCHAR('@'), TCHAR('#'), TCHAR('$'), TCHAR('%'), TCHAR('^'), TCHAR('&'), TCHAR('*'), TCHAR('('), TCHAR(')'), TCHAR('-'), TCHAR('+'),
		// TCHAR('！'), TCHAR('￥'), /*TCHAR('…'), TCHAR('（'), TCHAR('）'), TCHAR('—'),*/
		TCHAR('['), TCHAR(']'), TCHAR('\\'), TCHAR(';'), TCHAR('\''), TCHAR(','), TCHAR('.'), TCHAR('/'),
		// TCHAR('【'), TCHAR('】'), TCHAR('、'), TCHAR('；'), TCHAR('‘'), TCHAR('，'), TCHAR('。')
	};
	for(const auto& Char : InString)
	{
		if(FChar::IsWhitespace(Char) || SpecialChars.Contains(Char))
		{
			return true;
		}
	}
	return false;
}

static FString GetEngineBinariesDir()
{
	FString Path = FPaths::ConvertRelativePathToFull(FPlatformProcess::ExecutablePath());
	FPaths::MakeStandardFilename(Path);
	const int32 Pos = Path.Find(TEXT("/"), ESearchCase::IgnoreCase, ESearchDir::FromEnd);
	const FString Dir = (Pos != -1) ? Path.Left(Pos) : TEXT("");
#if PLATFORM_MAC
	return FPaths::ConvertRelativePathToFull(FPaths::Combine(Dir, TEXT("../../..")));
#else
	return Dir;
#endif
}

static FString GetXiaoAppPath(const FString& InAppName, const FString& InMiddlePath = TEXT(""), const bool bStandardPath = false)
{
	FString AppPath;
#if PLATFORM_WINDOWS
	AppPath = FString::Printf(TEXT("%s\\%s%s.exe"), *FPlatformProcess::GetCurrentWorkingDirectory(), *InMiddlePath, *InAppName);
#elif PLATFORM_MAC
	if(InAppName == TEXT("XiaoApp"))
	{
		AppPath = FString::Printf(TEXT("%s/%s%s%s"), *GetEngineBinariesDir(), *InMiddlePath, *InAppName, TEXT(".app/Contents/MacOS/XiaoApp"));
	}
	else
	{
		AppPath = FString::Printf(TEXT("/Applications/XiaoApp.app/Contents/UE/Engine/Binaries/Mac/%s"), *InAppName);
	}
#else
	AppPath = FString::Printf(TEXT("%s/%s%s"), *FPlatformProcess::GetCurrentWorkingDirectory(), *InMiddlePath, *InAppName);
#endif
	if (bStandardPath)
	{
		FPaths::MakeStandardFilename(AppPath);
	}
	else
	{
		FPaths::MakePlatformFilename(AppPath);
	}

	if (!FPaths::FileExists(AppPath) && !FPaths::DirectoryExists(AppPath))
	{
		XIAO_LOG(Error, TEXT("App file: \"%s\" not exist"), *AppPath);
	}
	return AppPath;
}

static void RunAs(const FString& InAppPath, const FString& InWorkingPath, const FString& InParam = TEXT(""), const bool bShowWindow = false)
{
#if PLATFORM_WINDOWS
	ShellExecute(nullptr, TEXT("runas"), *InAppPath, *InParam, *InWorkingPath, bShowWindow ? SW_SHOWNORMAL : SW_HIDE);
#elif PLATFORM_MAC
	// 转义成 AppleScript 命令
	const FString AppleScriptCommand = FString::Printf(TEXT("do shell script \\\"%s %s\\\" with administrator privileges"), *InAppPath, *InParam);
	// 再包成 osascript -e "..." 注意最终这一层需要再加双引号
	const FString FinalCmd = FString::Printf(TEXT("-e \"%s\""), *AppleScriptCommand);
	// 调用 osascript
	FPlatformProcess::CreateProc(TEXT("/usr/bin/osascript"), *FinalCmd, true, false, false, nullptr, 0, nullptr, nullptr);	
#elif PLATFORM_UNIX
	const FString Command = FString::Printf(TEXT("%s %s"), *InAppPath, *InParam);
	const FString CmdLine = FString::Printf(TEXT("pkexec bash -c \"%s\""), *Command.ReplaceCharWithEscapedChar());
	FPlatformProcess::CreateProc(TEXT("/bin/bash"), *FString::Printf(TEXT("-c \"%s\""), *CmdLine), true, false, false, nullptr, 0, nullptr, nullptr);
#else
	check(0);
#endif
}

static void RunAsAdmin(const FString& InAppName, const FString& InParam = TEXT(""), const bool bShowWindow = false)
{
	const FString XiaoAppPath = GetXiaoAppPath(InAppName, TEXT(""), false);
	const FString WorkingPath = FPaths::GetPath(XiaoAppPath);
	RunAs(XiaoAppPath, WorkingPath, InParam, bShowWindow);
}

static uint32 GetParentId()
{
	FPlatformProcess::FProcEnumerator ProcIter;
	const uint32 CurrentProcessId = FPlatformProcess::GetCurrentProcessId();
	while (ProcIter.MoveNext())
	{
		FPlatformProcess::FProcEnumInfo ProcInfo = ProcIter.GetCurrent();
		const uint32 Pid = ProcInfo.GetPID();
		if (Pid == CurrentProcessId)
		{
			return ProcInfo.GetParentPID();
		}
	}
	return 0;
}

// TODO 还是要加上路径最安全
static bool IsAppRunning(const FString InAppName, const uint32 InIgonrePID = 0)
{
	FPlatformProcess::FProcEnumerator ProcIter;
	while (ProcIter.MoveNext())
	{
		FPlatformProcess::FProcEnumInfo ProcInfo = ProcIter.GetCurrent();
		const FString ProcName = ProcInfo.GetName();
		if (ProcName.StartsWith(InAppName))
		{
			if (InIgonrePID == 0)
			{
				return true;
			}
			
			return !(ProcInfo.GetPID() == InIgonrePID);
		}
	}
	return false;
}


static std::wstring GetCommandLineByProcessId(const uint32 InProcessId)
{
	std::wstring cmdLine;
	
#if defined(DESKTOPPLATFORM_API) && PLATFORM_WINDOWS
	HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, 0, InProcessId);
	if (hProcess == NULL) 
	{
		XIAO_LOG(Error, TEXT("无法打开进程，错误代码:%d"), GetLastError());
		return L"";
	}

	PROCESS_BASIC_INFORMATION pbi;
	ULONG returnLength;
	if (NtQueryInformationProcess(hProcess, ProcessBasicInformation, &pbi, sizeof(pbi), &returnLength) != 0) 
	{
		XIAO_LOG(Error, TEXT("无法获取进程信息。"));
		CloseHandle(hProcess);
		return L"";
	}

	// 读取 PEB 中的进程参数
	PEB peb;
	if (!ReadProcessMemory(hProcess, pbi.PebBaseAddress, &peb, sizeof(peb), NULL)) 
	{
		XIAO_LOG(Error, TEXT("无法读取 PEB，错误代码: "));;
		CloseHandle(hProcess);
		return L"";
	}

	// 读取进程参数的地址
	RTL_USER_PROCESS_PARAMETERS procParams;
	if (!ReadProcessMemory(hProcess, peb.ProcessParameters, &procParams, sizeof(procParams), NULL)) 
	{
		XIAO_LOG(Error, TEXT("无法读取进程参数，错误代码: "));
		CloseHandle(hProcess);
		return L"";
	}

	// 读取命令行
	wchar_t* commandLineContents = new wchar_t[procParams.CommandLine.Length / sizeof(wchar_t) + 1];
	if (ReadProcessMemory(hProcess, procParams.CommandLine.Buffer, commandLineContents, procParams.CommandLine.Length, NULL)) 
	{
		commandLineContents[procParams.CommandLine.Length / sizeof(wchar_t)] = L'\0';
		cmdLine = commandLineContents;
	}
	else 
	{
		XIAO_LOG(Error, TEXT("无法读取命令行内容，错误代码: "));
	}

	delete[] commandLineContents;
	CloseHandle(hProcess);
#endif
	return cmdLine;
}

static FString GetValueFromString(const FString& InString, const FString& InCmd)
{
	if (!InString.IsEmpty())
	{
		TArray<FString> Parts1;
		InString.ParseIntoArray(Parts1, *InCmd);
		if (Parts1.Num() > 1)
		{
			TArray<FString> Parts2;
			Parts1[1].ParseIntoArray(Parts2, TEXT(" "));
			return Parts2.Num() > 0 ? Parts2[0] : Parts1[1];
		}
	}
	return TEXT("");
}

static uint32 GetProcId(const FString& InProcPath, const FString& InParam = TEXT(""), const uint64 InIgonoreId = 0)
{
	FPlatformProcess::FProcEnumerator ProcIter;
	const FString DesAppName = GetValueFromString(InParam, TEXT("-app="));
	while (ProcIter.MoveNext())
	{
		FPlatformProcess::FProcEnumInfo ProcInfo = ProcIter.GetCurrent();
		const FString ProcPath = ProcInfo.GetFullPath();
		if (FPaths::IsSamePath(ProcPath, InProcPath) && (InIgonoreId == 0 || InIgonoreId != ProcInfo.GetPID()))
		{
			if (InParam.IsEmpty())
			{
				return ProcInfo.GetPID();
			}

			const FString Params = UTF8_TO_TCHAR(GetCommandLineByProcessId(ProcInfo.GetPID()).c_str());
			const FString ThisAppName = GetValueFromString(Params, TEXT("-app="));
			if (DesAppName == ThisAppName)
			{
				XIAO_LOG(Display, TEXT("Find the target proc"));
				return ProcInfo.GetPID();
			}
		}
	}
	XIAO_LOG(Warning, TEXT("Can\'t Find the target proc::%s params::%s"), *InProcPath, *InParam);
	return 0;
}

static FProcHandle GetProcHandle(const FString& InProcPath)
{
	const uint32 ProcessId = GetProcId(InProcPath);
	if (ProcessId > 0)
	{
		return FPlatformProcess::OpenProcess(ProcessId);
	}
	return FProcHandle();
}

static bool ShutdownXiaoApp(const FString& InAppPath)
{
	FProcHandle ProcHandle = GetProcHandle(InAppPath);
	if (ProcHandle.IsValid())
	{
		FPlatformProcess::TerminateProc(ProcHandle, true);
		return true;
	}
	return false;
}

static void GetHelp(const FString& InParam)
{
	FString Error;
	FPlatformProcess::LaunchURL(*FString::Printf(TEXT("%s%s"), GLocalization==TEXT("zh-CN") ? *XiaoUrl::SXiaoBuildChineseMannual : *XiaoUrl::SXiaoBuildManual, *InParam), TEXT(""), &Error);
}

static FString GetBuildVersion()
{
	return TEXT("Version 1.1.0(build 1)");
}

#ifdef WITH_UNREALPNG
static TSharedPtr<FSlateDynamicImageBrush> LoadFromBuffer(const TArray<uint8>& InBuffer, const FString& InImageName)
{
	IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(FName("ImageWrapper"));
	EImageFormat ImageFormat = ImageWrapperModule.DetectImageFormat(InBuffer.GetData(), InBuffer.Num());

	if (ImageFormat == EImageFormat::Invalid)
	{
		UE_LOG(LogTemp, Error, TEXT("Image Parse: invalid image format."));
		return nullptr;
	}

	TSharedPtr<IImageWrapper> ImageWrapper = ImageWrapperModule.CreateImageWrapper(ImageFormat);
	if (!ImageWrapper.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("Image Parse: Unable to make image wrapper for image format %d"), (int32)ImageFormat);
		return nullptr;
	}

	if (!ImageWrapper->SetCompressed(InBuffer.GetData(), InBuffer.Num()))
	{
		UE_LOG(LogTemp, Error, TEXT("Image Parse: Unable to parse image format %d"), (int32)ImageFormat);
		return nullptr;
	}

	TArray<uint8> RawImageData;
	ERGBFormat RGBFormat = ERGBFormat::RGBA;
	if (!ImageWrapper->GetRaw(RGBFormat, 8, RawImageData))
	{
		UE_LOG(LogTemp, Error, TEXT("Image Parse: Unable to convert image format %d to BGRA 8"), (int32)ImageFormat);
		return nullptr;
	}

	const FName ResourceName(InImageName);
	return FSlateDynamicImageBrush::CreateWithImageData(ResourceName, FVector2D((float)ImageWrapper->GetWidth(), (float)ImageWrapper->GetHeight()), RawImageData);
}
#endif

// 机器许可
enum ELicenseType
{
	Type_OnlyInitiator = 1,
	Type_OnlyHelper = 1 << 1,
	Type_InitiatorAndHelper = 1 << 2,
	Type_NoLicense = 1 << 3,
	Type_All = Type_OnlyInitiator | Type_OnlyHelper | Type_InitiatorAndHelper | Type_NoLicense
};

// Helper Enabled
enum EHelperEnabled
{
	Heler_Enabled = 1,
	Heler_Disabled = 1 < 1,
};

// Agent Helper Permissions
enum EHelperPermission
{
	
};

static bool ConstructRESKey(const FString &InKeyString, FAES::FAESKey &OutKey)
{
	TArray<uint8> Key;
	FBase64::Decode(InKeyString, Key);
	check(Key.Num() == sizeof(FAES::FAESKey::Key));
	FNamedAESKey NewKey;
	NewKey.Name = TEXT("Default");
	NewKey.Guid = FGuid();
	FMemory::Memcpy(NewKey.Key.Key, &Key[0], sizeof(FAES::FAESKey::Key));
	OutKey = NewKey.Key;
	return NewKey.IsValid();
}

static const uint32 G_UIntSize = sizeof(G_UIntSize);

static bool EncryptString(const FString& InContent, const FString& InEncryptionKeyString, FString& OutContent)
{
    if (InContent.Len() > 0)
    {
        if (FAES::FAESKey Key; ConstructRESKey(InEncryptionKeyString, Key))
        {
            const int32 ContentSize = InContent.Len();
            const int32 PaddedSize = Align(ContentSize, FAES::AESBlockSize);
            const int32 AddedSize = PaddedSize - ContentSize;
            const int32 AddedLengthSize = AddedSize + Align(G_UIntSize, FAES::AESBlockSize);
            const int FinalSize = ContentSize + AddedLengthSize;
            TArray<uint8> Bytes;
            Bytes.AddUninitialized(FinalSize);
            StringToBytes(InContent, Bytes.GetData(), FinalSize);
            memcpy(Bytes.GetData() + FinalSize - G_UIntSize, &AddedLengthSize, G_UIntSize);
            FAES::EncryptData(Bytes.GetData(), FinalSize, Key);
            OutContent = BytesToString(Bytes.GetData(), FinalSize);
            return true;
        }
    }
    return false;
}

static bool DecryptString(const FString &InContent, const FString &InDecryptionKeyString, FString &OutContent)
{
	const auto ContentLen = InContent.Len();
    if (ContentLen > 0 && ContentLen % 16 == 0)
    {
        if (FAES::FAESKey Key; ConstructRESKey(InDecryptionKeyString, Key))
        {
            const int32 ContentSize = ContentLen;
            TArray<uint8> Bytes;
            Bytes.AddUninitialized(ContentSize);
            StringToBytes(InContent, Bytes.GetData(), ContentSize);
            FAES::DecryptData(Bytes.GetData(), ContentSize, Key);
            OutContent = BytesToString(Bytes.GetData(), ContentSize);
			int32 AddedSize = 0;
            memcpy(&AddedSize, Bytes.GetData() + ContentSize - G_UIntSize, G_UIntSize);
            OutContent = OutContent.LeftChop(AddedSize);
            return true;
        }
    }
    return false;
}

static bool Json2String(const TSharedPtr<FJsonObject> InObject, FString& OutString, const FString& InEncryptKey=TEXT(""))
{
	const TSharedRef<TJsonWriter<TCHAR>> JsonWriter = TJsonWriterFactory<TCHAR>::Create(&OutString);
	if (!FJsonSerializer::Serialize(InObject.ToSharedRef(), JsonWriter))
	{
		const uint32 ErrorCode = FPlatformMisc::GetLastError();
		XIAO_LOG(Error, TEXT("JsonSerializer::Serialize::Failed.ErrorCode::%d!"), ErrorCode);
		return false;
	}

	if(InEncryptKey.Len() > 0)
	{
		FString TempString = OutString;
		if(!EncryptString(TempString, InEncryptKey, OutString))
		{
			XIAO_LOG(Error, TEXT("EncryptString Failed!"));
			return false;
		}
	}
	return true;
}

static bool String2Json(const FString& InString, TSharedPtr<FJsonObject>& OutObject, const FString& InDecryptKey=TEXT(""))
{
	FString TempString = InString;
	if(InDecryptKey.Len() > 0)
	{
		if(!DecryptString(TempString, InDecryptKey, TempString))
		{
			XIAO_LOG(Error, TEXT("DecryptString Failed!"));
			return false;
		}
	}
	const TSharedRef<TJsonReader<TCHAR>> Reader = FJsonStringReader::Create(TempString);
		
	if (!FJsonSerializer::Deserialize(Reader, OutObject) || !OutObject.IsValid())
	{
		XIAO_LOG(Error, TEXT("Deserialize the \"%s\" failed!"), *TempString);
		return false;
	}
	return true;
}

// TODO 还不能保证唯一，需要进行加密计算
static FString GetUniqueDeviceID()
{
#if PLATFORM_DESKTOP
	PRAGMA_DISABLE_DEPRECATION_WARNINGS
#endif
	static const FString MacAddr = FPlatformMisc::GetMacAddressString();
#if PLATFORM_DESKTOP
	PRAGMA_ENABLE_DEPRECATION_WARNINGS
#endif
	
	return MacAddr;
}

#ifdef DESKTOPPLATFORM_API
static bool OpenFileDialog(const FString& InTitle, const FString& InDefaultPath, const FString& InFileTypes, TArray<FString>& OutFiles)
{
	if (IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get())
	{
		if (DesktopPlatform->OpenFileDialog(
			FSlateApplication::Get().FindBestParentWindowHandleForDialogs(nullptr),
			InTitle,
			InDefaultPath,
			TEXT(""),
			InFileTypes,
			EFileDialogFlags::None,
			OutFiles
		))
		{
			return true;
		}
	}
	return false;
}

static bool OpenFolderDialog(const FString& InTitle, const FString& InDefaultPath, FString& OutFolder)
{
	if (IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get())
	{
		if (DesktopPlatform->OpenDirectoryDialog(
			FSlateApplication::Get().FindBestParentWindowHandleForDialogs(nullptr),
			InTitle,
			InDefaultPath,
			OutFolder
		))
		{
			return true;
		}
	}
	return false;
}
#endif

#if PLATFORM_WINDOWS
inline HANDLE GAppMutex = nullptr;
#else
#include <fcntl.h>
#include <sys/file.h>
#include <unistd.h>
inline int GAppLockId = -1;
#endif

static bool CheckSingleton(const FString& InAppName = TEXT(""))
{
	FString AppName = InAppName.IsEmpty() ? FPlatformProcess::ExecutableName() : InAppName;
#if PLATFORM_WINDOWS
	GAppMutex = ::CreateMutexW(nullptr, false, *AppName);
	if (GAppMutex == nullptr || ::GetLastError() == ERROR_ALREADY_EXISTS)
	{
		::CloseHandle(GAppMutex);
		GAppMutex = nullptr;
		return false;
	}
#else
	AppName = TEXT("/tmp/") + AppName + TEXT(".lock");
	GAppLockId = open(TCHAR_TO_UTF8(*AppName), O_CREAT | O_RDWR, 0666);
	if (GAppLockId == -1)
	{
		XIAO_LOG(Error, TEXT("Can'\t create lock file!"));
		return false;
	}

	if (flock(GAppLockId, LOCK_EX | LOCK_NB) == -1)
	{
		close(GAppLockId);
		return false;
	}
#endif

	return true;
}

#if PLATFORM_WINDOWS
#ifdef DESKTOPPLATFORM_API
static HWND FindTargetWindow(const DWORD InProcessId)
{
	static HWND HWnd = nullptr;
	HWnd = nullptr;
	EnumWindows(
		[](HWND hWnd, LPARAM lParam) -> BOOL
		{
			DWORD ProcessId = 0;
			GetWindowThreadProcessId(hWnd, &ProcessId);
			if (ProcessId == static_cast<DWORD>(lParam))
			{
				HWnd = hWnd;
				SetLastError(1);
				return 0;
			}
			return 1;
		},
		(LPARAM)InProcessId
	);
	SetLastError(0);
	return HWnd;
};
#endif
#endif

static void BringAppToTop(const uint32& InProcId)
{
#if PLATFORM_WINDOWS
#ifdef DESKTOPPLATFORM_API
	auto hWnd = FindTargetWindow(InProcId);
	if (hWnd == nullptr)
	{
		XIAO_LOG(Error, TEXT("Can\' find the window!"));
		return;
	}

	/*if (!::IsWindow(hWnd)) return;

	SetWindowPos(hWnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

	ShowWindow(hWnd, SW_SHOWNORMAL);
	SendMessageW(hWnd, WM_ACTIVATE, WA_ACTIVE, 0);

	SetForegroundWindow(hWnd);*/
	/*if (!::BringWindowToTop(hWnd))
	{
		XIAO_LOG(Error, TEXT("BringWindowToTop failed LastError: %d"), GetLastError());
	}*/
#endif
#endif
}

static void ReleaseAppMutex()
{
#if PLATFORM_WINDOWS
	if (GAppMutex)
	{
		::CloseHandle(GAppMutex);
	}
#else
	if (GAppLockId > 0)
	{
		flock(GAppLockId, LOCK_UN);
		close(GAppLockId);
	}
#endif
}

inline bool GbQueryLicense = true;
inline FString GOutputFile;
inline FString GLogFile;
inline FString GMonitorFile;
inline int GLogLevel = 3;
inline int GMaxCpus = MAX_int16;
inline int GStopBuildId = -1;
inline FString GAppendText{TEXT(""});
inline EWinVersion GMinWinVer = EWinVersion::Win_XP;
inline EWinVersion GMaxWinVer = EWinVersion::Win_11;
inline TArray<TSharedPtr<FString>> GGroupArray;
inline TArray<TSharedPtr<FText>> GPriorityArray;
inline TArray<TSharedPtr<FText>> GLevelArray;
inline TArray<TSharedPtr<FText>> GRoleArray;
inline TArray<TSharedPtr<FText>> GMasterSlaveArray;
inline TArray<TSharedPtr<FText>> GStatusArray;

#undef LOCTEXT_NAMESPACE