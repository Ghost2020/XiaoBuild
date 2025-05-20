#include "SAgentSettingsWindow.h"

#include "XiaoStyle.h"
#include "SlateOptMacros.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SExpandableArea.h"
#include "Widgets/Layout/SWidgetSwitcher.h"
#include "Widgets/Notifications/SErrorText.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "Widgets/Layout/SSplitter.h"

#include "App/Slate/Widgets/SCategoryWidget.h"
#include "Widgets/Layout/SScrollBox.h"

#include "SettingsViews/SAgentGeneralView.h"
#include "SettingsViews/SUbaAgentSettingsView.h"
#include "SettingsViews/SNetworkCoordinatorView.h"
#include "SettingsViews/SInitiatorAdvancedView.h"

#include "Dialogs/SMessageWindow.h"

#include "Misc/CommandLine.h"
#include "XiaoShareField.h"
#include "XiaoShareRedis.h"
#include "XiaoAgent.h"
#include "XiaoAppBase.h"
#include "agent.pb.h"


#define LOCTEXT_NAMESPACE "SAgentSettingsWindow"


static FAgentProto SAgentProto;
static std::string SAgentUniqueId = TCHAR_TO_UTF8(*GetUniqueDeviceID());


SAgentSettingsWindow::SAgentSettingsWindow()
{
}

