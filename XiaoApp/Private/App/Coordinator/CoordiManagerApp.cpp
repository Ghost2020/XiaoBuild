/**
  * @author cxx2020@outlook.com
  * @date 10:49 AM
 */
#include "CoordiManagerApp.h"
#include "SLoginWindow.h"
#include "SCoordinatorWindow.h"
#include "Dialogs/SLicenseLockedDialog.h"
#include "XiaoShare.h"
#include "XiaoShareNetwork.h"
#include "XiaoShareRedis.h"
#include "ShareDefine.h"
#include "Database/Users.h"


#define LOCTEXT_NAMESPACE "CoordiManagerApp"

using namespace XiaoRedis;


FCoordiManagerApp::FCoordiManagerApp(FAppParam& InParam)
	: FXiaoAppBase(InParam)
{
}

bool FCoordiManagerApp::InitApp()
{
	if (!FXiaoAppBase::InitApp())
	{
		return false;
	}

	XIAO_LOG(Log, TEXT("FCoordiManagerApp::InitApp::Begin"));
	GLog->Flush();

#ifdef USE_IMGUI
	if (FModuleManager::Get().ModuleExists(TEXT("ImGui")))
	{
		FModuleManager::Get().LoadModule("ImGui");
	}
#endif

	if (!CheckCertificate())
	{
		ShowLoginWindow();
	}
	else
	{
		ShowMainWindow();
	}

	InitGlobalData();
	XIAO_LOG(Log, TEXT("FCoordiManagerApp::InitApp::Finish"));
	GLog->Flush();
	return true;
}

void FCoordiManagerApp::ShutApp()
{
	if (MainWindow.IsValid())
	{
		MainWindow.Pin()->DestroyWindowImmediately();
		MainWindow = nullptr;
	}

	if (LoginWindow.IsValid())
	{
		LoginWindow.Pin()->DestroyWindowImmediately();
		LoginWindow = nullptr;
	}

	FXiaoAppBase::ShutApp();
}

bool FCoordiManagerApp::CheckCertificate()
{
	if (!IsConnected())
	{
		return false;
	}

	// 使用缓存登录
	try
	{
		const auto Option = SRedisClient->hget(Hash::SLoginCache, GAgentUID);
		if (Option.has_value())
		{
			if (!Option.value().empty())
			{
				FLoginCache LoginCache;
				if (LoginCache.FromJson(UTF8_TO_TCHAR(Option.value().c_str())))
				{
					if (GCurrentUser.FromJson(LoginCache.AuthData))
					{
						return true;
					}
				}
			}
		}
	}
	CATCH_REDIS_EXCEPTRION()

	FString Content;
	FString DecryptStr;
	if (!FPaths::FileExists(SCertificateBufferFile) || !FFileHelper::LoadFileToString(Content, *SCertificateBufferFile) || !DecryptString(Content, XiaoEncryptKey::SAuth, DecryptStr))
	{
		return false;
	}

	FDateTime LastLoginTime;
	if (!GCurrentUser.FromJson(DecryptStr))
	{
		return false;
	}

	const int32 PassedMinite = (FDateTime::Now() - LastLoginTime).GetMinutes();
	if (PassedMinite >= 60 || !LoadAgentSettings(GAgentSettings))
	{
		return false;
	}

	return false;
}

void FCoordiManagerApp::ShowMainWindow()
{
	if (!MainWindow.IsValid())
	{
		const auto TempWindow = SNew(SCoordinatorWindow);
		Window = TempWindow;
		MainWindow = TempWindow;
		FSlateApplication::Get().AddWindow(TempWindow, true);
	}
	if (Window.IsValid())
	{
		Window.Pin()->SetCanTick(true);
		Window.Pin()->BringToFront();
	}

#if PLATFORM_WINDOWS
	constexpr int32 HighPriority = REALTIME_PRIORITY_CLASS;
	SetCurrentProcessPriority(HighPriority);
#endif
}

void FCoordiManagerApp::ShowLoginWindow()
{
	if (!LoginWindow.IsValid())
	{
		const auto TempWindow = SNew(SLoginWindow);
		Window = TempWindow;
		LoginWindow = TempWindow;
		FSlateApplication::Get().AddWindow(TempWindow, true);
	}
	if (MainWindow.IsValid())
	{
		MainWindow.Pin()->DestroyWindowImmediately();
		MainWindow = nullptr;
	}
	if (Window.IsValid())
	{
		Window.Pin()->BringToFront();
	}

#if PLATFORM_WINDOWS
	constexpr int32 HighPriority = NORMAL_PRIORITY_CLASS;
	SetCurrentProcessPriority(HighPriority);
#endif
}

void FCoordiManagerApp::ShowLicenseLockedWindow()
{
	if(MainWindow.IsValid())
	{
		MainWindow.Pin()->SetLockedState(true);
		AddNextTickTask(FSimpleDelegate::CreateLambda([this]()
		{
			const auto LicenseWindow = SNew(SLicenseLockedDialog);
			FSlateApplication::Get().AddModalWindow(LicenseWindow, MainWindow.Pin(), false);
			LicenseWindow->GetOnWindowClosedEvent().AddLambda([this](const TSharedRef<SWindow>&)
			{
				MainWindow.Pin()->SetLockedState(false);
			});
		}));
	}
	else
	{
		XIAO_LOG(Error, TEXT("Cant get Main Window!"));
	}
}

void FCoordiManagerApp::InitGlobalData() const
{
	if (GGroupArray.Num() == 0 )
	{
		GGroupArray.Add(MakeShared<FString>("Default"));
	}
	
	GPriorityArray.Add(MakeShared<FText>(GPriorityIdle));
	GPriorityArray.Add(MakeShared<FText>(GPriorityBlowNormal));
	GPriorityArray.Add(MakeShared<FText>(GPriorityNormal));
	GPriorityArray.Add(MakeShared<FText>(GPriorityAboveNormal));
	GPriorityArray.Add(MakeShared<FText>(GPriorityHigh));
	GPriorityArray.Add(MakeShared<FText>(GPriorityRealtime));
	GPriorityArray.Add(MakeShared<FText>(GMultiValue));

	GLevelArray.Add(MakeShared<FText>(GLogLevelIdle));
	GLevelArray.Add(MakeShared<FText>(GLogLevelBasic));
	GLevelArray.Add(MakeShared<FText>(GLogLevelIntermeidate));
	GLevelArray.Add(MakeShared<FText>(GLogLevelExtended));
	GLevelArray.Add(MakeShared<FText>(GLogLevelDetailed));
	GLevelArray.Add(MakeShared<FText>(GMultiValue));

	GRoleArray.Add(MakeShared<FText>(GLogRoleGridAmin));
	GRoleArray.Add(MakeShared<FText>(GLogRoleGroupManager));
	GRoleArray.Add(MakeShared<FText>(GLogRoleViewer));

	GMasterSlaveArray.Add(MakeShared<FText>(GMaster));
	GMasterSlaveArray.Add(MakeShared<FText>(GSlave));

	GStatusArray.Add(MakeShared<FText>(GActiveText));
	GStatusArray.Add(MakeShared<FText>(GInActiveText));
}

#undef LOCTEXT_NAMESPACE
