#include "Install.h"
#include "XiaoLog.h"
#include "XiaoAgent.h"
#include "XiaoShareField.h"
#include "Database/LicenseDatabase.h"
#include "Platform/Firewall.h"
#include "XiaoShareRedis.h"
#include "XiaoCompressor.h"
#include "agent.pb.h"

using namespace XiaoAppName;

static const FString SMonFileExtension = TEXT(".uba");
static const FString SMonitorFileDisplayName = TEXT("XiaoBuild-MonitorFile");
// static const FString SXiaoCompressPath = GetXiaoAppPath(SXiaoCompressor);
static const FString SXiaoAppPath = GetXiaoAppPath(SBuildApp);
static FString STempFolder;

#if PLATFORM_WINDOWS
static const FString SEnvironmentKey = TEXT("System\\CurrentControlSet\\Control\\Session Manager\\Environment");

static HRESULT CreateStartMenuShortCut(const FString& pszTargetPath, const FString& pszArgs, const FString& pszDescription, const FString& pszIconLocation, int iIcon, const FString& InShortCut)
{
	HRESULT hr = CoInitialize(NULL);
	if (FAILED(hr)) return hr;

	IShellLink* psl = nullptr;
	// Create the link object
	hr = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (void**)&psl);
	if (SUCCEEDED(hr)) 
	{
		// Set the path to the shortcut target and add any arguments
		psl->SetPath(*pszTargetPath);
		psl->SetArguments(*pszArgs);

		psl->SetWorkingDirectory(*FPaths::GetPath(pszTargetPath));

		// Optionally set the description and icon for the shortcut
		psl->SetDescription(*pszDescription);
		psl->SetIconLocation(*pszIconLocation, iIcon);

		IPersistFile* ppf = nullptr;
		// Query for the IPersistFile interface to save the shortcut
		hr = psl->QueryInterface(IID_IPersistFile, (void**)&ppf);
		if (SUCCEEDED(hr)) 
		{
			// Get the path to the start menu programs folder
			PWSTR szStartMenuPath = nullptr;
			hr = SHGetKnownFolderPath(FOLDERID_StartMenu, 0, NULL, &szStartMenuPath);
			if (SUCCEEDED(hr))
			{
				const FString MenuPath = FPaths::ConvertRelativePathToFull(FPaths::Combine(szStartMenuPath, TEXT("XiaoBuild"), InShortCut));
				const FString MenuFolder = FPaths::GetPath(MenuPath);
				if (!FPaths::DirectoryExists(MenuFolder))
				{
					IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
					PlatformFile.CreateDirectoryTree(*MenuFolder);
				}

				// Save the shortcut
				hr = ppf->Save(*MenuPath, 1);
			}
			else
			{
				XIAO_LOG(Error, TEXT("Can\'t get the StartMenu Folder!"));
			}
			ppf->Release();
		}
		psl->Release();
	}

	CoUninitialize();
	return hr;
}
#endif


