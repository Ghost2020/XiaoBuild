/**
  * @author cxx2020@outlook.com
  * @date 10:24 PM
 */
#pragma once

#include "Templates/UniquePtr.h"
#include "Serialization/JsonSerializerMacros.h"
#include "XmlFile.h"
#include "HAL/PlatformFileManager.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Misc/FileHelper.h"
#include "Dom/JsonObject.h"

#include "XiaoShareNetwork.h"
#include "XiaoShare.h"
#include "XiaoShareRedis.h"
#include "XiaoCompressor.h"
#include "XiaoAgent.h"


#if PLATFORM_WINDOWS
	#include "Windows/AllowWindowsPlatformTypes.h"
	#include <ShlObj.h>
	#include "Windows/HideWindowsPlatformTypes.h"
#endif

static const FString SPlatformName =
#if PLATFORM_WINDOWS
TEXT("Win64");
#elif PLATFORM_LINUXARM64
TEXT("LinuxArm64");
#elif PLATFORM_LINUX
TEXT("Linux");
#elif PLATFORM_MAC
TEXT("Mac");
#else
#error "Unsupported Platform"
#endif

static const FString SExeExtension =
#if PLATFORM_WINDOWS
TEXT(".exe")
#elif PLATFORM_MAC
TEXT("")
#elif PLATFORM_UNIX
TEXT("")
#endif			
;

static const FString SDllExtension =
#if PLATFORM_WINDOWS
TEXT(".dll")
#elif PLATFORM_MAC
TEXT(".dylib")
#elif PLATFORM_UNIX
TEXT(".so")
#endif			
;

enum EInstallType : uint8
{
	IT_Install = 0,
	IT_Update,
	// IT_Repair,
	// IT_Backup,
	IT_Unistall,
};

enum EComponentTye : uint8
{
	CT_None = 0,
	CT_Agent = 1,
	CT_Coordinator = 2,
	CT_AgentAndCoordi = CT_Agent | CT_Coordinator,
	CT_Visulizer = 4,
	CT_AgentVisualer = CT_Agent | CT_Visulizer,
	CT_AgentCoordiVisulizer = CT_Agent | CT_Coordinator | CT_Visulizer,
	CT_Cache = 8,
	CT_BackCoordi = 16
};

enum EAgentType : uint8
{
	AT_None = 0,
	AT_Helper,
	AT_Initiator,
	AT_Both
};

enum EInitiatorType : uint8
{
	InitiatorType_None = 0,
	InitiatorType_Fixed,
	InitiatorType_Floating,
	InitiatorType_CiFixed,
	InitiatorType_CiFloating
};

enum EHelperType : uint8
{
	ET_None = 0,
	ET_Fixed,
	ET_Floating
};


#include "XiaoInterprocess.h"
using namespace boost::interprocess;

#define JSON_MCI_VALUE(var) JSON_SERIALIZE(#var, var)

inline TSharedPtr<shared_memory_object> GProgressShm = nullptr;
inline TSharedPtr<mapped_region> GProgressRegion = nullptr;

bool static TryCreateIPC()
{
	XIAO_LOG(Log, TEXT("TryCreateIPC Begin"));
	try
	{
		GProgressShm = MakeShared<shared_memory_object>(open_or_create, XiaoIPC::SInstallProgressMemoryName.c_str(), read_write);
		if (!GProgressShm.IsValid())
		{
			XIAO_LOG(Error, TEXT("Can't create shared memory object!"));
			return false;
		}
		GProgressShm->truncate(XiaoIPC::SInstallProgressMemorySize);
		GProgressRegion = MakeShared<mapped_region>(*GProgressShm.Get(), read_write);
		if (!GProgressRegion.IsValid())
		{
			XIAO_LOG(Error, TEXT("Cant Create Progress Map Region!"));
			return false;
		}

		XIAO_LOG(Log, TEXT("TryCreateIPC Finish"));
		return true;
	}
	catch (interprocess_exception& Ex)
	{
		XIAO_LOG(Error, TEXT("Interprocee Object Create Exception::%s!"), UTF8_TO_TCHAR(Ex.what()));
		return false;
	}
}

#if PLATFORM_WINDOWS
static FString GetWindowsKnownDir(REFKNOWNFOLDERID InFolderType)
{
	FString KnownDir;

	TCHAR* UserPath;
	HRESULT Ret = SHGetKnownFolderPath(InFolderType, 0, NULL, &UserPath);
	if (SUCCEEDED(Ret))
	{
		// make the base user dir path
		KnownDir = FString(UserPath).Replace(TEXT("\\"), TEXT("/")) + TEXT("/");
		CoTaskMemFree(UserPath);
	}

	return KnownDir;
}
#endif

struct FInstallProgress : FJsonSerializable
{
	float Progress = 0.0f;
	int Status = 0;
	FString Message;

	BEGIN_JSON_SERIALIZER
		JSON_MCI_VALUE(Progress);
		JSON_MCI_VALUE(Status);
		JSON_MCI_VALUE(Message);
	END_JSON_SERIALIZER
}; inline FInstallProgress GProgress;

struct FInstallSettings : FJsonSerializable
{
	FString Localization = TEXT("zh-CN");

	uint32 SetupUpType = EInstallType::IT_Install;

	uint32 InstallType = EComponentTye::CT_AgentVisualer;

	bool bEnableAutoTray = true;

	//~ Begin Coordinator
	bool bSilentInstall = false;

	bool bHasLicense = false;
	FString LicenseKey;

	uint32 CoordiListenPort = XiaoNetwork::SCoordiServicePort;
	uint32 UIListenPort = XiaoNetwork::SUIServicePort;
	uint32 PerfTransport = XiaoNetwork::SIPerfServicePort;
	uint32 AgentListenPort = XiaoNetwork::SAgentServicePort;
	uint32 SchedulerServerPort = XiaoNetwork::SSchedulerServerPort;
	uint32 LicenseListenPort = XiaoNetwork::SLicenseServicePort;
	uint32 CacheListenPort = XiaoNetwork::SCacheServicePort;
	bool bAutoOpenFirewall = true;

