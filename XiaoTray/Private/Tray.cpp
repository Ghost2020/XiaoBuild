/**
  * @author cxx2020@outlook.com
  * @date 10:49 AM
 */

#include "Tray.h"
#include "RequiredProgramMainCPPInclude.h"
#include "XiaoShare.h"
#include "XiaoShareField.h"
#include "XiaoInterprocess.h"
#include "XiaoAgent.h"
#include "XiaoShareRedis.h"
#include "XiaoCompressor.h"
#include "Misc/CommandLine.h"
#include "Version.h"

#ifdef verify
#pragma push_macro("verify")
#undef verify
#endif

#ifdef check
#pragma push_macro("check")
#undef check
#endif

THIRD_PARTY_INCLUDES_START
#include "QtWidgets/qapplication.h"
#include "QtWidgets/qsystemtrayicon.h"
#include "QtWidgets/qmenu.h"
#include "QtWidgets/qaction.h"
#include "QtWidgets/qmessagebox.h"
#include "QtCore/qtimer.h"
THIRD_PARTY_INCLUDES_END

#include <map>

using namespace XiaoIPC;

static FSystemSettings SSystemSettings;
static std::string GAgentUniqueId;

static const QString SEnglish("en");
static const QString SChinese("zh-CN");

static const QString SSyncUpdate("Sync update");
static const QString SSyncUpdate_T("The latest version has been detected. Please update to the latest version as soon as possible.");

static const QString SCheckUpdate("Check update");
static const QString SCheckUpdate_T("Check if there is the latest version");

static const QString SDocument("Document");
static const QString SDocument_T("Software manual, etc.");

static const QString SAbout("About");
static const QString SAbout_T("System version information");

static const QString SLog("Logs");
static const QString SLog_T("Browse the log directory to track the operation status");

static const QString SClearCache("Clear cache");
static const QString SClearCache_T("When the disk space is insufficient, you can clean up the system cache data to free up space.");

static const QString SClearSystemCache("System cache");
static const QString SClearSystemCache_T("Clean up system temporary files");

static const QString SAgentCache("Agent cache");
static const QString SAgentCache_T("As the agent service runs, a lot of cache data is generated");

static const QString SSchedulerCache("Scheduler cache");
static const QString SSchedulerCache_T("As the scheduler runs, a lot of cache data is generated");

static const QString SBuildHistory("Build History");
static const QString SBuildHistory_T("View recent build history");

static const QString SNetworkTest("Network Connecti.");
static const QString SNetworkTest_T("Open the network test panel to view the test network status");

static const QString SCoordinator("Open Coordinator");
static const QString SCoordinator_T("Open the scheduling management panel and set up the proxy computing resources in the deployment system");

static const QString SAgentSettings("Agent Settings");
static const QString SAgentSettings_T("Open the agent settings panel to set");

static const QString SExit("Exit");
static const QString SExit_T("Want Exit tray?");

static const QString SConfirm("Comfirm");
static const QString SNotShutdownSystem("Do you want to exit the tray program? (Exiting this program will not stop the joint compilation service)");

static const QString SAgentNetworking("The agent service program is not running, please open the agent settings to set");
static const QString SAgentDisable("Currently disabled");

static const std::map<QString, std::map<QString, QString>> SLocalizationMap =
{
	{
		SEnglish,
		{

		}
	},
	{
		SChinese,
		{
			{ SSyncUpdate,   "同步更新"},
			{ SSyncUpdate_T, "检测到有最新版本，请尽快更新到最新版本"},

			{ SCheckUpdate, "检查更新" },
			{ SCheckUpdate_T, "检测是否有最新的版本"},

			{ SDocument, "文档" },
			{ SDocument_T, "软件使用手册等介绍" },

			{ SAbout, "关于" },
			{ SAbout_T, "系统版本信息" },

			{ SLog, "日志目录" },
			{ SLog_T, "浏览日志目录追踪运行状况" },

			{ SClearCache, "缓存清理" },
			{ SClearCache_T, "磁盘空间不够时，可清理系统缓存数据以释放空间" },

			{ SClearSystemCache, "系统临时文件" },
			{ SClearSystemCache_T, "清理系统临时文件" },

			{ SAgentCache, "清理代理Cas" },
			{ SAgentCache_T, "随着代理服务的运行，会产生许多缓存数据" },

			{ SSchedulerCache, "清理调度器Cas" },
			{ SSchedulerCache_T, "随着调度器的运行，会产生许多缓存数据" },

			{ SBuildHistory, "构建历史" },
			{ SBuildHistory_T, "查看最近的构建历史" },

			{ SNetworkTest, "网络测试" },
			{ SNetworkTest_T, "打开网络测试面板，查看测试网络状况" },

			{ SCoordinator, "调度管理" },
			{ SCoordinator_T, "打开调度管理面板,设置调配系统中代理计算资源" },

			{ SAgentSettings, "代理设置" },
			{ SAgentSettings_T, "打开代理设置面板进行设置" },

			{ SExit, "退出" },
			{ SExit_T, "退出程序" },

			{ SConfirm, "确认" },
			{ SNotShutdownSystem, "是否退出托盘程序？(退出此程序并不会停止联合编译服务)" },

			{ SAgentNetworking, "代理服务程序未运行，请打开代理设置进行设置"},
			{ SAgentDisable, "当前处于禁用状态中"}
		}
	}
};

