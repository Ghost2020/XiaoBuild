#include "SCoordinatorWindow.h"
#include "SlateOptMacros.h"
#include "Misc/Optional.h"
#include "Async/TaskGraphInterfaces.h"
#include "Widgets/Layout/SWidgetSwitcher.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Notifications/SErrorText.h"
#include "Widgets/Notifications/SNotificationList.h"

#include "Views/SStatisticsView.h"
#include "Views/SAgentView.h"
#include "Views/SUsersView.h"
#include "Views/SLogsView.h"
#include "Views/SSettingsView.h"
#include "Widgets/SNavButton.h"
#include "XiaoShareRedis.h"
#include "XiaoAppBase.h"
#include "ShareDefine.h"
#include "CoordiManagerApp.h"


#define LOCTEXT_NAMESPACE "SCoordinatorWindow"


namespace 
{
	const FText ConnectFailed = LOCTEXT("ConnectError_Text", "协调器服务丢失连接\n请打开\"任务管理器\",然后重启所有xiaobuild相关的服务,如果没有解决问题，请联系技术支持!");
	const FText TimeoutFailed = LOCTEXT("TimeoutError_Text", "协调器服务超时连接!");
	const FText RedisFailed = LOCTEXT("RedisError_Text", "协调器服务异常!");

	struct FRedisInfo
	{
		float UseCpuPerc = 0.0f;
		uint32 Command = 0;
		float NetworkInPer = 0.0f;
		float NetworkOutPer = 0.0f;
		float TotalMemUsed = 0.0f;
		uint32 ConntectedClients = 0.0f;
	};
	static FRedisInfo GRedisInfo;
}


#define ADD_REDIS_INFO_WIDGET(BRUSH, INFO_STRING, TIP_TEXT) \
	+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center) \
	[ \
		SNew(SHorizontalBox) \
		+ SHorizontalBox::Slot().AutoWidth() \
		[ \
			SNew(SImage).Image(FXiaoStyle::Get().GetBrush(BRUSH)) \
		] \
		+ SHorizontalBox::Slot().MinWidth(20.0f).Padding(10.0f, 0.0f) \
		[ \
			SNew(STextBlock).Text_Lambda([](){ \
				return FText::FromString(INFO_STRING); \
			}) \
			.ToolTipText(TIP_TEXT) \
		] \
	] \


using namespace XiaoRedis;


SCoordinatorWindow::SCoordinatorWindow()
{
	this->SetCanTick(true);
}