	FString InstallFolder;
	FString CacheFolder =
#if PLATFORM_WINDOWS
		FPaths::ConvertRelativePathToFull(FPaths::Combine( GetWindowsKnownDir(FOLDERID_ProgramData), TEXT("XiaoBuild/Cache") ))
#else
		TEXT("")
#endif
		;
	bool bAddEnvironment = true;

	bool bHasSSL = true;
	FString CertFile;
	FString KeyFile;

	FString Username = TEXT("admin");
	FString Domain = TEXT(".");
	FString Password;
	//~ End Coordinator


	//~ Begin AgentSettings
	FString CoordiIp = TEXT("localhost");
	uint32 CoordiPort = XiaoNetwork::SCoordiServicePort;
	uint32 HelpListenPort = XiaoNetwork::SHelpListenPort;

	uint32 FileCache = 4096;
	bool bInstallIDE = true;

	TArray<FString> EngineFolders;
	TArray<int> EngineTypes;
	TArray<FString> EngineVersions;
	//~ End AgentSettings

	BEGIN_JSON_SERIALIZER
		JSON_MCI_VALUE(Localization);
		JSON_MCI_VALUE(SetupUpType);
		JSON_MCI_VALUE(InstallType);
		JSON_MCI_VALUE(bEnableAutoTray);
		JSON_MCI_VALUE(bSilentInstall);
		JSON_MCI_VALUE(bHasLicense);
		JSON_MCI_VALUE(LicenseKey);
		JSON_MCI_VALUE(CoordiListenPort);
		JSON_MCI_VALUE(UIListenPort);
		JSON_MCI_VALUE(PerfTransport);
		JSON_MCI_VALUE(AgentListenPort);
		JSON_MCI_VALUE(SchedulerServerPort);
		JSON_MCI_VALUE(LicenseListenPort);
		JSON_MCI_VALUE(CacheListenPort);
		JSON_MCI_VALUE(bAutoOpenFirewall);
		JSON_MCI_VALUE(InstallFolder);
		JSON_MCI_VALUE(CacheFolder);
		JSON_MCI_VALUE(bAddEnvironment);
		JSON_MCI_VALUE(bHasSSL);
		JSON_MCI_VALUE(CertFile);
		JSON_MCI_VALUE(KeyFile);
		JSON_MCI_VALUE(Username);
		JSON_MCI_VALUE(Domain);
		JSON_MCI_VALUE(Password);

		JSON_MCI_VALUE(CoordiIp);
		JSON_MCI_VALUE(CoordiPort);
		JSON_MCI_VALUE(HelpListenPort);
		JSON_MCI_VALUE(FileCache);
		JSON_MCI_VALUE(bInstallIDE);
		JSON_SERIALIZE_ARRAY("EngineFolders", EngineFolders);
		JSON_SERIALIZE_ARRAY("EngineTypes", EngineTypes);
		JSON_SERIALIZE_ARRAY("EngineVersions", EngineVersions);
	END_JSON_SERIALIZER
}; inline FInstallSettings GInstallSettings;


struct FInstallFolder
{
	FString Folder;
	bool Type = false;
	bool bInstall = true;
	bool bPluginInstall = true;
	FString EngineVersion;

	explicit FInstallFolder(const FString& InFolder, const bool InType, const bool bInInstall, const bool bInPluginInstall=true)
		: Folder(InFolder)
		, Type(InType)
		, bInstall(bInInstall)
		, bPluginInstall(bInPluginInstall)
	{}

	FInstallFolder& operator=(const FInstallFolder& InAnother)
	{
		this->Folder = InAnother.Folder;
		this->Type = InAnother.Type;
		this->bInstall = InAnother.bInstall;
		this->bPluginInstall = InAnother.bPluginInstall;
		return *this;
	}
};

static void UpdateProgress(const float InSleepTime = 0.5f)
{
	try
	{
		if(GProgressRegion.IsValid())
		{
			FPlatformProcess::Sleep(InSleepTime);
			const std::string ProgressString = TCHAR_TO_UTF8(*GProgress.ToJson());
			strcpy_s(static_cast<char*>(GProgressRegion->get_address()), ProgressString.size()+1, ProgressString.c_str());
		}
	}
	catch (interprocess_exception& Ex)
	{
		XIAO_LOG(Error, TEXT("Interprocee Exception::%s!"), UTF8_TO_TCHAR(Ex.what()));
	}
}

static void UpdateMessage(const float InProgress, const FString InMessage=TEXT(""))
{
	GProgress.Progress = InProgress;
	if(!InMessage.IsEmpty())
	{
		GProgress.Message = InMessage;
	}
	UpdateProgress();
	XIAO_LOG(Log, TEXT("Progress::%s"), *InMessage);
}


static bool IsUnrealEditorRuning()
{
	if (IsAppRunning(TEXT("UnrealEditor")))
	{
		return true;
	}
	if (IsAppRunning(TEXT("UE4Editor")))
	{
		return true;
	}

	return false;
}