IMPLEMENT_APPLICATION(XiaoTray, XB_PRODUCT_NAME);

#if !PLATFORM_MAC
static void BeforeExit()
{
	ReleaseAppMutex();
	FEngineLoop::AppExit();
}
#endif

static QString GetIconDir()
{
	FString IconDir = FPaths::ConvertRelativePathToFull(FPaths::Combine(GetEngineBinariesDir(), TEXT("../../Content/Slate/XiaoBuild/Icons/Tray")));
#if PLATFORM_MAC
	if(!FPaths::DirectoryExists(IconDir))
	{
		IconDir = FPaths::ConvertRelativePathToFull(FPaths::Combine(FPaths::RootDir(), TEXT("../UE/Engine/Content/Slate/XiaoBuild/Icons/Tray")));
		if(!FPaths::DirectoryExists(IconDir))
		{
			XIAO_LOG(Error, TEXT("IconDir not valid::%s"), *IconDir);
		}
	}
#endif
	return TCHAR_TO_UTF8(*IconDir);
}


FXiaoTray::FXiaoTray()
{
	Permissions.set_unrestricted();
}

FXiaoTray::~FXiaoTray()
{
	if (ProgressShm)
	{
		delete ProgressShm;
	}
	if (ProgressRegion)
	{
		delete ProgressRegion;
	}
}

bool FXiaoTray::InitApp()
{
	GEngineLoop.PreInit(0, nullptr, *FString::Printf(TEXT("-LOG=XiaoTray.log")));

#if !PLATFORM_MAC
	if (!CheckSingleton(TEXT("XiaoTray")))
	{
		qWarning("Check Singleton failed !");
		return false;
	}

	atexit(BeforeExit);
#endif

	FTaskTagScope Scope(ETaskTag::EGameThread);

	const FString CmdLine = FCommandLine::BuildFromArgV(nullptr, 0, nullptr, TEXT(""));
	FCommandLine::Set(*CmdLine);

	LoadAgentSettings(SOriginalAgentSettings);
	const auto& NetworkCoordi = SOriginalAgentSettings.NetworkCoordinate;
	GMasterConnection.host = TCHAR_TO_UTF8(*NetworkCoordi.IP);
	GMasterConnection.port = NetworkCoordi.Port;
	GMasterConnection.keep_alive = true;
	XiaoRedis::TryConnectRedis();

	try
	{
		ProgressShm = new shared_memory_object(open_or_create, SMonitorProgressMemoryName.c_str(), read_write, XiaoIPC::Permissions);
		if (!ProgressShm)
		{
			XIAO_LOG(Error, TEXT("Cant OpenExisting IPC Progress Memory!"));
			return false;
		}
		ProgressShm->truncate(SMonitorProgressMemorySize);
		ProgressRegion = new mapped_region(*ProgressShm, read_only);
		if (!ProgressRegion)
		{
			XIAO_LOG(Error, TEXT("Cant Create Progress Map Region!"));
			return false;
		}
	}
	catch (interprocess_exception& Ex)
	{
		XIAO_LOG(Error, TEXT("Interprocee Object Create Exception::%s!"), UTF8_TO_TCHAR(Ex.what()));
		return false;
	}

	GAgentUniqueId = TCHAR_TO_UTF8(*GetUniqueDeviceID());
	return true;
}

