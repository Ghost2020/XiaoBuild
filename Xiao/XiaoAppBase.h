/**
  * @author cxx2020@outlook.com
  * @date 10:24 PM
 */
#pragma once

#include "CoreMinimal.h"
#include "Runtime/Launch/Resources/Version.h"
#include "Templates/UnrealTemplate.h"
#include "Async/TaskGraphInterfaces.h"
#include "Misc/MessageDialog.h"
#include "Misc/App.h"
#include "HAL/PlatformProcess.h"
#include "HAL/PlatformTime.h"
#include "HAL/PlatformApplicationMisc.h"
#include "Interfaces/IPluginManager.h"
#include "Modules/ModuleManager.h"
#include "StandaloneRenderer.h"
#include "Framework/Application/SlateApplication.h"
#include "Framework/Docking/TabManager.h"
#include "Internationalization/Culture.h"
#include "LaunchEngineLoop.h"
#include "XiaoStyle.h"
#include "XiaoAgent.h"
#include "Misc/CommandLine.h"
#include "XiaoShare.h"
#include "XiaoShareNetwork.h"
#include "XiaoShareRedis.h"
#ifdef HTTP_PACKAGE
#include "HttpModule.h"
#endif

using namespace XiaoNetwork;

#define LOCTEXT_NAMESPACE "XiaoApp"

class FXiaoAppBase : public FNoncopyable
{
	DECLARE_EVENT(FXiaoAppBase, FOnAgentSettingsChanged);
	
public:
	struct FAppParam
	{
		explicit FAppParam()
		{}

		explicit FAppParam(const FString& InAppName, const uint32 InIdealFramerate, const uint32 InNormalFramerate, const bool bInSingleton)
			: AppName(InAppName)
			, IdealFramerate(InIdealFramerate)
			, NormalFramerate(InNormalFramerate)
			, bSingleton(bInSingleton)
		{}

		FString AppName = TEXT("XiaoApp");
		uint32 IdealFramerate = 1;
		uint32 NormalFramerate = 24;
		bool bSingleton = false;
		bool bConnectRedis = true;
	};

	FXiaoAppBase(FAppParam& InParam)
		: Param(InParam)
	{}

	virtual ~FXiaoAppBase()
	{
		ShutApp();
	}