static bool CheckEnvRuning(FString& OutError, const uint32 InIgnore = 0)
{
	if (GInstallSettings.InstallType & CT_Agent)
	{
		if (IsAppRunning(XiaoAppName::SBuildAgentService))
		{
			OutError = TEXT("XiaoAgentService正在运行，安装前请停止运行！");
			return false;
		}
	
		if (IsAppRunning(XiaoAppName::SUbaAgent))
		{
			OutError = TEXT("UbaAgent正在运行，安装前请停止运行！");
			return false;
		}
		if (IsAppRunning(XiaoAppName::SXiaoScheduler))
		{
			OutError = TEXT("UbaScheduler 正在运行，安装前请停止运行！");
			return false;
		}
	}
	
	if ((GInstallSettings.InstallType & CT_Coordinator) && IsAppRunning(XiaoAppName::SBuildCoordiService))
	{
		OutError = TEXT("XiaoCoordiService 正在运行，安装前请停止运行！");
		return false;
	}
	
	if ((GInstallSettings.InstallType & CT_Cache) && IsAppRunning(XiaoAppName::SCacheServer))
	{
		OutError = TEXT("cache-server 正在运行，安装前请停止运行！");
		return false;
	}

	if (GInstallSettings.InstallType & CT_AgentCoordiVisulizer)
	{
		/*if (IsAppRunning(XiaoAppName::SBuildLicenseService))
		{
			OutError = TEXT("XiaoLicenseService 正在运行，安装前请停止运行！");
			return false;
		}*/
		if (IsAppRunning(XiaoAppName::SBuildApp, InIgnore))
		{
			OutError = TEXT("XiaoApp 正在运行，安装前请停止运行！");
			return false;
		}
		if (IsUnrealEditorRuning())
		{
			OutError = TEXT("Unreal 编辑器正在运行，安装前请停止运行！");
			return false;
		}
#if PLATFORM_WINDOWS
		if (IsAppRunning(XiaoAppName::SVisualStudio))
		{
			OutError = TEXT("Microsoft VisualStudio IDE正在运行，安装前请停止运行！");
			return false;
		}
#endif
	}

	if (GInstallSettings.InstallFolder.IsEmpty() || !(GInstallSettings.InstallFolder.Len() > 0 && FPaths::DirectoryExists(GInstallSettings.InstallFolder)))
	{
		GInstallSettings.InstallFolder = FPaths::ConvertRelativePathToFull(FPaths::Combine(FPaths::GetPath(FPlatformProcess::ExecutablePath()), TEXT("../../")));
	}

	if (GInstallSettings.InstallFolder.EndsWith(TEXT("/")))
	{
		GInstallSettings.InstallFolder.RemoveAt(GInstallSettings.InstallFolder.Len()-1);
	}
	if (!GInstallSettings.InstallFolder.EndsWith(TEXT("Engine")))
	{
		OutError = TEXT("安装程序所在的位置不正确::") + GInstallSettings.InstallFolder;
		return false;
	}
	return true;
}

static void UpdateError(const FString InError)
{
	GProgress.Message = InError;
	GProgress.Status = -1;
	XIAO_LOG(Error, TEXT("%s"), *InError);
	UpdateProgress(2.0f);
}

static bool GetCanSyncUpdate(std::string& OutData)
{
	if (!XiaoRedis::SRedisClient->exists(XiaoRedis::String::SSystemSync))
	{
		return false;
	}

	auto Option = XiaoRedis::SRedisClient->get(XiaoRedis::String::SSystemSync);
	if (!Option.has_value())
	{
		return false;
	}

	OutData = Option.value();
	if (OutData.size() == 0)
	{
		return false;
	}

	FString SystemHash;
	std::string OutBuffer;
	GetSystemHash(SystemHash, OutBuffer);
	if (XiaoRedis::SRedisClient->exists(XiaoRedis::Hash::SAgentHash))
	{
		XiaoRedis::SRedisClient->hset(XiaoRedis::Hash::SAgentHash, GAgentUID, TCHAR_TO_UTF8(*SystemHash));
	}
	if (OutBuffer.size() == OutData.size())
	{
		const FString Hash = FMD5::HashBytes(reinterpret_cast<const uint8*>(OutData.data()), OutData.size());
		if (Hash == SystemHash)
		{
			return false;
		}
	}

	return true;
}


static const FString SBuildCongfig(TEXT("BuildConfiguration"));
static const FString SBuildConfigXml(SBuildCongfig+TEXT(".xml"));

static void EditConfigXml(const FString& InXmlPath, const bool bInEnableUBAC)
{
	if (FPaths::FileExists(InXmlPath))
	{
		FXmlFile* File = new FXmlFile(InXmlPath);
		if (File->IsValid())
		{
			if (FXmlNode* RootNode = File->GetRootNode())
			{
				FXmlNode* ConfigNode = RootNode->FindChildNode(SBuildCongfig);
				if (!ConfigNode)
				{
					RootNode->AppendChildNode(SBuildCongfig);
					ConfigNode = RootNode->FindChildNode(SBuildCongfig);
				}

				const FString UBACContent = bInEnableUBAC ? TEXT("true") : TEXT("false");
				const FString NotUBACContent = bInEnableUBAC ? TEXT("false") : TEXT("true");
				FXmlNode* UBACNode = ConfigNode->FindChildNode(TEXT("bAllowUBACExecutor"));
				if (!UBACNode)
				{
					ConfigNode->AppendChildNode(TEXT("bAllowUBACExecutor"), UBACContent);
				}
				else
				{
					UBACNode->SetContent(UBACContent);
				}

				if (FXmlNode* UBANode = ConfigNode->FindChildNode(TEXT("bAllowUBAExecutor")))
				{
					UBANode->SetContent(NotUBACContent);
				}

				FXmlNode* XGENode = ConfigNode->FindChildNode(TEXT("bAllowXGE"));
				if (!XGENode)
				{
					ConfigNode->AppendChildNode(TEXT("bAllowXGE"), NotUBACContent);
				}
				else
				{
					XGENode->SetContent(NotUBACContent);
				}
			}
		}

		if (File)
		{
			File->Save(InXmlPath);
			delete File;
		}
	}
}

static FString GetEnghineVersion(const FString& InEngineVersion)
{
	TArray<FString> Sections;
	InEngineVersion.ParseIntoArray(Sections, TEXT("."));
	if (Sections.Num() > 2)
	{
		return FString::Printf(TEXT("%s.%s"), *Sections[0], *Sections[1]);
	}
	return InEngineVersion;
}