void FXiaoTray::RunApp()
{
	int argc = 0;
	char** argv = nullptr;
	QApplication App(argc, argv);

	// 检查系统是否支持托盘图标
	if (!QSystemTrayIcon::isSystemTrayAvailable())
	{
		qWarning("System Tray is not available！");
		return;
	}

	// 设置托盘图标
	QSystemTrayIcon TrayIcon;
	const QString IconDir = GetIconDir();
	TrayIcon.setIcon(QIcon(IconDir + "/xiao_cloud.png"));
	TrayIcon.setToolTip("XiaoBuildTray");

	// 创建右键菜单
	QMenu TrayMenu;

	SyncAction = new QAction(&App);
	SyncAction->setIcon(QIcon(IconDir + "/xiao_cloud.png"));
	SyncAction->setVisible(false);
	QObject::connect(SyncAction, &QAction::triggered, []()
		{
			RunXiaoApp(XiaoAppName::SBuildApp, FString::Printf(TEXT("-app=%s -sync_update"), *XiaoAppName::SBuildInstall));
		}
	);
	TrayMenu.addAction(SyncAction);

	DetectAction = new QAction(&App);
	DetectAction->setIcon(QIcon(IconDir + "/update.png"));
	DetectAction->setVisible(false);
	QObject::connect(DetectAction, &QAction::triggered, [this]()
		{
			(void)OnCheckUpdate();
		}
	);
	TrayMenu.addAction(DetectAction);

	DocumentAction = new QAction(&App);
	DocumentAction->setIcon(QIcon(IconDir + "/github-mark-white.png"));
	QObject::connect(DocumentAction, &QAction::triggered, []()
		{
			FString Error;
			FPlatformProcess::LaunchURL(*XiaoUrl::SXiaoBuildWeb, TEXT(""), &Error);
		}
	);
	TrayMenu.addAction(DocumentAction);

	AboutAction = new QAction(&App);
	AboutAction->setIcon(QIcon(IconDir + "/about.png"));
	QObject::connect(AboutAction, &QAction::triggered, []()
		{
			RunXiaoApp(XiaoAppName::SBuildApp, FString::Printf(TEXT("-app=%s"), *XiaoAppName::SBuildAbout), false, true, false, true, true);
		}
	);
	TrayMenu.addAction(AboutAction);

	TrayMenu.addSeparator();

	LogAction = new QAction(&App);
	LogAction->setIcon(QIcon(IconDir + "/log.png"));
	QObject::connect(LogAction, &QAction::triggered, []()
		{
			const FString Dir = FPaths::Combine(
#if PLATFORM_WINDOWS
				FPlatformProcess::UserDir(), TEXT("../AppData/Local/XiaoApp/Saved/Logs"));
#elif PLATFORM_MAC
				FPlatformProcess::UserHomeDir(), TEXT("Library/Logs/Unreal Engine"));
#elif PLATFORM_UNIX
				FPlatformProcess::UserHomeDir(), TEXT(".log/"));
#endif
			FPlatformProcess::ExploreFolder(*Dir);
		}
	);
	TrayMenu.addAction(LogAction);

	ClearMenu = new QMenu();
	ClearMenu->setIcon(QIcon(IconDir + "/cache.png"));

	ClearTempAction = new QAction(ClearMenu);
	QObject::connect(ClearTempAction, &QAction::triggered, []()
		{
			OnDeleteFolderFiles(FPlatformProcess::UserTempDir());
		}
	);
	ClearMenu->addAction(ClearTempAction);

	ClearAgentAction = new QAction(ClearMenu);
	QObject::connect(ClearAgentAction, &QAction::triggered, []()
		{
			FPlatformProcess::ExploreFolder(*SModifiedAgentSettings.UbaAgent.Dir);
		}
	);
	ClearMenu->addAction(ClearAgentAction);

	ClearSchedulerAction = new QAction(ClearMenu);
	QObject::connect(ClearSchedulerAction, &QAction::triggered, []()
		{
			FPlatformProcess::ExploreFolder(*SModifiedAgentSettings.UbaScheduler.Dir);
		}
	);
	ClearMenu->addAction(ClearSchedulerAction);

	TrayMenu.addMenu(ClearMenu);

	TrayMenu.addSeparator();

	HistoryAction = new QAction(&App);
	HistoryAction->setIcon(QIcon(IconDir + "/history.png"));
	QObject::connect(HistoryAction, &QAction::triggered, []()
		{
			RunXiaoApp(XiaoAppName::SBuildApp, FString::Printf(TEXT("-app=%s -history "), *XiaoAppName::SBuildMonitor), false, true, false, true, true);
		}
	);
	TrayMenu.addAction(HistoryAction);

	NetworkTestAction = new QAction(&App);
	NetworkTestAction->setIcon(QIcon(IconDir + "/network.png"));
	QObject::connect(NetworkTestAction, &QAction::triggered, []()
		{
			RunXiaoApp(XiaoAppName::SBuildApp, FString::Printf(TEXT("-app=%s -network "), *XiaoAppName::SBuildMonitor), true, true,
#if PLATFORM_WINDOWS
				true,
#else
				false,
#endif
				true, true);
		}
	);
	TrayMenu.addAction(NetworkTestAction);

	CoordiManagerAction = new QAction(&App);
	CoordiManagerAction->setIcon(QIcon(IconDir + "/coordinator.png"));
	QObject::connect(CoordiManagerAction, &QAction::triggered, []()
		{
			RunXiaoApp(XiaoAppName::SBuildApp, FString::Printf(TEXT("-app=%s"), *XiaoAppName::SBuildCoordiManager), false, true, false, true, true);
		}
	);
	TrayMenu.addAction(CoordiManagerAction);

	AgentSettingsAction = new QAction(&App);
	AgentSettingsAction->setIcon(QIcon(IconDir + "/settings.png"));
	QObject::connect(AgentSettingsAction, &QAction::triggered, []()
		{
			RunXiaoApp(XiaoAppName::SBuildApp, FString::Printf(TEXT("-app=%s"), *XiaoAppName::SBuildAgentSettings), true, true,
#if PLATFORM_WINDOWS
				true,
#else
				false,
#endif
				true, true);
		}
	);
	TrayMenu.addAction(AgentSettingsAction);

	TrayMenu.addSeparator();

	ExitAction = new QAction(&App);
	ExitAction->setIcon(QIcon(IconDir + "/exit.png"));
	QObject::connect(ExitAction, &QAction::triggered, [&, this]()
		{
			const auto& Map = SLocalizationMap.at(QString(*Localization));
			const QString ComfirmStr = Map.contains(SConfirm) ? Map.at(SConfirm) : SConfirm;
			const QString ComfirmMessageStr = Map.contains(SNotShutdownSystem) ? Map.at(SNotShutdownSystem) : SNotShutdownSystem;
			auto Reply = QMessageBox::question(&TrayMenu, ComfirmStr, ComfirmMessageStr);
			if (Reply == QMessageBox::Yes)
			{
				App.quit();
				RequestEngineExit(TEXT("RequestExit"));
			}
		}
	);
	TrayMenu.addAction(ExitAction);

	// 将菜单绑定到托盘图标
	TrayIcon.setContextMenu(&TrayMenu);

	QObject::connect(&TrayIcon, &QSystemTrayIcon::activated, [&](QSystemTrayIcon::ActivationReason InReason)
		{
			if (InReason == QSystemTrayIcon::ActivationReason::Context)
			{
				SyncAction->setVisible(bCanSyncUpdate);

				if (LoadAgentSettings(SModifiedAgentSettings, true))
				{
					OnUpdateLocalization(SModifiedAgentSettings.Localization);
				}

				ClearAgentAction->setEnabled(!IsAppRunning(XiaoAppName::SUbaAgent));
				ClearSchedulerAction->setEnabled(!IsAppRunning(XiaoAppName::SXiaoScheduler));
			}
			else if (InReason == QSystemTrayIcon::ActivationReason::DoubleClick)
			{
				RunXiaoApp(XiaoAppName::SBuildApp, FString::Printf(TEXT("-app=%s -host=\"127.0.0.1\""), *XiaoAppName::SBuildMonitor));
			}
			else
			{
				XIAO_LOG(Log, TEXT("InReason::%d"), InReason);
			}
		}
	);

	QTimer Timer(&App);
	QObject::connect(&Timer, &QTimer::timeout, [this, &TrayIcon]() { OnUpdate(&TrayIcon); });
	Timer.start(1000);

	// 显示托盘图标
	TrayIcon.show();

	App.exec();
}