SAgentSettingsWindow::~SAgentSettingsWindow()
{

}

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SAgentSettingsWindow::Construct(const FArguments& Args)
{
	ConstructWidgets();

	SWindow::Construct(SWindow::FArguments()
		.Title(LOCTEXT("AgentTitle", "代理设置"))
		.SupportsMaximize(false)
		.SizingRule(ESizingRule::FixedSize)
		.ClientSize(FVector2D(GWindow_Width * 2, GWindow_Height * 2))
		[
			SNew(SOverlay)
			+ SOverlay::Slot()
			[
			SNew(SVerticalBox)
#pragma region LeftMenu
			+SVerticalBox::Slot().AutoHeight()
			[
				SAssignNew(ErrorText, SErrorText)
				.Visibility(EVisibility::Collapsed)
				.ToolTipText(LOCTEXT("NetworkErrorToolTip_Text", "前往->网络->协调器-> 进行网络连通测试\n如无法连通，则可能调度器掉线，或IP地址变化！\n可向网络管理员询问"))
			]
			+ SVerticalBox::Slot()
			[
				SNew(SSplitter)
				.Orientation(Orient_Horizontal)
				+ SSplitter::Slot()
				.Resizable(false)
				.SizeRule(SSplitter::ESizeRule::FractionOfParent)
				.Value(0.2f)
				[
					SNew(SScrollBox).Orientation(Orient_Vertical)
					+ SScrollBox::Slot()
					[
						SNew(SVerticalBox)
#pragma region Network
						+ SVerticalBox::Slot().AutoHeight()

						[
							SAssignNew(NetworkArea, SExpandableArea)
							.InitiallyCollapsed(false)
							.AreaTitle(LOCTEXT("Network_Text", "网络"))
							.BodyContent()
							[
								SNew(SVerticalBox)

								+ SVerticalBox::Slot()
								[
									SAssignNew(NetworkCoordinatorCategory, SCategoryWidget)
									.Text(LOCTEXT("Coordinator_Text", "网络设置"))
									.OnPressed_Lambda([this]() {this->WidgetSwitcher->SetActiveWidget(NetworkCoordinator.ToSharedRef()); })
								]
							]
						]
#pragma endregion Network
#pragma region Agent
						+SVerticalBox::Slot().AutoHeight()
						[
							SAssignNew(AgentArea, SExpandableArea)
							.AreaTitle(LOCTEXT("Agent_Text", "代理"))
							.InitiallyCollapsed(false)
							.BodyContent()
							[
								SNew(SVerticalBox)
								
								+ SVerticalBox::Slot()
								[
									SAssignNew(AgentGeneralCategory, SCategoryWidget)
									.Text(LOCTEXT("General_Text", "核心组件"))
									.OnPressed_Lambda([this] (){this->WidgetSwitcher->SetActiveWidget(AgentGeneral.ToSharedRef());})
								]

								+ SVerticalBox::Slot()
								[
									SAssignNew(AgentAdvancedCategory, SCategoryWidget)
									.Text(LOCTEXT("AgentAdvanced_Text", "高级设置"))
									.OnPressed_Lambda([this] (){this->WidgetSwitcher->SetActiveWidget(UbaAgentAdvanced.ToSharedRef());})
								]
							]
						]
#pragma endregion Agent

#pragma region Initiator
						+SVerticalBox::Slot().AutoHeight()
						[
							SAssignNew(InitiatorArea, SExpandableArea)
							.InitiallyCollapsed(false)
							.AreaTitle(LOCTEXT("Initiator_Text", "调度"))
							.BodyContent()
							[
								SNew(SVerticalBox)
								
								+SVerticalBox::Slot()
								[
									SAssignNew(InitiatorAdvancedCategory, SCategoryWidget)
									.Text(LOCTEXT("UBACAdvanced_Text", "调度设置"))
									.OnPressed_Lambda([this] (){this->WidgetSwitcher->SetActiveWidget(InitiatorAdvanced.ToSharedRef());})
								]
							]
						]
#pragma endregion Initiator

#pragma endregion TrayIcon
					]
				]
#pragma endregion
#pragma region View
				+ SSplitter::Slot()
				[
					SAssignNew(WidgetSwitcher, SWidgetSwitcher)
					+ SWidgetSwitcher::Slot()[NetworkCoordinator.ToSharedRef()]
					+ SWidgetSwitcher::Slot()[AgentGeneral.ToSharedRef()]
					+ SWidgetSwitcher::Slot()[UbaAgentAdvanced.ToSharedRef()]
					+ SWidgetSwitcher::Slot()[InitiatorAdvanced.ToSharedRef()]
				]
			]
#pragma endregion View
#pragma region Foot
			+ SVerticalBox::Slot()
			.MaxHeight(45.0f).AutoHeight()
			.VAlign(VAlign_Bottom)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.HAlign(HAlign_Left)
				[
					SNew(SBorder)
					.BorderImage(FXiaoStyle::Get().GetBrush("NoBorder"))
					[
						SNew(SButton)
						.Text(LOCTEXT("Help_Text", "求助"))
						.TextStyle(FXiaoStyle::Get(), "FlatButton.DefaultTextStyle")
						.OnClicked_Lambda([] () ->FReply
						{
							GetHelp(GLocalization == TEXT("zh-CN") ? TEXT("5.代理设置工具") : TEXT("5.Agent Settings Tool"));
							return FReply::Handled();
						})
					]
				]

				+ SHorizontalBox::Slot().HAlign(HAlign_Right).AutoWidth()
				[
					SNew(SBorder)
					.Visibility_Lambda([this]() { return OnCanCommit() ? EVisibility::Visible : EVisibility::Collapsed; })
					.BorderImage(FXiaoStyle::Get().GetBrush("NoBorder"))
					[
						SNew(SButton).HAlign(HAlign_Center)
						.Text(LOCTEXT("Confirm_Text", "确定"))
						.ToolTipText(LOCTEXT("ConfimeToolTip_Text","保存当前的修改"))
						.ButtonStyle(FXiaoStyle::Get(), "FlatButton.Warning")
						.TextStyle(FXiaoStyle::Get(), "FlatButton.DefaultTextStyle")
						.OnClicked_Raw(this, &SAgentSettingsWindow::OnCommit)
					]
				]

				+ SHorizontalBox::Slot().HAlign(HAlign_Right).AutoWidth()
				[
					SNew(SBorder)
					.Visibility_Lambda([this]() { return OnCanCommit() ? EVisibility::Visible : EVisibility::Collapsed; })
					.BorderImage(FXiaoStyle::Get().GetBrush("NoBorder"))
					[
						SNew(SButton).HAlign(HAlign_Center)
						.Text(LOCTEXT("Cancel_Text", "取消"))
						.ToolTipText(LOCTEXT("CancelToolTip_Text", "放弃当前的修改,退回到修改之前"))
						.ButtonStyle(FXiaoStyle::Get(), "FlatButton.Primary")
						.TextStyle(FXiaoStyle::Get(), "FlatButton.DefaultTextStyle")
						.OnClicked_Lambda([this] ()
						{
							SModifiedAgentSettings = SOriginalAgentSettings;
							AgentGeneral->OnRevert();
							AgentGeneral->ForceVolatile(true);
							UbaAgentAdvanced->ForceVolatile(true);

							NetworkCoordinator->Redraw();
							NetworkCoordinator->ForceVolatile(true);

							InitiatorAdvanced->ForceVolatile(true);
							return FReply::Handled();
						})
					]
				]
			]
			]
			+ SOverlay::Slot()
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().HAlign(EHorizontalAlignment::HAlign_Fill)
				[
					SNullWidget::NullWidget
				]
				+ SHorizontalBox::Slot().HAlign(HAlign_Center).VAlign(VAlign_Top)
				[
					SAssignNew(NotificationList, SNotificationList)
				]
				+ SHorizontalBox::Slot().HAlign(EHorizontalAlignment::HAlign_Fill)
				[
					SNullWidget::NullWidget
				]
			]
#pragma endregion
		].SupportsMaximize(true).SizingRule(ESizingRule::UserSized)
	);

	WidgetSwitcher->SetActiveWidgetIndex(0);

	if(FParse::Param(FCommandLine::Get(), TEXT("coordi_network")))
	{
		WidgetSwitcher->SetActiveWidget(NetworkCoordinator.ToSharedRef());
	}

	OnWindowClosed.BindRaw(this, &SAgentSettingsWindow::OnExit);
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SAgentSettingsWindow::OnExit(const TSharedRef<SWindow>& InWindow) const
{
	OnExit();
}