	virtual bool InitApp()
	{
		if (!CheckApp())
		{
			return false;
		}

		LoadAgentSettings(SOriginalAgentSettings);
		const auto& NetworkCoordi = SOriginalAgentSettings.NetworkCoordinate;
		GMasterConnection.host = TCHAR_TO_UTF8(*NetworkCoordi.IP);
		GMasterConnection.port = NetworkCoordi.Port;
		GMasterConnection.keep_alive = true;
		if (Param.bConnectRedis)
		{
			XiaoRedis::TryConnectRedis();
		}

#pragma region Module
		// Make sure all UObject classes are registered and default properties have been initialized
		ProcessNewlyLoadedUObjects();

		// Tell the module manager it may now process newly-loaded UObjects when new C++ modules are loaded
		FModuleManager::Get().StartProcessingNewlyLoadedObjects();

		IPluginManager::Get().LoadModulesForEnabledPlugins(ELoadingPhase::PreDefault);
		IPluginManager::Get().LoadModulesForEnabledPlugins(ELoadingPhase::Default);

		FSlateApplication::InitHighDPI(true);
		FGlobalTabmanager::Get()->SetApplicationTitle(FText::FromString(Param.AppName));

		Slate = FSlateApplication::Create(MakeShareable(FPlatformApplicationMisc::CreateApplication()));

		FXiaoStyle::Get();
		// FAppStyle::SetAppStyleSet(FXiaoStyle::Get());

		const TSharedRef<FSlateRenderer> SlateRenderer = GetStandardStandaloneRenderer();

		// Try to initialize the renderer. It's possible that we launched when the driver crashed so try a few times before giving up.
		if (!Slate->InitializeRenderer(SlateRenderer, true))
		{
			FSlateApplication::Shutdown();
			return false;
		}
#pragma endregion

#pragma region Delagate
		WillDeactivateHandle = FCoreDelegates::ApplicationWillDeactivateDelegate.AddLambda([this]()
		{
			Ideal_Framerate = Param.IdealFramerate;
			FPlatformProcess::SetThreadPriority(EThreadPriority::TPri_Lowest);
		});
		WillEnterBackgroundHandle = FCoreDelegates::ApplicationWillEnterBackgroundDelegate.AddLambda([this]()
		{
			Ideal_Framerate = Param.IdealFramerate;
			FPlatformProcess::SetThreadPriority(EThreadPriority::TPri_Lowest);
		});

		HasReactivateHandle = FCoreDelegates::ApplicationHasReactivatedDelegate.AddLambda([this]()
		{
			Ideal_Framerate = Param.NormalFramerate;
			FPlatformProcess::SetThreadPriority(EThreadPriority::TPri_Normal);
		});
		HasEnteredForegroundHandle = FCoreDelegates::ApplicationHasEnteredForegroundDelegate.AddLambda([this]()
		{
			Ideal_Framerate = Param.NormalFramerate;
			FPlatformProcess::SetThreadPriority(EThreadPriority::TPri_Normal);
		});
#pragma endregion

		GApp = this;

#pragma region Lauguage
		FString Culture = SOriginalAgentSettings.Localization;
		const FString CommandLine = FCommandLine::Get();
		{
			FParse::Value(*CommandLine, TEXT("CULTURE="), Culture);
			FParse::Value(*CommandLine, TEXT("LANGUAGE="), Culture);
			FParse::Value(*CommandLine, TEXT("LOCALE="), Culture);
		}
		if (FInternationalization::Get().GetCurrentCulture()->GetName() != Culture)
		{
			FInternationalization::Get().SetCurrentCulture(Culture);
		}
		GLocalization = Culture;
#pragma endregion

		// SetProcessDisplyName(Param.AppName);

		FPlatformMisc::SetUTF8Output();
		return true;
	}

	virtual void RunApp()
	{
		XIAO_LOG(Log, TEXT("RunApp::Begin"));
#if PLATFORM_WINDOWS
		if (Window.IsValid())
		{
			if (void* WindowHandle = Window.Pin()->GetNativeWindow()->GetOSWindowHandle())
			{
				const FString IconsFolder = FPaths::Combine(FXiaoStyle::Get().GetContentRootDir(), TEXT("Icons/Tray/Status"));
				const HICON NormalIcon = static_cast<HICON>(LoadImage(nullptr, *FString(IconsFolder + TEXT("/nornalTray.ico")), IMAGE_ICON, 0, 0, LR_LOADFROMFILE));
				SendMessageW(static_cast<HWND>(WindowHandle), WM_SETICON, ICON_BIG, (LPARAM)NormalIcon);
			}
		}
#endif

		double DeltaTime = 0.0;
		double LastTime = FPlatformTime::Seconds();

		while (!IsEngineExitRequested())
		{
			FTaskGraphInterface::Get().ProcessThreadUntilIdle(ENamedThreads::GameThread);

			Slate->PumpMessages();
			Slate->Tick();

			for (auto& delegate : NextTickDelegates)
			{
				delegate.Execute();
			}NextTickDelegates.Empty();			
			
			OnTick(DeltaTime);
			FTSTicker::GetCoreTicker().Tick(DeltaTime);
			const float IdealFrameTime = 1.0f / Ideal_Framerate;
			FPlatformProcess::Sleep(FMath::Max<float>(0.0f, IdealFrameTime - (FPlatformTime::Seconds() - LastTime)));

			const double CurrentTime = FPlatformTime::Seconds();
			DeltaTime = CurrentTime - LastTime;
			LastTime = CurrentTime;

#if (ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION <= 5) && !UE_BUILD_SHIPPING
			FStats::AdvanceFrame(false);
#endif

			FCoreDelegates::OnEndFrame.Broadcast();
			GLog->FlushThreadedLogs();

			GFrameCounter++;
		}

		if (Window.IsValid())
		{
			Window.Pin()->HideWindow();
		}
	}