void FXiaoTray::OnChangeState(const int State, const float Progress, QSystemTrayIcon* InTrayIcon) const
{
	QString ToolTipMessage = "XiaoBuildTray-ready";
	const auto& Map = SLocalizationMap.at(QString(*Localization));
	const QString IconDir = GetIconDir();
	QString IconPath;
	if (!bAgentServiceState)
	{
		IconPath = IconDir + "/Status/NotWorking.png";
		ToolTipMessage = Map.contains(SAgentNetworking) ? Map.at(SAgentNetworking) : SAgentNetworking;
	}
	else
	{
		if (SModifiedAgentSettings.bEnableUbac)
		{
			if (State == -1)
			{
				IconPath = IconDir + "/Status/Failed.png";
			}
			else if (State == 0)
			{
				IconPath = IconDir + "/Status/Normal.png";
			}
			else if (State == 1)
			{
				IconPath = IconDir + "/Status/Working.png";
				ToolTipMessage = QString(TCHAR_TO_UTF8(*FString::Printf(TEXT("%s \nProgress::%.1f%%"), *XiaoAppName::SBuildTray, Progress * 100.0f)));
			}
		}
		else
		{
			IconPath = IconDir + "/Status/Disable.png";
			ToolTipMessage = Map.contains(SAgentDisable) ? Map.at(SAgentDisable) : SAgentNetworking;
		}
	}

	InTrayIcon->setToolTip(ToolTipMessage);
	InTrayIcon->setIcon(QIcon(IconPath));
}