static bool IsSupportUBAC(const FString& InVersionStr)
{
	static const TSet<FString> InternalSupportEngineSet = { 
		TEXT("4.26."), TEXT("4.27."), 
		TEXT("5.0."), TEXT("5.1."), TEXT("5.2."), TEXT("5.3."), TEXT("5.4."), TEXT("5.5."), TEXT("5.6.") 
	};
	auto IsInSet = [](const TSet<FString>& InSet, const FString& InString) ->bool
	{
		for (const FString& InStr : InSet)
		{
			if (InString.Contains(InStr))
			{
				return true;
			}
		}

		return false;
	};

	bool Support = IsInSet(InternalSupportEngineSet, InVersionStr);
	if (!Support)
	{
		// 检查目录下是否有对应的版本
		const FString XIAO_HOME = FPlatformMisc::GetEnvironmentVariable(TEXT("XIAO_HOME"));
		const FString UBACFolder = FPaths::Combine(XIAO_HOME, TEXT("Binaries/DotNET/UnrealBuildTool/UBAC"));
		if (FPaths::DirectoryExists(UBACFolder))
		{
			auto& Platform = FPlatformFileManager::Get().GetPlatformFile();
			static TSet<FString> SUbacFolders;
			if (SUbacFolders.IsEmpty())
			{
				Platform.IterateDirectory(*UBACFolder, [](const TCHAR* FileOrFolderPath, bool bIsDirectory) ->bool
				{
					if (bIsDirectory)
					{
						const FString Folder = FileOrFolderPath;
						if (Folder.StartsWith(TEXT("4.")) || Folder.StartsWith(TEXT("5.")))
						{
							SUbacFolders.Add(Folder);
							return true;
						}
					}
					return false;
				});
			}
			
			Support = IsInSet(SUbacFolders, InVersionStr);
		}
	}
	return Support;
}

static void GetAllEngineFolder(TArray<TSharedPtr<FInstallFolder>>& OutFolderArray)
{
	TMap<FString, FString> EngineInstallations;
#ifdef DESKTOPPLATFORM_API
	auto DeskPlatformModule = FDesktopPlatformModule::Get();
	DeskPlatformModule->EnumerateEngineInstallations(EngineInstallations);
	GInstallSettings.EngineFolders.Reset();
	GInstallSettings.EngineTypes.Reset();
	GInstallSettings.EngineVersions.Reset();
	for (const auto& Iter : EngineInstallations)
	{
		FEngineVersion EngineVersion;
		DeskPlatformModule->TryGetEngineVersion(Iter.Value, EngineVersion);
		const FString VersionString = EngineVersion.ToString();
		if (IsSupportUBAC(VersionString))
		{
			const bool bSourceEngine = DeskPlatformModule->IsSourceDistribution(Iter.Value);
			OutFolderArray.Add(MakeShareable(new FInstallFolder(Iter.Value, bSourceEngine, true)));
			OutFolderArray.Last()->EngineVersion = VersionString;
			GInstallSettings.EngineFolders.Add(Iter.Value);
			GInstallSettings.EngineTypes.Add(bSourceEngine ? 1 : 0);
			GInstallSettings.EngineVersions.Add(VersionString);
		}
	}
#endif
}

static void GetEngineStates(TArray<TSharedPtr<FInstallFolder>>& OutFolderArray)
{
	for (auto& Desc : OutFolderArray)
	{
		const FString UBTDll = FPaths::ConvertRelativePathToFull(FPaths::Combine(Desc->Folder, FString::Printf(TEXT("Engine/Binaries/DotNET/%sUnrealBuildTool.backup"), !Desc->EngineVersion.StartsWith(TEXT("4")) ? TEXT("UnrealBuildTool/") : TEXT(""))));
		Desc->bInstall = FPaths::FileExists(UBTDll);
		const FString UBACController = FPaths::ConvertRelativePathToFull(FPaths::Combine(Desc->Folder, TEXT("Engine/Plugins/Marketplace/UbaCompatibleController")));
		Desc->bPluginInstall = FPaths::DirectoryExists(UBACController);
	}
}

static void _CopyFile(const bool bInstall, const FString& InSrcFile, const FString& InDesFile, const FString& InBackFileExtension)
{
	const FString BackupFile = InDesFile.Replace(*InBackFileExtension, TEXT(".backup"));
	if (bInstall)
	{
		if (FPaths::FileExists(InDesFile))
		{
			IFileManager::Get().Move(*BackupFile, *InDesFile, true, true, false, true);
		}
	}
	else if (FPaths::FileExists(BackupFile))
	{
		IFileManager::Get().Move(*InDesFile, *BackupFile, true, true, false, true);
	}

	const FString TargetFolder = FPaths::GetPath(InDesFile);
	if (!FPaths::DirectoryExists(TargetFolder))
	{
		XIAO_LOG(Warning, TEXT("Copy SrcFile::%s Target Folder::%s is not exist"), *InSrcFile, *TargetFolder);
		return;
	}
	auto& Platform = FPlatformFileManager::Get().GetPlatformFile();
	if (bInstall && !Platform.CopyFile(*InDesFile, *InSrcFile, EPlatformFileRead::AllowWrite, EPlatformFileWrite::AllowRead))
	{
		XIAO_LOG(Error, TEXT("Copy SrcFile::%s -> DesFile::%s with LastError::%d"), *InSrcFile , *InDesFile, FPlatformMisc::GetLastError());
	}
}

