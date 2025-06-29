/**
  * @author cxx2020@outlook.com
  * @date 11:02 PM
 */
#include "SInstallProgressView.h"
#include "SlateOptMacros.h"
#include "Widgets/Notifications/SErrorText.h"
#include "Widgets/Notifications/SProgressBar.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Images/SImage.h"
#include "ShareWidget.h"
#include "XiaoStyle.h"
#include "XiaoLog.h"
#include "XiaoInstall.h"
#include "XiaoAgent.h"
#include "XiaoShareField.h"

#define LOCTEXT_NAMESPACE "SInstallProgressView"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SInstallProgressView::Construct(const FArguments& InArgs)
{
	ChildSlot
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		[
			SAssignNew(ErrorText, SErrorText)
		]
		TOP_WIDGET

		+ SVerticalBox::Slot()
		[
			SNew(SVerticalBox)
			.Visibility_Lambda([this]() 
			{
				return bValidEnv ? EVisibility::Visible : EVisibility::Collapsed;
			})
			+ SVerticalBox::Slot().SEC_PADDING
			[
				SAssignNew(ProgressBar, SProgressBar)
				.Visibility_Lambda([this]()
				{
					return bFinish ? EVisibility::Collapsed : EVisibility::Visible;
				})
			]
			+ SVerticalBox::Slot().SEC_PADDING
			[
				SAssignNew(MessageTextBox, SEditableTextBox)
				.IsEnabled(false)
				.Visibility_Lambda([this]()
				{
					return bFinish ? EVisibility::Collapsed : EVisibility::Visible;
				})
			]
			+ SVerticalBox::Slot().SEC_PADDING.AutoHeight().HAlign(HAlign_Center)
			[
				SNew(SImage)
				.Image(FXiaoStyle::Get().GetBrush(TEXT("Install.Congradulations")))
				.Visibility_Lambda([this]()
				{
					return bFinish ? EVisibility::Visible : EVisibility::Collapsed;
				})
			]
		]
	];

	if (GInstallSettings.SetupUpType == 0)
	{
		const bool bSuccess = TryCreateIPC();
		ErrorText->SetError(bSuccess ? FText::GetEmpty() : LOCTEXT("InterprocessError_Text", "进程间通信出现异常"));
		SetCanTick(bSuccess);
	}
	else if (GInstallSettings.SetupUpType == 1)
	{
		if (!OnUpdate())
		{
			ErrorText->SetError(LOCTEXT("SyncUpdateFailed_Text", "执行同步更新失败!"));
		}
		else
		{
			RequestEngineExit(TEXT("RequestExit for sync update"));
		}
	}
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SInstallProgressView::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	UpdateTimer += InDeltaTime;
	if (UpdateTimer > 1.0f)
	{
		return;
	}

	UpdateTimer = 0.0f;
	if (!bValidEnv)
	{
		bValidEnv = CheckEnv();
		if (!bValidEnv)
		{
			return;
		}

		ProgressBar->SetPercent(0.1f);
		GProgress.Status = 0;
		FText BeginText;

		if (GInstallSettings.SetupUpType == 0 && OnInstall())
		{
			BeginText = LOCTEXT("StartInstall_Text", "开始安装！");
		}
		else if (GInstallSettings.SetupUpType == 2 && OnUninstall())
		{
			BeginText = LOCTEXT("StartUninstall_Text", "开始卸载！");
		}

		MessageTextBox->SetText(BeginText);
		UpdateMessage(0.0f, BeginText.ToString());
	}

	try
	{
		if (GProgressRegion.IsValid())
		{
			const FString Content = UTF8_TO_TCHAR(static_cast<char*>(GProgressRegion->get_address()));
			if (GProgress.FromJson(Content))
			{
				ProgressBar->SetPercent(GProgress.Progress);
				MessageTextBox->SetText(FText::FromString(GProgress.Message));
				if (GProgress.Progress > 1.0f && GProgress.Status == 1)
				{
					bFinish = true;
					SetCanTick(false);
				}
				if (GProgress.Status == -1)
				{
					MessageTextBox->SetError(FText::FromString(GProgress.Message));
					SetCanTick(false);
				}
			}
		}

		if(GProgress.Progress < 1.0f && !IsAppRunning(XiaoAppName::SInstallConsole))
		{
			ErrorText->SetError(LOCTEXT("NotRunning_Text", "Install Console is not running!"));
		}
	}
	catch (interprocess_exception& Ex)
	{
		const FString Erorr = FString::Printf(TEXT("Interprocee Object Create Exception::%s!"), UTF8_TO_TCHAR(Ex.what()));
		ErrorText->SetError(FText::FromString(Erorr));
		XIAO_LOG(Error, TEXT("%s"), *Erorr);
		return;
	}
}