	void AddNextTickTask(FSimpleDelegate&& InDeletegate)
	{
		NextTickDelegates.Add(InDeletegate);
	}

	virtual void ShutApp()
	{
		if (Window.IsValid())
		{
			Window.Pin()->RequestDestroyWindow();
			Window = nullptr;
		}

		if (WillDeactivateHandle.IsValid())
		{
			FCoreDelegates::ApplicationWillDeactivateDelegate.Remove(WillDeactivateHandle);
			WillDeactivateHandle.Reset();
		}
		if (WillEnterBackgroundHandle.IsValid())
		{
			FCoreDelegates::ApplicationWillEnterBackgroundDelegate.Remove(WillEnterBackgroundHandle);
			WillEnterBackgroundHandle.Reset();
		}
		if (HasReactivateHandle.IsValid())
		{
			FCoreDelegates::ApplicationHasReactivatedDelegate.Remove(HasReactivateHandle);
			HasReactivateHandle.Reset();
		}
		if (HasEnteredForegroundHandle.IsValid())
		{
			FCoreDelegates::ApplicationHasEnteredForegroundDelegate.Remove(HasEnteredForegroundHandle);
			HasEnteredForegroundHandle.Reset();
		}

		FCoreDelegates::OnExit.Broadcast();

		FSlateApplication::Shutdown();
		FModuleManager::Get().UnloadModulesAtShutdown();

		GEngineLoop.AppPreExit();
		GEngineLoop.AppExit();

		ReleaseAppMutex();
	}

	virtual TWeakPtr<SWindow> GetMainWindow() { return Window; }

protected:
	virtual void OnTick(const float InDeltaTime)
	{
	}

	virtual bool CheckApp()
	{
		// 是否是有效的环境
		if (!IsValidDir())
		{
			// FXiaoStyle::DoModel(LOCTEXT("NotValidDir_Text", "运行环境存在问题"));
			return false;
		}

		// 单例检查
		if (!IsSingleton())
		{
			XIAO_LOG(Warning, TEXT("Check Sington failed so force window to front!"));
			return false;
		}

		// TODO 检查许可信息
		return true;
	}

private:
	bool IsSingleton()
	{
		if (Param.bSingleton)
		{
			if (!CheckSingleton(Param.AppName))
			{
				// #FIXEME Text 只能显示在Title当中
				const FText AreayExitText = LOCTEXT("AlreayExist_Text", "The program only allows a single instance to run");
				const FText TitleText = LOCTEXT("AlreayExist_Title", "警告");
				FPlatformMisc::MessageBoxExt(EAppMsgType::Type::Ok, *TitleText.ToString(), *AreayExitText.ToString());
				BringAppToTop(GetProcId(FPlatformProcess::ExecutablePath(), FCommandLine::Get(), FPlatformProcess::GetCurrentProcessId()));
				return false;
			}
		}
		return true;
	}

protected:
	FAppParam& Param;
	FAgentSettings AgentSettings;
	TWeakPtr<class SWindow> Window = nullptr;
	TSharedPtr<FSlateApplication> Slate = nullptr;
	TArray<FSimpleDelegate> NextTickDelegates;
	FOnAgentSettingsChanged OnAgentSettingsChanged;

private:
	FDelegateHandle WillDeactivateHandle;
	FDelegateHandle WillEnterBackgroundHandle;
	FDelegateHandle HasReactivateHandle;
	FDelegateHandle HasEnteredForegroundHandle;

public:
	static inline FXiaoAppBase* GApp = nullptr;
	uint8 Ideal_Framerate = 24;
};

#undef LOCTEXT_NAMESPACE