static void RegistApp(const FString& InAppName)
{
#if PLATFORM_WINDOWS
#pragma region RegistApp
	const FString AppPath = GetXiaoAppPath(InAppName);
	HKEY HKey;
	const FString AppPathsKey = TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\App Paths");
	auto Result = RegOpenKeyEx(HKEY_LOCAL_MACHINE, *AppPathsKey, 0, KEY_READ | KEY_WRITE, &HKey);
	if (Result != ERROR_SUCCESS)
	{
		XIAO_LOG(Error, TEXT("Failed to Open registry key for AppPaths ErrorCode::%d"), Result);
		return;
	}

	const FString KeyName = FPaths::GetCleanFilename(AppPath);
	Result = RegCreateKeyEx(HKey, *KeyName, 0, nullptr, 0, KEY_SET_VALUE, nullptr, &HKey, nullptr);
	if (Result != ERROR_SUCCESS)
	{
		XIAO_LOG(Error, TEXT("Failed to RegCreateKeyEx for AppPaths ErrorCode::%d"), Result);
		return;
	}
	auto Len = wcslen(*KeyName) * sizeof(wchar_t);
	RegSetValue(HKey, nullptr, REG_SZ, *KeyName, Len);
	RegCloseKey(HKey);

	const FString UnInstallKey = TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall");
	Result = RegOpenKeyEx(HKEY_LOCAL_MACHINE, *UnInstallKey, 0, KEY_READ | KEY_WRITE, &HKey);
	if (Result != ERROR_SUCCESS)
	{
		XIAO_LOG(Error, TEXT("Failed to Open registry key for UnInstall ErrorCode::%d"), Result);
		return;
	}

	const FString SXiaoAppKey = TEXT("XiaoBuild");
	HKEY AppInfoKey;
	Result = RegCreateKeyEx(HKey, *SXiaoAppKey, 0, nullptr, 0, KEY_SET_VALUE, nullptr, &AppInfoKey, nullptr);
	if (Result != ERROR_SUCCESS)
	{
		XIAO_LOG(Error, TEXT("Failed to Open registry key for XiaoApp ErrorCode::%d"), Result);
		return;
	}
	const FString DisplayName = TEXT("XiaoBuild");
	Len = wcslen(*DisplayName) * sizeof(wchar_t);
	Result = RegSetValueEx(AppInfoKey, TEXT("DisplayName"), NULL, REG_SZ, (BYTE*)*DisplayName, Len);
	if (Result != ERROR_SUCCESS)
	{
		XIAO_LOG(Error, TEXT("Failed to RegSetValueEx for DisplayName ErrorCode::%d"), Result);
	}
	FString DisplayIcon = AppPath;
	DisplayIcon.ReplaceInline(TEXT("/"), TEXT("\\"));
	Len = wcslen(*DisplayIcon) * sizeof(wchar_t);
	Result = RegSetValueEx(AppInfoKey, TEXT("DisplayIcon"), NULL, REG_SZ, (BYTE*)*DisplayIcon, Len);
	if (Result != ERROR_SUCCESS)
	{
		XIAO_LOG(Error, TEXT("Failed to RegSetValueEx for DisplayIcon ErrorCode::%d"), Result);
	}

	FString InstallLocation = GInstallSettings.InstallFolder;
	InstallLocation.ReplaceInline(TEXT("/"), TEXT("\\"));
	Len = wcslen(*InstallLocation) * sizeof(wchar_t);
	Result = RegSetValueEx(AppInfoKey, TEXT("InstallLocation"), NULL, REG_SZ, (BYTE*)*InstallLocation, Len);
	if (Result != ERROR_SUCCESS)
	{
		XIAO_LOG(Error, TEXT("Failed to RegSetValueEx for InstallLocation ErrorCode::%d"), Result);
	}

	const FString Publisher = TEXT("XiaoStudio");
	Len = wcslen(*Publisher) * sizeof(wchar_t);
	Result = RegSetValueEx(AppInfoKey, TEXT("Publisher"), NULL, REG_SZ, (BYTE*)*Publisher, Len);
	if (Result != ERROR_SUCCESS)
	{
		XIAO_LOG(Error, TEXT("Failed to RegSetValueEx for Publisher ErrorCode::%d"), Result);
	}

	const FString DisplayVersion = XB_VERSION_STRING;
	Len = wcslen(*DisplayVersion) * sizeof(wchar_t);
	Result = RegSetValueEx(AppInfoKey, TEXT("DisplayVersion"), NULL, REG_SZ, (BYTE*)*DisplayVersion, Len);
	if (Result != ERROR_SUCCESS)
	{
		XIAO_LOG(Error, TEXT("Failed to RegSetValueEx for DisplayVersion ErrorCode::%d"), Result);
	}

	FString UnInstallString = FPaths::ConvertRelativePathToFull(FPaths::Combine(FPlatformProcess::GetCurrentWorkingDirectory(), TEXT("../../../uninstall.exe")));
	UnInstallString.ReplaceInline(TEXT("/"), TEXT("\\"));
	Len = wcslen(*UnInstallString) * sizeof(wchar_t);
	Result = RegSetValueEx(AppInfoKey, TEXT("UninstallString"), NULL, REG_SZ, (BYTE*)*UnInstallString, Len);
	if (Result != ERROR_SUCCESS)
	{
		XIAO_LOG(Error, TEXT("Failed to RegSetValueEx for UninstallString ErrorCode::%d"), Result);
	}

	const DWORD NoModify = 1;
	Result = RegSetValueEx(AppInfoKey, TEXT("NoModify"), NULL, REG_DWORD, (BYTE*)(&NoModify), sizeof(DWORD));
	if (Result != ERROR_SUCCESS)
	{
		XIAO_LOG(Error, TEXT("Failed to RegSetValueEx for NoModify ErrorCode::%d"), Result);
	}
	const DWORD NoRepair = 1;
	Result = RegSetValueEx(AppInfoKey, TEXT("NoRepair"), NULL, REG_DWORD, (BYTE*)(&NoRepair), sizeof(DWORD));
	if (Result != ERROR_SUCCESS)
	{
		XIAO_LOG(Error, TEXT("Failed to RegSetValueEx for NoRepair ErrorCode::%d"), Result);
	}
	const DWORD EstimatedSize = 161353;
	Result = RegSetValueEx(AppInfoKey, TEXT("EstimatedSize"), NULL, REG_DWORD, (BYTE*)(&EstimatedSize), sizeof(DWORD));
	if (Result != ERROR_SUCCESS)
	{
		XIAO_LOG(Error, TEXT("Failed to RegSetValueEx for EstimatedSize ErrorCode::%d"), Result);
	}

	RegFlushKey(AppInfoKey);
	RegCloseKey(AppInfoKey);
#pragma endregion
#endif
}