void SAgentSettingsWindow::ConstructWidgets()
{
	LoadSettings();

	SModifiedAgentSettings = SOriginalAgentSettings;
	
	if(!AgentGeneral.IsValid())
	{
		AgentGeneral = SNew(SAgentGeneralView);
	}
	if(!UbaAgentAdvanced.IsValid())
	{
		UbaAgentAdvanced = SNew(SUbaAgentSettingsView);
	}

	if(!NetworkCoordinator.IsValid())
	{
		NetworkCoordinator = SNew(SNetworkCoordinatorView, &SModifiedAgentSettings.NetworkCoordinate)
		.OnTestPressed_Raw(this, &SAgentSettingsWindow::AsyncNetworkTest);
	}
	if(!InitiatorAdvanced.IsValid())
	{
		InitiatorAdvanced = SNew(SInitiatorAdvancedView);
	}

	AsyncNetworkTest();
}

bool SAgentSettingsWindow::OnCanCommit() const
{
	return !(SModifiedAgentSettings == SOriginalAgentSettings) || (AgentGeneral.IsValid() ? AgentGeneral->OnCanCommit() : false);
}

FReply SAgentSettingsWindow::OnCommit()
{
	if (XiaoRedis::IsConnected())
	{
		try
		{
			if (const auto AgentOption = XiaoRedis::SRedisClient->hget(XiaoRedis::Hash::SAgentStats, SAgentUniqueId))
			{
				const std::string Value = AgentOption.value();
				if (Value.size() > 0 && SAgentProto.ParseFromString(Value))
				{
					SAgentProto.set_helperport(SModifiedAgentSettings.NetworkCoordinate.Port);
					SAgentProto.set_maxcon(SModifiedAgentSettings.UbaScheduler.MaxCon);
					SAgentProto.set_maxcpu(SModifiedAgentSettings.UbaScheduler.MaxCpu);
					SAgentProto.set_helpercore(SModifiedAgentSettings.UbaAgent.MaxCpu);
					const std::string Desc = TCHAR_TO_UTF8(*SModifiedAgentSettings.NetworkCoordinate.Desc);
					SAgentProto.set_desc(Desc);
					const std::string Group = TCHAR_TO_UTF8(*SModifiedAgentSettings.NetworkCoordinate.Group);
					SAgentProto.set_group(Group);

					std::string Str;
					if (SAgentProto.SerializeToString(&Str) && Str.size() > 0)
					{
						XiaoRedis::SRedisClient->hset(XiaoRedis::Hash::SAgentStats, SAgentUniqueId, Str);
					}
				}
			}
			else
			{
				XIAO_LOG(Error, TEXT("hget agent desc failed"));
			}
		}
		CATCH_REDIS_EXCEPTRION();
	}

	// 清除对应的缓存数据
	const FString OriUbaDir = SOriginalAgentSettings.UbaAgent.Dir;
	if (!FPaths::IsSamePath(SModifiedAgentSettings.UbaAgent.Dir, OriUbaDir) && FPaths::DirectoryExists(OriUbaDir) && !IsAppRunning(XiaoAppName::SUbaAgent))
	{
		IFileManager::Get().DeleteDirectory(*OriUbaDir, /*RequireExists=*/false, /*Tree=*/true);	
	}
	const FString OriSchedulerDir = SOriginalAgentSettings.UbaScheduler.Dir;
	if (!FPaths::IsSamePath(SModifiedAgentSettings.UbaScheduler.Dir, OriSchedulerDir) && FPaths::DirectoryExists(OriSchedulerDir) && !IsAppRunning(XiaoAppName::SXiaoScheduler))
	{
		IFileManager::Get().DeleteDirectory(*OriSchedulerDir, /*RequireExists=*/false, /*Tree=*/true);	
	}
	SOriginalAgentSettings = SModifiedAgentSettings;
	if (SaveAgentSettings(SOriginalAgentSettings))
	{
		OnQueueNotification(0, LOCTEXT("SaveSuccess_Text", "保存成功"));
	}

	if (AgentGeneral.IsValid())
	{
		AgentGeneral->OnCommit();
	}
	
	return FReply::Handled();
}