SCoordinatorWindow::~SCoordinatorWindow()
{
	this->SetCanTick(false);
}

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SCoordinatorWindow::Construct(const FArguments& Args)
{
	XIAO_LOG(Log, TEXT("SCoordinatorWindow::Construct::Begin"));
	GLog->Flush();

	ConstructWidgets();

	SWindow::Construct(SWindow::FArguments()
		.Title(LOCTEXT("WindowTitle", "调度管理"))
		.ClientSize(FVector2D(650 * 2, 400 * 2))
		[
			SNew(SOverlay)
			+ SOverlay::Slot()
			[
				SNew(SSplitter)
				.Orientation(Orient_Horizontal)
#pragma region NavBar
				+ SSplitter::Slot()
				.Resizable(false)
				.SizeRule(SSplitter::ESizeRule::SizeToContent)
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot().VAlign(VAlign_Top).AutoHeight().HAlign(HAlign_Center)
					[
						SNew(SImage).Image(FXiaoStyle::Get().GetBrush("BigLogo"))
					]
#ifdef USE_IMGUI
					+ SVerticalBox::Slot().VAlign(VAlign_Top).AutoHeight().Padding(10.0f, 5.0f, 10.0, 5.0)
					[
						SAssignNew(StatsButton, SNavButton)
						.NormalImage(TEXT("stats"))
						.SeletedImage(TEXT("stats_selected"))
						.Text(LOCTEXT("NavStats_Text", "构建统计"))
						.OnPressed_Lambda([this]() 
						{
							this->ViewSwitcher->SetActiveWidgetIndex(0);
							this->ResetNavState();
							this->StatsButton->SetSelect(true);
						})
					]
#endif
					+ SVerticalBox::Slot().VAlign(VAlign_Top).AutoHeight().Padding(10.0f, 5.0f, 10.0, 5.0)
					[
						SAssignNew(AgentButton, SNavButton)
						.NormalImage(TEXT("agents"))
						.SeletedImage(TEXT("agents_selected"))
						.Text(LOCTEXT("NavAgent_Text", "代理管理"))
						.OnPressed_Lambda([this]()
						{
							this->ViewSwitcher->SetActiveWidgetIndex(0);
							this->ResetNavState();
							this->AgentButton->SetSelect(true);
						})
					]

					+ SVerticalBox::Slot().VAlign(VAlign_Top).AutoHeight().Padding(10.0f, 5.0f, 10.0, 5.0)
					[
						SAssignNew(UsersButton, SNavButton)
						.NormalImage(TEXT("users"))
						.SeletedImage(TEXT("users_selected"))
						.Text(LOCTEXT("NavUsers_Text", "用户管理"))
						.OnPressed_Lambda([this]()
						{
							this->ViewSwitcher->SetActiveWidgetIndex(1);
							this->ResetNavState();
							this->UsersButton->SetSelect(true);
						})
					]

					+ SVerticalBox::Slot().VAlign(VAlign_Top).AutoHeight().Padding(10.0f, 5.0f, 10.0, 5.0)
					[
						SAssignNew(LogsButton, SNavButton)
						.NormalImage(TEXT("agent_logs"))
						.SeletedImage(NAME_None)
						.Text(LOCTEXT("Navlogs_Text", "异常日志"))
						.OnPressed_Lambda([this]()
						{
							this->ViewSwitcher->SetActiveWidgetIndex(2);
							this->ResetNavState();
							this->LogsButton->SetSelect(true);
						})
					]

					+ SVerticalBox::Slot().VAlign(VAlign_Top).AutoHeight().Padding(10.0f, 5.0f, 10.0, 5.0)
					[
						SAssignNew(SettingsButton, SNavButton)
						.NormalImage(TEXT("settings"))
						.SeletedImage(NAME_None)
						.Text(LOCTEXT("NavSettings_Text", "系统设置"))
						.OnPressed_Lambda([this]()
						{
							this->ViewSwitcher->SetActiveWidgetIndex(3);
							this->ResetNavState();
							this->SettingsButton->SetSelect(true);
						})
					]

					+ SVerticalBox::Slot().AutoHeight().VAlign(VAlign_Center).FillHeight(1.0f)
					[
						SNullWidget::NullWidget
					]

					+ SVerticalBox::Slot().VAlign(VAlign_Bottom).AutoHeight().Padding(10.0f, 5.f, 10.0, 5.0)
					[
						SAssignNew(HelpButton, SNavButton)
						.NormalImage(TEXT("help"))
						.SeletedImage(TEXT("help_selected"))
						.Text(LOCTEXT("NavHelp_Text", "查看帮助"))
						.OnPressed_Lambda([this]()
						{
							GetHelp(GLocalization == TEXT("zh-CN") ? TEXT("4.构建协调器工具") : TEXT("4.Build Coordinator Tool"));
							this->ResetNavState();
							this->HelpButton->SetSelect(true);
						})
					]
				]
#pragma endregion
#pragma region View 
				+ SSplitter::Slot().SizeRule(SSplitter::ESizeRule::FractionOfParent)
				[
					SNew(SVerticalBox)
					+SVerticalBox::Slot().AutoHeight().VAlign(VAlign_Top)
					[
						SAssignNew(NotificationText, SErrorText)
						.AutoWrapText(true)
					]
#pragma region Stats
					+SVerticalBox::Slot().AutoHeight().VAlign(VAlign_Top).HAlign(HAlign_Right)
					[
						SNew(SHorizontalBox)
						+SHorizontalBox::Slot().HAlign(HAlign_Right)
						[
							SNew(SHorizontalBox)
							ADD_REDIS_INFO_WIDGET(TEXT("Coordi.Time"), FString::Printf(TEXT("%.1f%%"), GRedisInfo.UseCpuPerc), LOCTEXT("UseCpuPerc_ToolTup", "缓存系统使用cpu%"))
							+ SHorizontalBox::Slot().AutoWidth()[SNew(SBorder)].Padding(10.0f, 0.0f)
							ADD_REDIS_INFO_WIDGET(TEXT("Coordi.Measure"), FString::Printf(TEXT("%d/s"), GRedisInfo.Command), LOCTEXT("Command_ToolTip", "每秒执行的command数"))
							+ SHorizontalBox::Slot().AutoWidth()[SNew(SBorder)].Padding(10.0f, 0.0f)
							ADD_REDIS_INFO_WIDGET(TEXT("Coordi.Memory"), FString::Printf(TEXT("%.1f MB"), GRedisInfo.TotalMemUsed), LOCTEXT("TotalMemUsed_ToolTip", "缓存系统使用的内存"))
							+ SHorizontalBox::Slot().AutoWidth()[SNew(SBorder)].Padding(10.0f, 0.0f)
							ADD_REDIS_INFO_WIDGET(TEXT("Coordi.NetworkUpDown"), FString::Printf(TEXT("%.1f/%.1f(kbps)"), GRedisInfo.NetworkInPer, GRedisInfo.NetworkOutPer), LOCTEXT("Network_ToolTip", "网络上传下载速率"))
							+ SHorizontalBox::Slot().AutoWidth()[SNew(SBorder)].Padding(10.0f, 0.0f)
							ADD_REDIS_INFO_WIDGET(TEXT("Coordi.Client"), FString::FromInt(GRedisInfo.ConntectedClients), LOCTEXT("ConntectedClients_ToolTip", "连接的客户端数量")).Padding(0.0f, 0.0f, 60.0, 0.0f)
						]
					]
#pragma endregion
					+ SVerticalBox::Slot()
					[
						SAssignNew(ViewSwitcher, SWidgetSwitcher)
						+ SWidgetSwitcher::Slot()[AgentView.ToSharedRef()]
						+ SWidgetSwitcher::Slot()[UsersView.ToSharedRef()]
						+ SWidgetSwitcher::Slot()[LogsView.ToSharedRef()]
						+ SWidgetSwitcher::Slot()[SettingsView.ToSharedRef()]
					]
				]
#pragma endregion
			]
			+ SOverlay::Slot()
			[
				SAssignNew(MaskBorder, SBorder)
				.BorderBackgroundColor(FColor(0.0f,0.0f,0.0f, 125.0f))
				.Visibility(EVisibility::Collapsed)
			]
			+ SOverlay::Slot()
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().HAlign(EHorizontalAlignment::HAlign_Fill)
				[
					SNullWidget::NullWidget
				]
				+SHorizontalBox::Slot().HAlign(HAlign_Center).VAlign(VAlign_Top)
				[
					SAssignNew(NotificationList, SNotificationList)
				]
				+ SHorizontalBox::Slot().HAlign(EHorizontalAlignment::HAlign_Fill)
				[
					SNullWidget::NullWidget
				]
			]
		]
		.SupportsMaximize(true).SizingRule(ESizingRule::UserSized)
		.MinWidth(500.0f).MinHeight(350.0)
		.IsEnabled_Lambda([]() {
			return XiaoRedis::IsConnected() ? true : false;
		})
	);

	ViewSwitcher->SetActiveWidgetIndex(0);

	OnWindowClosed.BindRaw(this, &SCoordinatorWindow::OnExit);
	
	GOnRedisChanged.Bind([this](uint8 InRedis)
	{
		switch (static_cast<ERedisStatus>(InRedis))
		{
		case Redis_Ok:
		{
			SetErrorText(FText::GetEmpty());
			break;
		}
		case Redis_CloseError:
		case Redis_IoError:
		case Redis_TimeoutError:
		{
			SetErrorText(ConnectFailed);
			break;
		}
		case Redis_Error:
		default:
			SetErrorText(RedisFailed);
			break;
		}
	});

	FSlateApplication::Get().OnFocusChanging().AddLambda([](const FFocusEvent& FocusEvent, const FWeakWidgetPath& OldFocusedWidgetPath, const TSharedPtr<SWidget>& OldFocusedWidget, const FWidgetPath& NewFocusedWidgetPath, const TSharedPtr<SWidget>& NewFocusedWidget)
	{
		if (NewFocusedWidget.IsValid())
		{
			GCanUpdate = true;
		}
	});

	if (XiaoRedis::SRedisClient)
	{
		OnUpdate();
		AgentView->OnUpdate(true);
		UsersView->OnUpdate(true);
		LogsView->OnUpdate(true);
		SettingsView->OnUpdate(true);
	}

	this->AgentButton->SetSelect(true);

	XIAO_LOG(Log, TEXT("SCoordinatorWindow::Construct::Finish"));
	GLog->Flush();
	LastLoginTime = FPlatformTime::Seconds();
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SCoordinatorWindow::Tick(const FGeometry& InAllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	SWindow::Tick(InAllottedGeometry, InCurrentTime, InDeltaTime);

	// 非管理类型则有时间限制
	if (FXiaoAppBase::GApp && GCurrentUser.Role != 2)
	{
		const double PassTime = FPlatformTime::Seconds() - LastLoginTime;
		// 需要再次登录 锁定切到登录页面
		static constexpr double SCheckTime = 3600.0f * 6;
		if (PassTime > SCheckTime)
		{
			SetCanTick(false);
			LastLoginTime = FPlatformTime::Seconds();
			if (FCoordiManagerApp* App = static_cast<FCoordiManagerApp*>(FXiaoAppBase::GApp))
			{
				App->ShowLoginWindow();
				return;
			}
		}
	}

	if (!GCanUpdate)
	{
		return;
	}

	// 重新连接
	if (!XiaoRedis::IsConnected())
	{
		XiaoRedis::AsyncReconnectRedis();
		return;
	}

	static double LastTime = 0.0;
	if ((FPlatformTime::Seconds() - LastTime) > GSleepUpdate)
	{
		LastTime = FPlatformTime::Seconds();
		OnUpdate();
		AgentView->OnUpdate(false);
		UsersView->OnUpdate(false);
		LogsView->OnUpdate(false);
		SettingsView->OnUpdate(false);
		return;
	}
}