void FXiaoTray::OnUpdate(QSystemTrayIcon* InTrayIcon)
{
	if (IsAppRunning(XiaoAppName::SXiaoScheduler))
	{
		OnPullMessage(InTrayIcon);
	}

	static constexpr float SUpdateNeedTime = 20.0f;
	static double LastCheck = 0.0f;
	if (FPlatformTime::Seconds() - LastCheck > SUpdateNeedTime)
	{
		LastCheck = FPlatformTime::Seconds();
		bAgentServiceState = IsAppRunning(XiaoAppName::SBuildAgentService);
		OnChangeState(0, 0.0f, InTrayIcon);
		
		if (!XiaoRedis::IsConnected())
		{
			LoadAgentSettings(SModifiedAgentSettings);
			const auto& NetworkCoordi = SModifiedAgentSettings.NetworkCoordinate;
			GMasterConnection.host = TCHAR_TO_UTF8(*NetworkCoordi.IP);
			GMasterConnection.port = NetworkCoordi.Port;
			XiaoRedis::AsyncReconnectRedis();
			return;
		}
		if (XiaoRedis::IsConnected())
		{
			try
			{
				// 获取系统设置
				{
					const auto Option = XiaoRedis::SRedisClient->get(XiaoRedis::String::SSystemSettings);
					if (Option.has_value())
					{
						const std::string Protobuf = Option.value();
						SSystemSettings.ParseFromString(Protobuf);
					}
				}

				// 更新代理状态
				if (!bAgentServiceState)
				{
					XiaoRedis::UpdateAgent(GAgentUniqueId, Status_Offline, "Offline");
				}

				// 检查是否有更新
				(void)OnCheckUpdate();
			}
			CATCH_REDIS_EXCEPTRION();
		}
	}
}

void FXiaoTray::OnPullMessage(QSystemTrayIcon* InTrayIcon)
{	
	if (ProgressRegion)
	{
		const std::string Content(static_cast<char*>(ProgressRegion->get_address()));
		if(!Content.empty() && ProgressStatus.ParseFromString(Content))
		{
			OnChangeState(ProgressStatus.status(), ProgressStatus.progress(), InTrayIcon);
		}
	}
}

bool FXiaoTray::OnCheckUpdate() const
{
	static double SLastCheckTime = FPlatformTime::Seconds();
	static constexpr double SSyncTime = 21600;
	if ((FPlatformTime::Seconds() - SLastCheckTime) > SSyncTime)
	{
		SLastCheckTime = FPlatformTime::Seconds();
		if (XiaoRedis::IsConnected())
		{
			std::string Data;
			// bCanSyncUpdate = GetCanSyncUpdate(Data);
		}
	}

	return bCanSyncUpdate;
}