static bool RegistHomePath()
{
	UpdateMessage(0.16f, TEXT("Register environment..."));

#if PLATFORM_WINDOWS
	HKEY HKey;
	const FString PathEnvironmentKey(TEXT("Path"));
	FString PathVal = FPlatformMisc::GetEnvironmentVariable(*PathEnvironmentKey);
	const FString CurrentWorkingPath = FPlatformProcess::GetCurrentWorkingDirectory();

	if (PathVal.Contains(CurrentWorkingPath))
	{
		return true;
	}

	PathVal += SSepetator + CurrentWorkingPath;
	auto Result = RegOpenKeyEx(HKEY_LOCAL_MACHINE, *SEnvironmentKey, 0, KEY_READ | KEY_WRITE, &HKey);
	if (Result != ERROR_SUCCESS)
	{
		XIAO_LOG(Error, TEXT("Failed to Open registry key for Environment ErrorCode::%d"), Result);
		return false;
	}

	// PATH 环境变量
	auto Len = wcslen(*PathVal) * sizeof(wchar_t);
	Result = RegSetValueEx(HKey, *PathEnvironmentKey, NULL, REG_SZ, (BYTE*)*PathVal, Len);
	if (Result != ERROR_SUCCESS)
	{
		const FString Message = FString::Printf(TEXT("Failed to Set registry key for \"PATH\" ErrorCode::%d"), Result);
		UpdateError(Message);
		return false;
	}

	// XIAO_HOME 环境变量
	const FString XiaoHome = FPaths::ConvertRelativePathToFull(GInstallSettings.InstallFolder);
	Len = wcslen(*XiaoHome) * sizeof(wchar_t);
	Result = RegSetValueEx(HKey, *SXiaoHome, NULL, REG_SZ, (BYTE*)*XiaoHome, Len);
	if (Result != ERROR_SUCCESS)
	{
		const FString Message = FString::Printf(TEXT("Failed to Set registry key for \"XIAO_HOME\" ErrorCode::%d"), Result);
		UpdateError(Message);
		return false;
	}

	// 广播消息 系统环境变量已经修改
	SendMessageTimeoutW(HWND_BROADCAST, WM_SETTINGCHANGE, 0, (LPARAM)L"Environment", SMTO_ABORTIFHUNG, 5000, NULL);
#endif

	return true;
}