static void _ModifyBuildID(const FString InXiaoModulesFile, const FString& InXGEModulesFile)
{
	XIAO_LOG(Log, TEXT("ModifyBuildID::%s Begin."), *InXiaoModulesFile);
	if (FPaths::FileExists(InXiaoModulesFile) && FPaths::FileExists(InXGEModulesFile))
	{
		// 获取XGE中的模块ID
		FString JsonStr;
		if (!FFileHelper::LoadFileToString(JsonStr, *InXGEModulesFile))
		{
			XIAO_LOG(Error, TEXT("XGE LoadFileToString Failed: %s"), *InXGEModulesFile);
			return;
		}

		TSharedRef<TJsonReader<>> XGEModuesReader = TJsonReaderFactory<>::Create(JsonStr);
		TSharedPtr<FJsonObject> XGEJsonObject;
		if (!FJsonSerializer::Deserialize(XGEModuesReader, XGEJsonObject) || !XGEJsonObject.IsValid())
		{
			XIAO_LOG(Error, TEXT("XGE JsonSerializer::Deserialize failed %s"), *InXGEModulesFile);
			return;
		}

		static const FString SBuildId(TEXT("BuildId"));

		FString BuildId;
		if (!XGEJsonObject->TryGetStringField(SBuildId, BuildId))
		{
			XIAO_LOG(Error, TEXT("TryGetStringField failed %s"), *InXGEModulesFile);
			return;
		}
		XIAO_LOG(Log, TEXT("BuildID %s"), *BuildId);

		if (!FPaths::FileExists(InXiaoModulesFile))
		{
			XIAO_LOG(Error, TEXT("Modules file not exist %s"), *InXiaoModulesFile);
			return;
		}

		// UbaPlugin
		FString UbaPluginJsonStr;
		if (!FFileHelper::LoadFileToString(UbaPluginJsonStr, *InXiaoModulesFile))
		{
			XIAO_LOG(Error, TEXT("UbaPlugin LoadFileToString Failed: %s"), *InXiaoModulesFile);
			return;
		}

		TSharedRef<TJsonReader<>> UBACModuesReader = TJsonReaderFactory<>::Create(UbaPluginJsonStr);
		TSharedPtr<FJsonObject> UBACJsonObject;
		if (!FJsonSerializer::Deserialize(UBACModuesReader, UBACJsonObject) || !UBACJsonObject.IsValid())
		{
			XIAO_LOG(Error, TEXT("Uba JsonSerializer::Deserialize failed %s"), *InXiaoModulesFile);
			return;
		}

		UBACJsonObject->SetStringField(SBuildId, BuildId);

		FString OutputString;
		TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
		if (!FJsonSerializer::Serialize(UBACJsonObject.ToSharedRef(), Writer))
		{
			XIAO_LOG(Error, TEXT("FJsonSerializer::Serialize failed %s"), *InXiaoModulesFile);
			return;
		}

		if (FFileHelper::SaveStringToFile(OutputString, *InXiaoModulesFile))
		{
			XIAO_LOG(Log, TEXT("Successfully Modify the modulesfile %s BuildID -> %s"), *InXiaoModulesFile, *BuildId);
		}

		XIAO_LOG(Log, TEXT("ModifyBuildID::%s Finish."), *InXiaoModulesFile);
	}
	else
	{
		XIAO_LOG(Warning, TEXT("Target file \"%s\" or Ref file \"\" not exist."), *InXiaoModulesFile, *InXGEModulesFile);
	}
}

static bool _AnnotateFile(const FString& InSourceFile, const TArray<uint32>& InhLines)
{
	if (!FPaths::FileExists(InSourceFile))
	{
		XIAO_LOG(Error, TEXT("AnnotateFile::FileExists::Failed::%s"), *InSourceFile);
		return false;
	}

	TArray<FString> Lines;
	if (!FFileHelper::LoadFileToStringArray(Lines, *InSourceFile))
	{
		XIAO_LOG(Error, TEXT("AnnotateFile::LoadFileToStringArray::Failed::%s"), *InSourceFile);
		return false;
	}

	for (const uint32& Line : InhLines)
	{
		Lines[Line] = TEXT("//") + Lines[Line];
	}

	if (!FFileHelper::SaveStringArrayToFile(Lines, *InSourceFile))
	{
		XIAO_LOG(Error, TEXT("AnnotateFile::SaveStringArrayToFile::Failed::%s"), *InSourceFile);
		return false;
	}

	return true;
}

static void _EditPlugin(const FString& InPluginPath, const bool bInEnable)
{
	XIAO_LOG(Log, TEXT("EditPlugin::%s Begin."), *InPluginPath);

	if (FPaths::FileExists(InPluginPath))
	{
		FString JsonStr;
		if (!FFileHelper::LoadFileToString(JsonStr, *InPluginPath))
		{
			XIAO_LOG(Error, TEXT("Failed to load JSON file: %s"), *InPluginPath);
			return;
		}

		TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(JsonStr);
		TSharedPtr<FJsonObject> JsonObject;
		if (!FJsonSerializer::Deserialize(JsonReader, JsonObject) || !JsonObject.IsValid())
		{
			XIAO_LOG(Error, TEXT("FJsonSerializer::Deserialize failed %s"), *InPluginPath);
			return;
		}

		bool bEnableByDefault;
		if (JsonObject->TryGetBoolField(TEXT("EnabledByDefault"), bEnableByDefault))
		{
			JsonObject->SetBoolField(TEXT("EnabledByDefault"), bInEnable);
		}

		FString OutputString;
		TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
		if (!FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer))
		{
			XIAO_LOG(Error, TEXT("FJsonSerializer::Serialize failed %s"), *InPluginPath);
			return;
		}

		if (FFileHelper::SaveStringToFile(OutputString, *InPluginPath))
		{
			XIAO_LOG(Log, TEXT("Successfully Modify the UPlugin %s."), *InPluginPath);
		}
	}
}

static void EnumerateProjectsKnownByEngine(const FString& InEngineVerison, TSet<FString>& OutProjectFileNames)
{
#ifdef DESKTOPPLATFORM_API
	// Find all the engine installations
	TMap<FString, FString> EngineInstallations;
	auto PlatformModule = FDesktopPlatformModule::Get();
	PlatformModule->EnumerateEngineInstallations(EngineInstallations);

	XIAO_LOG(Log, TEXT("Looking for projects..."));

	// Add projects from every branch that we know about
	for (const auto& Iter : EngineInstallations)
	{
		TArray<FString> ProjectFiles;

		XIAO_LOG(Log, TEXT("Found Engine Installation \"%s\"(%s)"), *Iter.Key, *Iter.Value);

		if (PlatformModule->EnumerateProjectsKnownByEngine(Iter.Key, false, ProjectFiles))
		{
			FEngineVersion EngineVersion;
			if (PlatformModule->TryGetEngineVersion(Iter.Value, EngineVersion))
			{
				if (InEngineVerison == EngineVersion.ToString())
				{
					OutProjectFileNames.Append(MoveTemp(ProjectFiles));
					return;
				}
			}
		}
	}
#endif
}