void FXiaoTray::OnUpdateLocalization(const FString& InLocalization)
{
	static bool bFirst = true;
	if (InLocalization != Localization || bFirst)
	{
		const auto& Map = SLocalizationMap.at(QString(*InLocalization));

		SyncAction->setText(Map.contains(SSyncUpdate) ? Map.at(SSyncUpdate) : SSyncUpdate);

		DocumentAction->setText(Map.contains(SDocument) ? Map.at(SDocument) : SDocument);
		DocumentAction->setToolTip(Map.contains(SDocument_T) ? Map.at(SDocument_T) : SDocument_T);

		AboutAction->setText(Map.contains(SAbout) ? Map.at(SAbout) : SAbout);
		AboutAction->setToolTip(Map.contains(SAbout_T) ? Map.at(SAbout_T) : SAbout_T);

		LogAction->setText(Map.contains(SLog) ? Map.at(SLog) : SLog);
		LogAction->setToolTip(Map.contains(SLog_T) ? Map.at(SLog_T) : SLog_T);

		ClearMenu->setTitle(Map.contains(SClearCache) ? Map.at(SClearCache) : SClearCache);
		ClearMenu->setToolTip(Map.contains(SClearCache_T) ? Map.at(SClearCache_T) : SClearCache_T);

		ClearTempAction->setText(Map.contains(SClearSystemCache) ? Map.at(SClearSystemCache) : SClearSystemCache);
		ClearTempAction->setToolTip(Map.contains(SClearSystemCache_T) ? Map.at(SClearSystemCache_T) : SClearSystemCache_T);

		ClearAgentAction->setText(Map.contains(SAgentCache) ? Map.at(SAgentCache) : SAgentCache);
		ClearAgentAction->setToolTip(Map.contains(SAgentCache_T) ? Map.at(SAgentCache_T) : SAgentCache_T);

		ClearSchedulerAction->setText(Map.contains(SSchedulerCache) ? Map.at(SSchedulerCache) : SSchedulerCache);
		ClearSchedulerAction->setToolTip(Map.contains(SSchedulerCache_T) ? Map.at(SSchedulerCache_T) : SSchedulerCache_T);

		HistoryAction->setText(Map.contains(SBuildHistory) ? Map.at(SBuildHistory) : SBuildHistory);
		HistoryAction->setToolTip(Map.contains(SBuildHistory_T) ? Map.at(SBuildHistory_T) : SBuildHistory_T);

		NetworkTestAction->setText(Map.contains(SNetworkTest) ? Map.at(SNetworkTest) : SNetworkTest);
		NetworkTestAction->setToolTip(Map.contains(SNetworkTest_T) ? Map.at(SNetworkTest_T) : SNetworkTest_T);

		CoordiManagerAction->setText(Map.contains(SCoordinator) ? Map.at(SCoordinator) : SCoordinator);
		CoordiManagerAction->setToolTip(Map.contains(SCoordinator_T) ? Map.at(SCoordinator_T) : SCoordinator_T);

		AgentSettingsAction->setText(Map.contains(SAgentSettings) ? Map.at(SAgentSettings) : SAgentSettings);
		AgentSettingsAction->setToolTip(Map.contains(SAgentSettings_T) ? Map.at(SAgentSettings_T) : SAgentSettings_T);

		ExitAction->setText(Map.contains(SExit) ? Map.at(SExit) : SExit);
		ExitAction->setToolTip(Map.contains(SExit_T) ? Map.at(SExit_T) : SExit_T);

		Localization = InLocalization;
	}

	bFirst = false;
}

void FXiaoTray::OnDeleteFolderFiles(const FString& InFolder)
{
	if (FPaths::DirectoryExists(InFolder))
	{
		IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
		TArray<FString> Files;
		PlatformFile.FindFilesRecursively(Files, *InFolder, nullptr);

		for(const FString& FilePath : Files)
		{
			if(!PlatformFile.IsReadOnly(*FilePath))
			{
				if (PlatformFile.DeleteFile(*FilePath))
				{
					XIAO_LOG(Log, TEXT("DeleteFile::%s"), *FilePath);
				}
			}
		}
	}
}

void FXiaoTray::OnCleanTempFolder()
{
	static const FString TempFolder = FPlatformProcess::UserTempDir();
	OnDeleteFolderFiles(TempFolder);
}

#pragma pop_macro("check")
#pragma pop_macro("verify")