static bool RegistMisc()
{
#if PLATFORM_WINDOWS
	UpdateMessage(0.55f, TEXT("Register file extension..."));

	// 删除已有的特定文件后缀打开方式
	HKEY UbaKey;
	const FString UbaFileExtsKey = TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\FileExts\\.uba");
	auto Result = RegOpenKeyEx(HKEY_CURRENT_USER, *UbaFileExtsKey, 0, KEY_READ | KEY_WRITE, &UbaKey);
	if (Result == ERROR_SUCCESS)
	{
		const FString OpenWithProgidsKey = TEXT("OpenWithProgids");
		RegDeleteKeyW(UbaKey, *OpenWithProgidsKey);
		const FString UserChoiceKey = TEXT("UserChoice");
		RegDeleteKeyW(UbaKey, *UserChoiceKey);
	}

	auto Len = wcslen(*SMonitorFileDisplayName) * sizeof(wchar_t);
	HKEY HKey;
	Result = RegCreateKeyEx(HKEY_CLASSES_ROOT, *SMonFileExtension, 0, nullptr, 0, KEY_SET_VALUE, nullptr, &HKey, nullptr);
	if (Result == ERROR_SUCCESS)
	{
		RegSetValue(HKey, nullptr, REG_SZ, *SMonitorFileDisplayName, Len);
		RegCloseKey(HKey);
	}
	else
	{
		const FString Message = FString::Printf(TEXT("Failed to create registry key for file extension! ErrorCode::%d"), Result);
		UpdateError(Message);
		return false;
	}

	HKEY HOpenKey;
#pragma region MonitorMenu
	UpdateMessage(0.58f, TEXT("Register file menu..."));
	Result = RegCreateKeyEx(HKEY_CLASSES_ROOT, *SMonitorFileDisplayName, 0, nullptr, 0, KEY_SET_VALUE, nullptr, &HKey, nullptr);
	if (Result == ERROR_SUCCESS)
	{
		RegSetValue(HKey, nullptr, REG_SZ, *SMonitorFileDisplayName, Len);
		// 注册默认双击调用的程序
		Result = RegCreateKeyEx(HKey, L"shell\\open\\command", 0, nullptr, 0, KEY_SET_VALUE, nullptr, &HOpenKey, nullptr);
		if (Result == ERROR_SUCCESS)
		{
			// 必须是Windows的路径分隔符
			const FString Execute = FString::Printf(TEXT("\"%s\" -app=%s -file=\"%%1\""), *SXiaoAppPath, *XiaoAppName::SBuildMonitor);
			Len = wcslen(*Execute) * sizeof(wchar_t);
			RegSetValue(HOpenKey, nullptr, REG_SZ, *Execute, Len);
			RegCloseKey(HOpenKey);
		}
		else
		{
			const FString Message = FString::Printf(TEXT("Failed to create registry key for associated program! ErrorCode::%d"), Result);
			UpdateError(Message);
			return false;
		}

		// 注册默认图标
		HKEY HIconKey;
		Result = RegCreateKeyEx(HKey, L"DefaultIcon", 0, nullptr, 0, KEY_SET_VALUE, nullptr, &HIconKey, nullptr);
		if (Result == ERROR_SUCCESS)
		{
			// 必须是Windows的路径分隔符
			Len = wcslen(*SXiaoAppPath) * sizeof(wchar_t);
			RegSetValue(HIconKey, nullptr, REG_SZ, *SXiaoAppPath, Len);
			RegCloseKey(HIconKey);
		}
		RegCloseKey(HKey);
	}
	else
	{
		const FString Message = FString::Printf(TEXT("Failed to create registry key for %s! ErrorCode::%d"), *SMonitorFileDisplayName, Result);
		UpdateError(Message);
		return false;
	}
#pragma endregion

#pragma region ShortcutKey
	const FString ResourceTryFolder = FPaths::ConvertRelativePathToFull(FPaths::Combine(FPaths::EngineDir(), TEXT("Content/Slate/XiaoBuild/Icons")));
	const FString IconPath = FPaths::Combine(ResourceTryFolder, TEXT("Tray/Status/normalTray.ico"));
	CreateStartMenuShortCut(GetXiaoAppPath(SBuildTray), TEXT(""), TEXT("Xiao Tray"), IconPath, 0, TEXT("XiaoTray.lnk"));

	CreateStartMenuShortCut(SXiaoAppPath, FString::Printf(TEXT("-app=%s"), *XiaoAppName::SBuildMonitor), TEXT("Build Monitor"), IconPath, 0, TEXT("XiaoMonitor.lnk"));

	CreateStartMenuShortCut(SXiaoAppPath, FString::Printf(TEXT("-app=%s"), *XiaoAppName::SBuildCoordiManager), TEXT("Coorinator Manager"), IconPath, 0, TEXT("XiaoCoordinator.lnk"));

	CreateStartMenuShortCut(SXiaoAppPath, FString::Printf(TEXT("-app=%s"), *XiaoAppName::SBuildAgentSettings), TEXT("Agent Settings"), IconPath, 0, TEXT("XiaoSettings.lnk"));

#pragma endregion

#pragma region XiaoCompressor
	//Result = RegCreateKeyEx(HKey, L"*\\shellex\\ContextMenuHandlers\\CompressWithXiao", 0, nullptr, 0, KEY_SET_VALUE, nullptr, &HOpenKey, nullptr);
	//if (Result == ERROR_SUCCESS)
	//{
	//	// 必须是Windows的路径分隔符
	//	const FString Execute = FString::Printf(TEXT("\"%s\" -file=\"%%1\""), *SXiaoCompressPath);
	//	Len = wcslen(*Execute) * sizeof(wchar_t);
	//	RegSetValue(HOpenKey, nullptr, REG_SZ, *Execute, Len);
	//	RegCloseKey(HOpenKey);
	//}
	//else
	//{
	//	XIAO_LOG(Error, TEXT("Failed to create registry key for associated program! ErrorCode::%d"), Result);
	//	return false;
	//}
#pragma endregion

#endif // PLATFORM_WINDOWS

	return true;
}