static bool RemovePluginFromUProject(const FString& InProjectFilePath, const FString& InPluginNameToRemove)
{
	FString FileContent;
	if (!FFileHelper::LoadFileToString(FileContent, *InProjectFilePath))
	{
		XIAO_LOG(Error, TEXT("Failed to load project file: %s"), *InProjectFilePath);
		return false;
	}

	TSharedPtr<FJsonObject> RootObject;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(FileContent);

	if (!FJsonSerializer::Deserialize(Reader, RootObject) || !RootObject.IsValid())
	{
		XIAO_LOG(Error, TEXT("Failed to parse JSON for \"%s\"."), *InProjectFilePath);
		return false;
	}

	const TArray<TSharedPtr<FJsonValue>>* PluginsArray;
	bool bFind = false;
	if (RootObject->TryGetArrayField(TEXT("Plugins"), PluginsArray))
	{
		TArray<TSharedPtr<FJsonValue>> NewPluginsArray;

		for (const TSharedPtr<FJsonValue>& PluginValue : *PluginsArray)
		{
			const TSharedPtr<FJsonObject>* PluginObjPtr = nullptr;
			if (PluginValue->TryGetObject(PluginObjPtr) && PluginObjPtr)
			{
				FString Name;
				if ((*PluginObjPtr)->TryGetStringField(TEXT("Name"), Name) && Name == InPluginNameToRemove)
				{
					XIAO_LOG(Log, TEXT("Removing plugin: %s"), *Name);
					bFind = true;
					continue; // Skip this plugin (i.e. delete it)
				}

				NewPluginsArray.Add(PluginValue); // Keep others
			}
		}

		RootObject->SetArrayField("Plugins", NewPluginsArray);
	}

	if (!bFind)
	{
		XIAO_LOG(Log, TEXT("No Plugins array found in project file \"%s\"."), *InProjectFilePath);
		return true;
	}

	FString OutputString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
	if (!FJsonSerializer::Serialize(RootObject.ToSharedRef(), Writer))
	{
		XIAO_LOG(Error, TEXT("Failed to serialize modified JSON."));
		return false;
	}

	if (!FFileHelper::SaveStringToFile(OutputString, *InProjectFilePath))
	{
		XIAO_LOG(Error, TEXT("Failed to save modified project file for \"%s\"."), *InProjectFilePath);
		return false;
	}

	return true;
}