FReply SCoordinatorWindow::OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent)
{
	if (InKeyEvent.GetKey() == EKeys::F11)
	{
		bFullScreen = !bFullScreen;
		const TSharedPtr<FGenericWindow> NativeWindow = GetNativeWindow();
		if (!NativeWindow.IsValid())
		{
			return FReply::Handled();
		}
		if (bFullScreen && !IsWindowMaximized())
		{
			NativeWindow->Maximize();
		}
		else if (!bFullScreen && IsWindowMaximized())
		{
			NativeWindow->Restore();
		}
		return FReply::Handled();
	}
	return SWindow::OnKeyDown(MyGeometry, InKeyEvent);
}

void SCoordinatorWindow::OnExit(const TSharedRef<SWindow>& InWindow)
{
	RequestEngineExit(TEXT("XiaoCoordiManager Closed"));
}

void SCoordinatorWindow::SetLockedState(const bool InbLock) const
{
	MaskBorder->SetVisibility(InbLock ? EVisibility::Visible : EVisibility::Collapsed);
}

void SCoordinatorWindow::SetErrorText(const FText& InText)
{	
	if (NotificationText.IsValid())
	{
		NotificationText->SetError(InText);
	}
}

static FORCEINLINE std::string TryGetVal(const std::string& InStr, const std::string& InTarget)
{
	const auto FlagSize = InTarget.size();
	const auto First = InStr.find(InTarget);
	if (First != std::string::npos)
	{
		const auto Off = First + FlagSize;
		const auto End = InStr.find_first_of("\r\n", Off);
		if (End != std::string::npos)
		{
			return InStr.substr(Off, End-Off);
		}
	}
	return "";
}