static bool AutoRun()
{
	UpdateMessage(0.51f, TEXT("Register auto run..."));

#if PLATFORM_WINDOWS
	if (GInstallSettings.bEnableAutoTray)
	{
		HKEY HKey;
		const FString RunKey = TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Run");
		DWORD DwDisposition = REG_OPENED_EXISTING_KEY;
		const auto Result = RegCreateKeyEx(HKEY_LOCAL_MACHINE, *RunKey, 0, nullptr, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, nullptr, &HKey, &DwDisposition);
		if (Result == ERROR_SUCCESS)
		{
			// windows上路径用双引号包裹，然后使用windows的分隔符是有效的
			const FString XiaoTrayPath = GetXiaoAppPath(SBuildTray);
			const FString Params = FString::Printf(TEXT("\"%s\" -LOG=XiaoBuildTray.log"), *XiaoTrayPath.Replace(TEXT("/"), TEXT("\\")));
			const auto Len = wcslen(*Params) * sizeof(wchar_t);
			RegSetValueEx(HKey, *SBuildTray, 0, REG_SZ, (BYTE*)*Params, Len);
			RegCloseKey(HKey);
			UpdateMessage(0.51f, TEXT("RegSetValueEx success..."));
		}
		else
		{
			const FString Message = FString::Printf(TEXT("Failed to create registry key for autorun! ErrorCode::%d"), Result);
			UpdateError(Message);
			return false;
		}
	}
#endif

	return true;
}

static bool RegistService()
{
	// 服务程序注册
	UpdateMessage(0.2f, TEXT("Register service begin..."));

	FString Params;
	if ((GInstallSettings.InstallType & CT_Coordinator) || (GInstallSettings.InstallType & CT_BackCoordi))
	{
		Params = FString::Printf(TEXT("%s -%s"), *SBuildCoordiService, *AppControl::SInstall);
		RunXiaoApp(SBuildCoordiService, *Params);
		UpdateMessage(0.4f, FString::Printf(TEXT("Install \"%s\" service finish..."), *SBuildCoordiService));	
	}
	if (GInstallSettings.InstallType & CT_Agent)
	{
		Params = FString::Printf(TEXT("%s -%s"), *SBuildAgentService, *AppControl::SInstall);
		RunXiaoApp(SBuildAgentService, *Params);
		UpdateMessage(0.45f, FString::Printf(TEXT("Install \"%s\" service finish..."), *SBuildAgentService));

		/*
		int32 RtnCode = -1;
#if PLATFORM_WINDOWS
		FPlatformProcess::ExecElevatedProcess(TEXT("icacls"), *FString::Printf(TEXT("\"%s\" /grant Users:F"), *BuildHistoryDir), &RtnCode);
#elif
		FPlatformProcess::ExecProcess(TEXT("chmod"), *FString::Print(TEXT("%s"), *BuildHistoryDir) &RtnCode);
#endif
		if (RtnCode != 0)
		{
			XIAO_LOG(Error, TEXT("Change Folder permissios failed::ErrorCode::%d LastError:%d"), RtnCode, FPlatformMisc::GetLastError());
		}*/

		if (!InstallUBT())
		{
			UpdateError(TEXT("Install UnrealBuildTool failed!"));
		}
	}

#if PLATFORM_WINDOWS
	if (GInstallSettings.InstallType == CT_AgentCoordiVisulizer)
	{
		RunAs(TEXT("sc"), FPlatformProcess::GetCurrentWorkingDirectory(), FString::Printf(TEXT("config %s depend=%s"), *SBuildAgentService, *SBuildCoordiService), true);
	}
#endif

	// Cache从服务器
	if ((GInstallSettings.InstallType & CT_Coordinator) || (GInstallSettings.InstallType & CT_BackCoordi))
	{
		const FString RedisConfigFile = 
#if PLATFORM_MAC
			TEXT("/Applications/XiaoApp.app/Contents/UE/Engine/Config/cache.conf");
#else
			FPaths::ConvertRelativePathToFull(FPaths::Combine(FPaths::GetPath(FPlatformProcess::ExecutablePath()), TEXT("../../Config/cache.conf")));
#endif
		if(!FPaths::FileExists(RedisConfigFile))
		{
			UpdateError(FString::Printf(TEXT("CacheConfig %s file not exist"), *RedisConfigFile));
			return false;
		}

		FString Content;
		if (!FFileHelper::LoadFileToString(Content, *RedisConfigFile))
		{
			UpdateError(FString::Printf(TEXT("LoadFileToString %s file failed"), *RedisConfigFile));
			return false;
		}

		if(GInstallSettings.InstallType & CT_Coordinator)
		{
			Content.ReplaceInline(TEXT("port 37000"), *FString::Printf(TEXT("port %u"), GInstallSettings.CoordiPort));
		}
		else
		{
			const FString SlaveOf = FString::Printf(TEXT("slaveof %s %u\n"), *GInstallSettings.CoordiIp, GInstallSettings.CoordiPort);
			int32 Index = Content.ReplaceInline(TEXT("# slaveof <masterip> <masterport>"), *SlaveOf);
			if (Index <= 0)
			{
				Content.Append(SlaveOf);
			}
		}
		FFileHelper::SaveStringToFile(Content, *RedisConfigFile);
		UpdateMessage(0.46f, FString::Printf(TEXT("Update the redis config file with master server %s:%u"), *GInstallSettings.CoordiIp, GInstallSettings.CoordiPort));
	}

	UpdateMessage(0.5f, TEXT("Register service finish..."));

	return true;
}