bool SInstallProgressView::OnCanBack()
{
	return false;
}

bool SInstallProgressView::OnCanNext()
{
	return bFinish;
}

FText SInstallProgressView::GetNextTitle()
{
	return FText::FromString(ErrorText->HasError() ? TEXT("Exit"): TEXT("Finish"));
}

bool SInstallProgressView::IsFinal()
{
	return true;
}

bool SInstallProgressView::OnCanExit()
{
	return ErrorText->HasError();
}

void SInstallProgressView::OnFinish()
{
	XIAO_LOG(Log, TEXT("SInstallProgressView::OnFinish::Begin"));
	if (GInstallSettings.SetupUpType == EInstallType::IT_Install)
	{
		RunXiaoApp(XiaoAppName::SBuildTray, TEXT(""), false, true, true, true);
	}

	XIAO_LOG(Log, TEXT("SInstallProgressView::OnFinish::Finish"));

	RequestEngineExit(TEXT("Install Finish"));
}

bool SInstallProgressView::OnInstall() const
{
	XIAO_LOG(Log, TEXT("Execute Intall Begin"));

	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();

	FAgentSettings AgentSettings;

	AgentSettings.Localization = GInstallSettings.Localization;
	AgentSettings.NetworkGeneral.PrimaryPort = GInstallSettings.AgentListenPort;
	AgentSettings.UbaAgent.ListenPort = GInstallSettings.AgentListenPort;
	AgentSettings.UbaScheduler.Port = GInstallSettings.SchedulerServerPort;
	AgentSettings.NetworkCoordinate.IP = GInstallSettings.CoordiIp;
	AgentSettings.NetworkCoordinate.Port = GInstallSettings.CoordiPort;
	AgentSettings.UbaAgent.Dir = FPaths::ConvertRelativePathToFull(FPaths::Combine(GInstallSettings.CacheFolder, XiaoAppName::SUbaAgent));
	AgentSettings.UbaScheduler.Dir = FPaths::ConvertRelativePathToFull(FPaths::Combine(GInstallSettings.CacheFolder, XiaoAppName::SXiaoScheduler));
	SaveAgentSettings(AgentSettings);

	const FString SaveInstallSettingPath = FPaths::ConvertRelativePathToFull(FPaths::Combine(
#if PLATFORM_MAC
		TEXT("/Library/Application Support/XiaoBuild")
#else
		FPaths::GetPath(FPlatformProcess::ApplicationSettingsDir())
#endif
		, TEXT("install_setting.json"))
	);
	if(!FFileHelper::SaveStringToFile(GInstallSettings.ToJson(true), *SaveInstallSettingPath, FFileHelper::EEncodingOptions::AutoDetect, &IFileManager::Get(), FILEWRITE_EvenIfReadOnly))
	{
		const FText ErrorMessage = LOCTEXT("SaveString_Text", "保存安装参数文件失败");
		ErrorText->SetError(ErrorMessage);
		XIAO_LOG(Error, TEXT("Execute Intall Failed"));
		return false;
	}
	
	const FString Params = FString::Printf(TEXT("-install -install_setting=\"%s\" -ppid=%d -LOG=%s.log"), *SaveInstallSettingPath, FPlatformProcess::GetCurrentProcessId(), *XiaoAppName::SInstallConsole);
	RunXiaoApp(XiaoAppName::SInstallConsole, Params);
	XIAO_LOG(Log, TEXT("Execute Intall Begin!"));
	return true;
}

bool SInstallProgressView::OnUpdate()
{
	RunXiaoApp(XiaoAppName::SInstallConsole, FString::Printf(TEXT("-update -ppid=%d -LOG=%s.log"), FPlatformProcess::GetCurrentProcessId(), *XiaoAppName::SInstallConsole));
	XIAO_LOG(Log, TEXT("Execute Update Finish"));
	return true;
}

bool SInstallProgressView::OnUninstall()
{
	RunXiaoApp(XiaoAppName::SInstallConsole, FString::Printf(TEXT("-uninstall -ppid=%d -LOG=%s.log"), FPlatformProcess::GetCurrentProcessId(), *XiaoAppName::SInstallConsole));
	XIAO_LOG(Log, TEXT("Execute Update Finish"));
	return true;
}

bool SInstallProgressView::CheckEnv()
{
	FString Error;
	const bool Rtn = CheckEnvRuning(Error, FPlatformProcess::GetCurrentProcessId());
	ErrorText->SetError(Error);
	return Rtn;
}

#undef LOCTEXT_NAMESPACE