void SAgentSettingsWindow::OnExit() const
{
	RequestEngineExit(TEXT("RequestExit"));
}

bool SAgentSettingsWindow::LoadSettings()
{
	if (LoadAgentSettings(SOriginalAgentSettings))
	{
		SModifiedAgentSettings = SOriginalAgentSettings;
		return true;
	}
	return false;
}

void SAgentSettingsWindow::ChangeStatus(const bool Status) const
{
	if (FXiaoAppBase::GApp)
	{
		FXiaoAppBase::GApp->AddNextTickTask(FSimpleDelegate::CreateLambda([this, Status]()
		{
			ErrorText->SetError(LOCTEXT("ConnectedFailed_Text", "无法连接调度器"));
			
			ErrorText->SetVisibility(Status ? EVisibility::Visible : EVisibility::Collapsed);
			if (Status)
			{
				const auto Window = SNew(SMessageWindow)
					.TiTile(XiaoError::SConnectToCoordinatorTitle)
					.Message(XiaoError::SConnectToCoordinatorMessage);

				FSlateApplication::Get().AddModalWindow(Window, SharedThis(this), false);
			}
		}));
		NetworkCoordinator->SetStatus(Status ? -1 : 0);
	}
}

void SAgentSettingsWindow::OnQueueNotification(const int8 InStatus, const FText& InText) const
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

void SAgentSettingsWindow::AsyncNetworkTest()
{
	AsyncThread([this]()
	{
		ConnectionOptions Options;
		Options.host = TCHAR_TO_UTF8(*SModifiedAgentSettings.NetworkCoordinate.IP);
		Options.port = SModifiedAgentSettings.NetworkCoordinate.Port;
		Options.keep_alive = true;
		Options.db = 0;
		Options.connect_timeout = std::chrono::milliseconds(1000);
		XiaoRedis::TryDisconnectRedis();

		try
		{
			XiaoRedis::SRedisClient = std::make_shared<Redis>(Options);
#if !PLATFORM_MAC
			const std::string Msg = XiaoRedis::SRedisClient->ping().c_str();
			if (Msg == XiaoRedis::Key::SPong)
			{
#endif
				SRedisStatus = ERedisStatus::Redis_Ok;
				if (SaveAgentSettings(SModifiedAgentSettings))
				{
					ChangeStatus(false);

					if (const auto AgentOption = XiaoRedis::SRedisClient->hget(XiaoRedis::Hash::SAgentStats, SAgentUniqueId))
					{
						const std::string Value = AgentOption.value();
						if (Value.size() > 0 && SAgentProto.ParseFromString(Value))
						{
							const bool bLog = SAgentProto.loglevel() > 3 ? true : false;
							const bool bQuite = SAgentProto.loglevel() <= 1 ? true : false;
							SModifiedAgentSettings.UbaScheduler.MaxCon = SAgentProto.maxcon();
							SModifiedAgentSettings.UbaScheduler.MaxCpu = SAgentProto.maxcpu();
							SModifiedAgentSettings.UbaScheduler.bLog = bLog;
							SModifiedAgentSettings.UbaScheduler.bSummary = bLog;
							SModifiedAgentSettings.UbaScheduler.bQuiet = bQuite;
							SModifiedAgentSettings.UbaAgent.MaxCpu = SAgentProto.helpercore();
							SModifiedAgentSettings.UbaAgent.bLog = bLog;
							SModifiedAgentSettings.UbaAgent.bSummary = bLog;
							SModifiedAgentSettings.UbaAgent.bQuiet = bQuite;
							SModifiedAgentSettings.NetworkCoordinate.Desc = UTF8_TO_TCHAR(SAgentProto.desc().c_str());
							SModifiedAgentSettings.NetworkCoordinate.Group = UTF8_TO_TCHAR(SAgentProto.group().c_str());
							// #TODO SModifiedAgentSettings.UbaScheduler.MaxLocalCore = SAgentProto.
						}
					}

					SOriginalAgentSettings = SModifiedAgentSettings;
					NetworkCoordinator->SetStatus(0);
				}
				return;
#if !PLATFORM_MAC
			}
#endif
		}
		CATCH_REDIS_EXCEPTRION();
		ChangeStatus(true);
	});
}

#undef LOCTEXT_NAMESPACE