static bool RunService()
{
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();

	AutoRun();
	RegistMisc();

	if ((GInstallSettings.InstallType & CT_Coordinator) || (GInstallSettings.InstallType & CT_BackCoordi))
	{
		FString Error;
		if(!SetServiceState(SBuildCoordiService, true, Error))
		{
			UpdateError(FString::Printf(TEXT("Change [%s] service state FAILED::%s!"), *SBuildCoordiService, *Error));
			return false;
		}
		UpdateMessage(0.65f, FString::Printf(TEXT("startup \"%s\" service finish..."), *SBuildCoordiService));

		if (GInstallSettings.InstallType & CT_Coordinator)
		{
			FPlatformProcess::Sleep(1.0f);
			XiaoDB::FUserDetail User;
			User.Username = GInstallSettings.Username;
			User.Password = GInstallSettings.Password;

			GMasterConnection.host = "127.0.0.1";
			GMasterConnection.port = GInstallSettings.CoordiListenPort;
			GMasterConnection.keep_alive = true;

			if (XiaoRedis::TryConnectRedis())
			{
				const std::string Key = TCHAR_TO_UTF8(*GInstallSettings.Username);
				FString EncryptContent;
				if (EncryptString(User.ToJson(), XiaoEncryptKey::SAuth, EncryptContent))
				{
					const std::string Val = TCHAR_TO_UTF8(*EncryptContent);
					XiaoRedis::SRedisClient->hset(XiaoRedis::Hash::SUserDetail, Key, Val);
				}
				else
				{
					UpdateError(TEXT("Can\'t EncryptString!"));
				}
			}
			else
			{
				UpdateError(TEXT("Can\'t connect to CacheServer!"));
			}
		}
	}
	if (GInstallSettings.InstallType & CT_Agent)
	{
		FString Error;
		if(!SetServiceState(SBuildAgentService, true, Error))
		{
			UpdateError(FString::Printf(TEXT("Change [%s] service state FAILED::%s!"), *SBuildAgentService, *Error));
			return false;
		}
		UpdateMessage(0.7f, FString::Printf(TEXT("startup \"%s\" service finish..."), *SBuildAgentService));
	}

	return true;
}

bool OnInstall()
{
	UnInstall();

	UpdateMessage(0.1f, TEXT("Begin Install..."));

	UpdateMessage(0.135f, TEXT("Begin build firewall!"));
	FFirewall Firewall;
	if (!Firewall.BuildFirewall())
	{
		XIAO_LOG(Error, TEXT("BuildFirewall::Failed!"));
		return false;
	}
	UpdateMessage(0.15f, TEXT("Finish build firewall!"));
	
	if (!RegistHomePath())
	{
		return false;
	}
	if (!RegistService())
	{
		return false;
	}
	if(!RunService())
	{
		return false;
	}
	RegistApp(SBuildApp);

	UpdateMessage(0.9f, TEXT("Startup all service finish..."));
	UpdateMessage(0.99f, TEXT("Almost finish!!!"));
	UpdateMessage(0.99f, TEXT("Done!!!"));

	GProgress.Status = 1;
	GProgress.Progress = 100.0f;
	UpdateProgress(0.5f);
	
	return true;
}

static void StopService(const float InBeginProgress, const float InFinishProgress)
{
	UpdateMessage(InBeginProgress, TEXT("Stop Service begin!"));

	const FString WorkingPath = FPlatformProcess::GetCurrentWorkingDirectory();
	FString Params;
	if (IsAppRunning(SBuildAgentService))
	{
		Params = FPaths::ConvertRelativePathToFull(FPaths::Combine(WorkingPath, SBuildAgentService));
		ShutdownXiaoApp(Params);
	}

	if (IsAppRunning(SBuildCoordiService))
	{
		Params = FPaths::ConvertRelativePathToFull(FPaths::Combine(WorkingPath, SBuildCoordiService));
		ShutdownXiaoApp(Params);
	}

	UpdateMessage(InFinishProgress, TEXT("Stop Service finish!"));
}

static void DeleteService(const float InBeginProgress, const float InFinishProgress)
{
	UpdateMessage(InBeginProgress, TEXT("Delete Service begin!"));

	FString Params;
	if (GInstallSettings.InstallType & CT_Coordinator)
	{
		Params = FString::Printf(TEXT("%s -%s"), *SBuildCoordiService, *AppControl::SDelete);
		RunXiaoApp(SBuildCoordiService, *Params);
	}
	if (GInstallSettings.InstallType & CT_Agent)
	{
		Params = FString::Printf(TEXT("%s -%s"), *SBuildAgentService, *AppControl::SDelete);
		RunXiaoApp(SBuildAgentService, *Params);
	}

	UpdateMessage(InFinishProgress, TEXT("Delete Service finish!"));
}