void SCoordinatorWindow::OnUpdate()
{
	if (!XiaoRedis::IsConnected())
	{
		return;
	}
	try
	{
		static const std::string SINFO("INFO");
		if (const auto CpuReplay = SRedisClient->command(SINFO, "cpu"))
		{
			const std::string Val = TryGetVal(CpuReplay->str, "used_cpu_sys_children:");
			if (!Val.empty())
			{
				GRedisInfo.UseCpuPerc = std::atof(Val.c_str());
			}
		}

		if (const auto StatsReplay = SRedisClient->command(SINFO, "stats"))
		{
			std::string Val = TryGetVal(StatsReplay->str, "instantaneous_ops_per_sec:");
			if (!Val.empty())
			{
				GRedisInfo.Command = std::atoi(Val.c_str());
			}

			Val = "";
			Val = TryGetVal(StatsReplay->str, "instantaneous_input_kbps:");
			if (!Val.empty())
			{
				GRedisInfo.NetworkInPer = std::atof(Val.c_str());
			}

			Val = "";
			Val = TryGetVal(StatsReplay->str, "instantaneous_output_kbps:");
			if (!Val.empty())
			{
				GRedisInfo.NetworkOutPer = std::atof(Val.c_str());
			}
		}

		if (const auto MemoryReplay = SRedisClient->command(SINFO, "memory"))
		{
			const std::string Val = TryGetVal(MemoryReplay->str, "used_memory:");
			if (!Val.empty())
			{
				GRedisInfo.TotalMemUsed = std::atof(Val.c_str()) / 1024.0/ 1024.0f;
			}
		}

		if (const auto ClientsReplay = SRedisClient->command(SINFO, "clients"))
		{
			const std::string Val = TryGetVal(ClientsReplay->str, "connected_clients:");
			if (!Val.empty())
			{
				GRedisInfo.ConntectedClients = std::atoi(Val.c_str());
			}
		}
	}
	CATCH_REDIS_EXCEPTRION();
}