static bool InstallComponent(const FInstallFolder& InDesc)
{
	static const FString SUBTStr = TEXT("UnrealBuildTool");
	static const FString SAutomationTool = TEXT("AutomationTool");
	static const FString SAutomationUtils = TEXT("AutomationUtils");
	static const FString SUBACStr = TEXT("UBAC");
	static const FString SUbaControllerPlugin = TEXT("UbaCompatibleController");
	static const FString SXGEControllerPlugin = TEXT("XGEController");
	static const FString SCSStr = TEXT(".cs");
	static const FString SEngineStr = TEXT("Engine");

	const FString SrcEngineDir = FPaths::ConvertRelativePathToFull(FPaths::Combine(FPaths::GetPath(FPlatformProcess::ExecutablePath()), TEXT("../../")));
	const FString SrcUBTDir = FPaths::ConvertRelativePathToFull(FPaths::Combine(SrcEngineDir, TEXT("Binaries/DotNET"), SUBTStr, SUBACStr));
	const FString SrcPluginDir = FPaths::ConvertRelativePathToFull(FPaths::Combine(SrcEngineDir, TEXT("Plugins")));
	const FString SourceToUBT = FString::Printf(TEXT("Source/Programs/%s"), *SUBTStr);
	auto& Platform = FPlatformFileManager::Get().GetPlatformFile();

	const bool bSouceEngine = InDesc.Type;
	const FString EngineVersion = GetEnghineVersion(InDesc.EngineVersion);
	XIAO_LOG(Log, TEXT("EngineVersion::%s"), *EngineVersion);
	// Executor相关源文件
	if (bSouceEngine)
	{
		XIAO_LOG(Log, TEXT("InstallUBT::SouceEngine!"));
		const FString SrcXiaoExecutorPath = FPaths::ConvertRelativePathToFull(FPaths::Combine(SrcUBTDir, EngineVersion, TEXT("UBACompatibleExecutor.cs")));
		const FString DesXiaoExecutorPath = FPaths::ConvertRelativePathToFull(FPaths::Combine(InDesc.Folder, SEngineStr, SourceToUBT, TEXT("Executors/Experimental/UBACompatibleExecutor.cs")));
		_CopyFile(InDesc.bInstall, SrcXiaoExecutorPath, DesXiaoExecutorPath, SCSStr);

		const FString SrcActionGraphPath = FPaths::ConvertRelativePathToFull(FPaths::Combine(SrcUBTDir, EngineVersion, TEXT("ActionGraph.cs")));
		const FString DesActionGraphPath = FPaths::ConvertRelativePathToFull(FPaths::Combine(InDesc.Folder, SEngineStr, SourceToUBT, TEXT("System/ActionGraph.cs")));
		_CopyFile(InDesc.bInstall, SrcActionGraphPath, DesActionGraphPath, SCSStr);

		const FString SrcBuildModePath = FPaths::ConvertRelativePathToFull(FPaths::Combine(SrcUBTDir, EngineVersion, TEXT("BuildMode.cs")));
		const FString DesBuildModePath = FPaths::ConvertRelativePathToFull(FPaths::Combine(InDesc.Folder, SEngineStr, SourceToUBT, TEXT("Modes/BuildMode.cs")));
		_CopyFile(InDesc.bInstall, SrcBuildModePath, DesBuildModePath, SCSStr);

		const FString SrcBuildConfigPath = FPaths::ConvertRelativePathToFull(FPaths::Combine(SrcUBTDir, EngineVersion, TEXT("BuildConfiguration.cs")));
		const FString DesBuildConfigPath = FPaths::ConvertRelativePathToFull(FPaths::Combine(InDesc.Folder, SEngineStr, SourceToUBT, TEXT("Configuration/BuildConfiguration.cs")));
		_CopyFile(InDesc.bInstall, SrcBuildConfigPath, DesBuildConfigPath, SCSStr);
	}

	// 直接拷贝UnrealBuildTool.dll
	{
		XIAO_LOG(Log, TEXT("InstallUBT::BuildEngine!"));
		const FString DesUBTDir = FPaths::Combine(InDesc.Folder, TEXT("Engine/Binaries/DotNET"));

		FString Extension = SDllExtension;

		// UnrealEngine 4 版本的UnrealBuildTool都是可执行文件
		if (EngineVersion.StartsWith(TEXT("4")))
		{
			Extension = TEXT(".exe");
		}
		
		const FString UnrealBuildToolFile = FString::Printf(TEXT("%s%s"), *SUBTStr, *Extension);
		const FString SrcUBTDll = FPaths::ConvertRelativePathToFull(FPaths::Combine(SrcUBTDir, EngineVersion, UnrealBuildToolFile));
		FString DesUBTDll;
		if (!EngineVersion.StartsWith(TEXT("4")))
		{
			// DotNET/UnrealBuildTool/UnrealBuildTool.dll
			DesUBTDll = FPaths::ConvertRelativePathToFull(FPaths::Combine(DesUBTDir, SUBTStr, UnrealBuildToolFile));
		}
		else
		{
			// DotNet/UnrealBuildTool.exe
			DesUBTDll = FPaths::ConvertRelativePathToFull(FPaths::Combine(DesUBTDir, UnrealBuildToolFile));
		}
		_CopyFile(InDesc.bInstall, SrcUBTDll, DesUBTDll, Extension);

		// 公版引擎
		if (!bSouceEngine)
		{
			// Engine/Source/Programs/UnrealBuildTool/obj/Development/UnrealBuildTool.dll
			DesUBTDll = FPaths::ConvertRelativePathToFull(FPaths::Combine(InDesc.Folder, TEXT("Engine/Source/Programs/UnrealBuildTool/obj/Development"), UnrealBuildToolFile));
			_CopyFile(InDesc.bInstall, SrcUBTDll, DesUBTDll, Extension);
		}

		if (!EngineVersion.StartsWith(TEXT("4")))
		{
			// DotNET/AutomationTool/UnrealBuildTool.dll
			DesUBTDll = FPaths::ConvertRelativePathToFull(FPaths::Combine(DesUBTDir, SAutomationTool, UnrealBuildToolFile));
			_CopyFile(InDesc.bInstall, SrcUBTDll, DesUBTDll, Extension);

			// DotNET/AutomationTool/AutomationUtils/UnrealBuildTool.dll
			DesUBTDll = FPaths::ConvertRelativePathToFull(FPaths::Combine(DesUBTDir, SAutomationTool, SAutomationUtils, UnrealBuildToolFile));
			_CopyFile(InDesc.bInstall, SrcUBTDll, DesUBTDll, Extension);
		}
	}

	// 拷贝或卸载插件程序
	{
		const FString DesPluginDir = FPaths::ConvertRelativePathToFull(FPaths::Combine(InDesc.Folder, TEXT("Engine/Plugins/Marketplace")));
		const FString SrcXiaoControllerDir = FPaths::ConvertRelativePathToFull(FPaths::Combine(SrcPluginDir, SUbaControllerPlugin, EngineVersion, SUbaControllerPlugin));
		const FString DesXiaoControllerDir = FPaths::ConvertRelativePathToFull(DesPluginDir, SUbaControllerPlugin);
		// 清理原始文件
		if (FPaths::DirectoryExists(DesXiaoControllerDir))
		{
			if (Platform.DeleteDirectoryRecursively(*DesXiaoControllerDir))
			{
				XIAO_LOG(Log, TEXT("DeleteDirectoryRecursively %s"), *DesXiaoControllerDir);
			}
		}

		if (InDesc.bInstall && InDesc.bPluginInstall)
		{
			if (!Platform.CopyDirectoryTree(*DesXiaoControllerDir, *SrcXiaoControllerDir, true))
			{
				XIAO_LOG(Error, TEXT("CopyDirectoryTree failed::%s"), *DesXiaoControllerDir);
			}

			// 插件依赖的第三方库拷贝 引擎版本小于等于5.1
			if (EngineVersion.StartsWith(TEXT("4")) || EngineVersion.StartsWith(TEXT("5.1")))
			{
				const FString DesThridPartyDir = FPaths::ConvertRelativePathToFull(FPaths::Combine(DesXiaoControllerDir, TEXT("ThirdParty")));
				const FString SrcThirdPartyDir = FPaths::ConvertRelativePathToFull(FPaths::Combine(SrcEngineDir, TEXT("ThirdParty")));
				if (!Platform.CopyDirectoryTree(*DesThridPartyDir, *SrcThirdPartyDir, true))
				{
					XIAO_LOG(Error, TEXT("CopyDirectoryTree failed::%s"), *DesXiaoControllerDir);
				}
			}

			// 源码引擎需要删除Binaries和InterMediate
			if (bSouceEngine)
			{
				const FString BinariesDir = FPaths::ConvertRelativePathToFull(FPaths::Combine(DesXiaoControllerDir, TEXT("Binaries")));
				if (FPaths::DirectoryExists(BinariesDir))
				{
					Platform.DeleteDirectoryRecursively(*BinariesDir);
					XIAO_LOG(Display, TEXT("DeleteDirectoryRecursively::%s"), *BinariesDir);
				}
				const FString IntermediateDir = FPaths::ConvertRelativePathToFull(FPaths::Combine(DesXiaoControllerDir, TEXT("Intermediate")));
				if (FPaths::DirectoryExists(IntermediateDir))
				{
					Platform.DeleteDirectoryRecursively(*IntermediateDir);
					XIAO_LOG(Display, TEXT("DeleteDirectoryRecursively::%s"), *IntermediateDir);
				}

				// 修改build.cs 注释文件
				const FString BuildFile = FPaths::ConvertRelativePathToFull(FPaths::Combine(DesXiaoControllerDir, TEXT("Source/UbaCompatibleController/UbaCompatibleController.build.cs")));
				const TArray<uint32> CommentLines = { 9, 10 };
				_AnnotateFile(BuildFile, CommentLines);
			}
			// 公版引擎插件的BuildID修改
			else
			{
				const FString SrcModulesFile = FPaths::ConvertRelativePathToFull(FPaths::Combine(InDesc.Folder, TEXT("Engine/Plugins"), SXGEControllerPlugin, TEXT("Binaries"), SPlatformName, TEXT("UnrealEditor.modules")));
				const FString DesModulesFile = FPaths::ConvertRelativePathToFull(FPaths::Combine(DesXiaoControllerDir, TEXT("Binaries"), SPlatformName, TEXT("UnrealEditor.modules")));
				_ModifyBuildID(DesModulesFile, SrcModulesFile);
			}
		}
		
		if (!InDesc.bInstall || !InDesc.bPluginInstall)
		{
			// 移除项目文件中的插件描述
			TSet<FString> ProjectFilePaths;
			EnumerateProjectsKnownByEngine(InDesc.EngineVersion, ProjectFilePaths);
			for (const auto& ProjectPath : ProjectFilePaths)
			{
				RemovePluginFromUProject(ProjectPath, SUbaControllerPlugin);
			}
		}

		// XGEPlugin设置是否加载
		const FString XGEControllerPluginFile = FPaths::ConvertRelativePathToFull(FPaths::Combine(InDesc.Folder, TEXT("Engine/Plugins/XGEController/XGEController.uplugin")));
		_EditPlugin(XGEControllerPluginFile, !InDesc.bInstall);
	}

	// 引擎中的Restricted中的构建配置文件
	FString DesBuildConfigPath = FPaths::ConvertRelativePathToFull(FPaths::Combine(InDesc.Folder, TEXT("Engine/Restricted/NotForLicensees/Programs"), SUBTStr, SBuildConfigXml));
	if (FPaths::FileExists(DesBuildConfigPath))
	{
		EditConfigXml(DesBuildConfigPath, InDesc.bInstall);
	}

	// 引擎中的Saved中的构建配置文件
	DesBuildConfigPath = FPaths::ConvertRelativePathToFull(FPaths::Combine(InDesc.Folder, TEXT("Engine/Saved"), SUBTStr, SBuildConfigXml));
	if (FPaths::FileExists(DesBuildConfigPath))
	{
		EditConfigXml(DesBuildConfigPath, InDesc.bInstall);
	}

	return true;
}