static void UnRegistEnvirment()
{
#if PLATFORM_WINDOWS
	HKEY EnviKey;
	auto Result = RegOpenKeyEx(HKEY_LOCAL_MACHINE, *SEnvironmentKey, 0, KEY_READ | KEY_WRITE, &EnviKey);
	if (Result != ERROR_SUCCESS)
	{
		XIAO_LOG(Error, TEXT("Failed to Open registry key for Environment ErrorCode::%d"), Result);
		return;
	}

	// 清除Path环境变量中的 路径
	const FString PathEnvironmentKey(TEXT("Path"));
	FString PathVal = FPlatformMisc::GetEnvironmentVariable(*PathEnvironmentKey);
	const FString CurrentWorkingPath = SSepetator + FPlatformProcess::GetCurrentWorkingDirectory();
	PathVal = PathVal.Replace(*CurrentWorkingPath, TEXT(""));
	auto Len = wcslen(*PathVal) * sizeof(wchar_t);
	Result = RegSetValueEx(EnviKey, *PathEnvironmentKey, NULL, REG_SZ, (BYTE*)*PathVal, Len);
	if (Result != ERROR_SUCCESS)
	{
		XIAO_LOG(Error, TEXT("Failed to Set registry key for \"PATH\" ErrorCode::%d"), Result);
	}

	// 删除环境变量
	RegDeleteKeyExW(EnviKey, *SXiaoHome, KEY_WOW64_64KEY, 0);
	RegCloseKey(EnviKey);

	// 广播消息 系统环境变量已经修改
	SendMessageTimeoutW(HWND_BROADCAST, WM_SETTINGCHANGE, 0, (LPARAM)L"Environment", SMTO_ABORTIFHUNG, 5000, NULL);
#elif PLATFORM_UNIX
	;
#endif
}

static void TerminateAllApp(const FString& InAppName, const uint32 InIgnorePid)
{
	UpdateMessage(0.05f, FString::Printf(TEXT("Terminate all App [%s] begin!"), *InAppName));

	TArray<uint32> NeedTerminateProcs;
	FPlatformProcess::FProcEnumerator ProcIter;
	while (ProcIter.MoveNext())
	{
		FPlatformProcess::FProcEnumInfo ProcInfo = ProcIter.GetCurrent();
		const uint32 Pid = ProcInfo.GetPID();
		if (InIgnorePid == Pid)
		{
			continue;
		}
		const FString ProcName = ProcInfo.GetName();
		if (ProcName == InAppName)
		{
			NeedTerminateProcs.Add(Pid);
		}
	}

	for (const uint32 Pid : NeedTerminateProcs)
	{
		FProcHandle ProcHandle = FPlatformProcess::OpenProcess(Pid);
		if (ProcHandle.IsValid())
		{
			FPlatformProcess::TerminateProc(ProcHandle, true);
		}
	}
	
	UpdateMessage(0.1f, TEXT("Terminate all XiaoApp Finish!"));
}

static bool Before(const bool bInCheck)
{
	// 加载配置文件
	bool bRtn = LoadAgentSettings(SOriginalAgentSettings);
	if (bInCheck && !bRtn)
	{
		UpdateError(TEXT("读取代理参数文件失败!"));
		return false;
	}

	InstallUBT(false);

	// 先停止所有的app的运行
	TerminateAllApp(XiaoAppName::SBuildApp, 0);
	TerminateAllApp(XiaoAppName::SBuildTray, 0);

	// 先停止所有的服务程序
	StopService(0.15f, 0.2f);
	return true;
}