void SCoordinatorWindow::ResetNavState() const
{
	AgentButton->SetSelect(false);
	UsersButton->SetSelect(false);
	LogsButton->SetSelect(false);
	SettingsButton->SetSelect(false);

	HelpButton->SetSelect(false);
}

void SCoordinatorWindow::ConstructWidgets()
{
	if (!AgentView.IsValid())
	{
		AgentView = SNew(SAgentView)
		.OnQueueNotification_Raw(this, &SCoordinatorWindow::OnQueueNotification);
	}

	if (!UsersView.IsValid())
	{
		UsersView = SNew(SUsersView)
		.OnQueueNotification_Raw(this, &SCoordinatorWindow::OnQueueNotification);
	}

	if (!LogsView.IsValid())
	{
		LogsView = SNew(SLogsView)
		.OnQueueNotification_Raw(this, &SCoordinatorWindow::OnQueueNotification);
	}

	if (!SettingsView.IsValid())
	{
		SettingsView = SNew(SSettingsView)
		.OnQueueNotification_Raw(this, &SCoordinatorWindow::OnQueueNotification);
	}
}

void SCoordinatorWindow::OnQueueNotification(const int8 InStatus, const FText& InText)
{
	FNotificationInfo Info(InText);
	Info.Image = FXiaoStyle::Get().GetBrush(InStatus == 0 ? TEXT("Port.OK") : (InStatus == -1 ? TEXT("Port.NG") : TEXT("warning")));
	Info.FadeInDuration = 0.5f;
	Info.FadeOutDuration = 0.5f;
	Info.ExpireDuration = InStatus == 0 ? 1.0f : (InStatus == -1 ? 3.0f : 2.0f);
	Info.bFireAndForget = true;
	Info.ForWindow = FSlateApplication::Get().FindWidgetWindow(SharedThis(this));
	NotificationList->AddNotification(Info);
}

#undef REDIS_INFO_WIDGET

#undef LOCTEXT_NAMESPACE