static bool InstallUBT(const bool bInstallOrUninstall = true)
{
	// 核心组件安装
	int Index = 0;
	for (const auto& EngineFolder : GInstallSettings.EngineFolders)
	{
		const bool bSouceEngine = GInstallSettings.EngineTypes[Index] == 1 ? true : false;
		FInstallFolder InstallDesc(EngineFolder, bSouceEngine, bInstallOrUninstall);
		InstallDesc.EngineVersion = GInstallSettings.EngineVersions[Index];
		InstallComponent(InstallDesc);
		++Index;
	}

	// 配置文件修改
	auto& Platform = FPlatformFileManager::Get().GetPlatformFile();
	const FString SrcEngineDir = FPaths::ConvertRelativePathToFull(FPaths::Combine(FPaths::GetPath(FPlatformProcess::ExecutablePath()), TEXT("../../")));
	const FString SrcPluginDir = FPaths::ConvertRelativePathToFull(FPaths::Combine(SrcEngineDir, TEXT("Plugins")));
	const FString SrcBuildConfigPath = FPaths::ConvertRelativePathToFull(FPaths::Combine(SrcPluginDir, SBuildConfigXml));
	/*
	* In addition to being added to the generated Unreal Engine (UE) project under the Config/UnrealBuildTool folder, Unreal Build Tool (UBT) reads settings from XML config files in the following locations 
	
		on Windows:
		Engine/Saved/UnrealBuildTool/BuildConfiguration.xml
		USER/AppData/Roaming/Unreal Engine/UnrealBuildTool/BuildConfiguration.xml
		My Documents/Unreal Engine/UnrealBuildTool/BuildConfiguration.xml

		On Linux and Mac, the following paths are used instead:
		/Users/USER/.config//Unreal Engine/UnrealBuildTool/BuildConfiguration.xml
		/Users/USER/Unreal Engine/UnrealBuildTool/BuildConfiguration.xml
	*/
	// 全局配置文件
	FString DesBuildConfigPath = FPaths::ConvertRelativePathToFull(FString::Printf(TEXT("%sUnreal Engine/UnrealBuildTool/%s"),
#if PLATFORM_WINDOWS
		*GetWindowsKnownDir(FOLDERID_ProgramData), 
#else
		TEXT("/Users/Shared"),
#endif
		*SBuildConfigXml));
	const FString UnrealBuildToolFolder = FPaths::GetPath(DesBuildConfigPath);
	if (!FPaths::DirectoryExists(UnrealBuildToolFolder))
	{
		if (!Platform.CreateDirectoryTree(*UnrealBuildToolFolder))
		{
			XIAO_LOG(Error, TEXT("CreateDirectoryTree %s failed with LastError::%d"), *UnrealBuildToolFolder, FPlatformMisc::GetLastError());
		}
	}
	if (FPaths::FileExists(DesBuildConfigPath))
	{
		EditConfigXml(DesBuildConfigPath, bInstallOrUninstall);
	}
	else if (bInstallOrUninstall && !Platform.CopyFile(*DesBuildConfigPath, *SrcBuildConfigPath, EPlatformFileRead::AllowWrite))
	{
		XIAO_LOG(Error, TEXT("Copy BuildConfiguration.xml SrcFile::%s -> DesFile::%s With LastError::%d"), *SrcBuildConfigPath, *DesBuildConfigPath, FPlatformMisc::GetLastError());
	}

#if PLATFORM_WINDOWS
	// AppData Local
	DesBuildConfigPath = FPaths::ConvertRelativePathToFull(FString::Printf(TEXT("%sUnreal Engine/UnrealBuildTool/%s"),
		*GetWindowsKnownDir(FOLDERID_LocalAppData), 
		*SBuildConfigXml));
	EditConfigXml(DesBuildConfigPath, bInstallOrUninstall);

	// Documents 
	DesBuildConfigPath = FPaths::ConvertRelativePathToFull(FString::Printf(TEXT("%sUnreal Engine/UnrealBuildTool/%s"),
		*GetWindowsKnownDir(FOLDERID_Documents),
		*SBuildConfigXml));
	EditConfigXml(DesBuildConfigPath, bInstallOrUninstall);

	// AppData Roaming
	DesBuildConfigPath = FPaths::ConvertRelativePathToFull(FString::Printf(TEXT("%sUnreal Engine/UnrealBuildTool/%s"),
		*GetWindowsKnownDir(FOLDERID_RoamingAppData),
		*SBuildConfigXml));
	EditConfigXml(DesBuildConfigPath, bInstallOrUninstall);
#endif
	return true;
}