static bool OverrideFiles()
{
	XIAO_LOG(Log, TEXT("Override files begin!"));
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	STempFolder = FPaths::ConvertRelativePathToFull(FPaths::Combine(FPlatformProcess::UserTempDir(), FString::Printf(TEXT("%s/TEMP"), *SXiaoBuild)));
	if (FPaths::DirectoryExists(STempFolder))
	{
		if (PlatformFile.DeleteDirectoryRecursively(*STempFolder))
		{
			XIAO_LOG(Log, TEXT("Successfully deleted directory: %s"), *STempFolder);
		}
		else
		{
			XIAO_LOG(Error, TEXT("Failed to delete directory: %s"), *STempFolder);
			return false;
		}
	}

	if (!PlatformFile.CreateDirectoryTree(*STempFolder))
	{
		XIAO_LOG(Error, TEXT("Failed to create directory: %s"), *STempFolder);
		return false;
	}

	std::string Data;
	if (!GetCanSyncUpdate(Data))
	{
		XIAO_LOG(Error, TEXT("Not have sync update data"));
		return false;
	}

	TArray<uint8> CompressedBuffer;
	CompressedBuffer.SetNum(Data.size());
	FMemory::Memcpy(CompressedBuffer.GetData(), Data.data(), Data.size());

	const FString TempFile = FPaths::ConvertRelativePathToFull(FPaths::Combine(STempFolder, TEXT("TEMP.xb")));
	if (!FFileHelper::SaveArrayToFile(CompressedBuffer, *TempFile))
	{
		XIAO_LOG(Error, TEXT("Cant save data to file::%s"), *TempFile);
		return false;
	}

	FFilesProxy FileProxy;
	const FString DecompressDir = FPaths::ConvertRelativePathToFull(FPaths::Combine(STempFolder, TEXT("DECP")));
	PlatformFile.CreateDirectoryTree(*DecompressDir);
	if (!FileProxy.Decompress(TempFile, DecompressDir))
	{
		XIAO_LOG(Error, TEXT("Decompress file failed::%s"), *TempFile);
		return false;
	}
	
	// 删除XiaoInstallConsole, 避免覆盖冲突
	const FString XiaoInstallConsolePath = FPaths::Combine(DecompressDir, TEXT("Binaries"), SPlatformName, TEXT("XiaoInstallConsole"), SExeExtension);
	if (!PlatformFile.DeleteFile(*XiaoInstallConsolePath))
	{
		XIAO_LOG(Error, TEXT("DeleteFile [%s] failed!"), *XiaoInstallConsolePath);
	}
	const TSet<FString> IgonoreFiles = { TEXT("XiaoInstallConsole") };
	const FString DescDir = GetXiaoHomePath();
	if (!PlatformFile.CopyDirectoryTree(*DescDir, *DecompressDir, true))
	{
		XIAO_LOG(Error, TEXT("CopyDirectoryTree from [%s]->[%s] failed!"), *DecompressDir, *DescDir);
	}

	if (PlatformFile.DeleteDirectoryRecursively(*STempFolder))
	{
		XIAO_LOG(Error, TEXT("DeleteDirectoryRecursively [%s] failed!"), *STempFolder);
	}

	XIAO_LOG(Log, TEXT("Overrite files finish!"));
	return true;
}

bool OnUpdate()
{
	if (!Before(true))
	{
		return false;
	}

	// 更新代理状态为更新中
	{
		XIAO_LOG(Log, TEXT("Update Agent state!"));
		auto Option = XiaoRedis::SRedisClient->hget(XiaoRedis::Hash::SAgentStats, GAgentUID);
		if (!Option.has_value())
		{
			XIAO_LOG(Error, TEXT("获取缓存代理数据失败！"));
			return false;
		}

		const std::string Buf = Option.value();
		FAgentProto Proto;
		if (Buf.size() == 0 || !Proto.ParseFromString(Buf))
		{
			XIAO_LOG(Error, TEXT("获取缓存代理数据失败！"));
			return false;
		}

		Proto.set_status(3);

		std::string BufUpdate;
		if (!Proto.SerializeToString(&BufUpdate))
		{
			XIAO_LOG(Error, TEXT("更新缓存代理数据失败！"));
			return false;
		}

		XiaoRedis::SRedisClient->hset(XiaoRedis::Hash::SAgentStats, GAgentUID, BufUpdate);
	}

	// 然后覆盖数据
	if (!OverrideFiles())
	{
		return false;
	}

	// 开始运行服务程序
	FString Error;
	if(!SetServiceState(SBuildAgentService, true, Error))
	{
		UpdateError(FString::Printf(TEXT("Change [%s] service state FAILED::%s!"), *SBuildAgentService, *Error));
		return false;
	}

	// 运行Tray
	RunXiaoApp(SBuildTray);
	// SayGoodby
	return true;
}

bool UnInstall()
{
	Before(false);

	// 首先建立连接
	if (GInstallSettings.SetupUpType & IT_Unistall)
	{
		GMasterConnection.host = TCHAR_TO_UTF8(*SOriginalAgentSettings.NetworkCoordinate.IP);
		GMasterConnection.port = SOriginalAgentSettings.NetworkCoordinate.Port;
		GMasterConnection.keep_alive = true;
		XiaoRedis::TryConnectRedis();

		// 删除代理缓存数据
		if (XiaoRedis::IsConnected())
		{
			XiaoRedis::SRedisClient->hdel(XiaoRedis::Hash::SAgentStats, GAgentUID);
		}
	}
#pragma region MonitorMenu

#pragma endregion

	UnRegistEnvirment();

	// TOOD 把对应的消息发送到后台
	return false;
}