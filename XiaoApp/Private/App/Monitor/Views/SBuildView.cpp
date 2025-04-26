/**
  * @author cxx2020@outlook.com
  * @date 10:47 PM
 */
#include "SBuildView.h"
#include "SBuildProgressView.h"
#include "SBuildOutputView.h"

#include "XiaoShare.h"
#include "XiaoShareField.h"
#include "XiaoStyle.h"
#include "XiaoLog.h"

#include "Framework/Docking/TabManager.h"
#include "Misc/MessageDialog.h"
#include "Misc/CommandLine.h"
#include "Widgets/Notifications/SProgressBar.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "SlateOptMacros.h"
#include "Widgets/Docking/SDockTab.h"

#define LOCTEXT_NAMESPACE "SBuildView"

static const FName SBuildProgressName(TEXT("BuildProgress"));
static const FName SBuildOutputName(TEXT("BuildOutput"));


BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SBuildView::Construct(const FArguments& InArgs)
{
	OwnerWindow = InArgs._OwnerWindow;

	ConstructWidgets();
	
	const TSharedRef<SDockTab> DockTab = SNew(SDockTab).TabRole(MajorTab);
	TabManager = FGlobalTabmanager::Get()->NewTabManager(DockTab);
	const TSharedRef<FWorkspaceItem> ProgressMenuGroup = TabManager->AddLocalWorkspaceMenuCategory(LOCTEXT("BuildMenuGourpName", "进度视图"));

	TabManager->RegisterTabSpawner(SBuildProgressName, FOnSpawnTab::CreateRaw(this, &SBuildView::OnSpawnTab_ProgressView))
	.SetDisplayName(LOCTEXT("BuildProgressView_Text", "构建进度"))
	.SetIcon(FSlateIcon(FXiaoStyle::Get().GetStyleSetName(), TEXT("Icons.Trace")))
	.SetGroup(ProgressMenuGroup);

	TabManager->RegisterTabSpawner(SBuildOutputName, FOnSpawnTab::CreateRaw(this, &SBuildView::OnSpawnTab_OutputView))
	.SetDisplayName(LOCTEXT("BuildOutputView_Text", "构建输出"))
	.SetIcon(FSlateIcon(FXiaoStyle::Get().GetStyleSetName(), TEXT("Icons.Output")))
	.SetGroup(ProgressMenuGroup);

	const TSharedRef<FTabManager::FLayout> Layout = FTabManager::NewLayout("BuildView_v1")
	->AddArea
	(
		FTabManager::NewPrimaryArea()
		->SetOrientation(Orient_Vertical)
		->Split
		(
			FTabManager::NewStack()
			->AddTab(SBuildProgressName, ETabState::OpenedTab)
		)
		->Split
		(
			FTabManager::NewStack()
			->AddTab(SBuildOutputName, ETabState::OpenedTab)
			->SetSizeCoefficient(0.4f)
		)
	);

	FSlimHorizontalToolBarBuilder LeftToolbar(nullptr, FMultiBoxCustomization::None);

	LeftToolbar.BeginSection("Menus");
	LeftToolbar.AddToolBarButton(
		FUIAction(
			FExecuteAction::CreateSP(this, &SBuildView::OnViewSessionGraph),
			FCanExecuteAction::CreateLambda([]() { return true; }),
			FIsActionChecked::CreateSP(this, &SBuildView::IsShowingSesionGraph)
		),
		NAME_None,
		LOCTEXT("ShowSession", "显示Session"),
		LOCTEXT("ShowSession_ToolTip", "是否在显示session性能图表指标"),
		FSlateIcon(FXiaoStyle::Get().GetStyleSetName(), "Icons.network"),
		EUserInterfaceActionType::ToggleButton
	);

	LeftToolbar.AddToolBarButton(
		FUIAction(
			FExecuteAction::CreateSP(this, &SBuildView::OnViewProcessorTrack),
			FCanExecuteAction::CreateLambda([]() { return true; }),
			FIsActionChecked::CreateSP(this, &SBuildView::IsShowingProcessorTrack)
		),
		NAME_None,
		LOCTEXT("ShowProssor", "显示Track"),
		LOCTEXT("ShowProssor_ToolTip", "是否显示各个处理器上处理任务的进度"),
		FSlateIcon(FXiaoStyle::Get().GetStyleSetName(), "Icons.Prosesor"),
		EUserInterfaceActionType::ToggleButton
	);

	LeftToolbar.AddToolBarButton(
		FUIAction(
			FExecuteAction::CreateSP(this, &SBuildView::OnViewDetailsTrack),
			FCanExecuteAction::CreateLambda([]() { return true; }),
			FIsActionChecked::CreateSP(this, &SBuildView::IsShowingDetailsTrack)
		),
		NAME_None,
		LOCTEXT("ShowDetails", "显示Details"),
		LOCTEXT("ShowDetails_ToolTip", "是否显示每个会话的细节信息"),
		FSlateIcon(FXiaoStyle::Get().GetStyleSetName(), "Icons.Details"),
		EUserInterfaceActionType::ToggleButton
	);

	ChildSlot
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot().VAlign(VAlign_Top).AutoHeight()
		[
			LeftToolbar.MakeWidget()
		]
		+SVerticalBox::Slot().VAlign(VAlign_Fill)
		[
			TabManager->RestoreFrom(Layout, OwnerWindow.Pin()).ToSharedRef()
		]
#pragma region BottomProgress
 		+ SVerticalBox::Slot()
 		.MaxHeight(45.0f).AutoHeight()
 		.VAlign(VAlign_Bottom)
 		[
 			SNew(SOverlay)
 			+SOverlay::Slot().HAlign(HAlign_Fill)
 			[
 				SAssignNew(ProgressBar, SProgressBar)
 				.Visibility(EVisibility::Collapsed)
 				.BarFillType(EProgressBarFillType::Type::LeftToRight)
 				.BorderPadding(FVector2D(2.0, 2.0))
 				.Percent(0.0f)
 			]
 			+SOverlay::Slot().HAlign(HAlign_Center)
 			[
 				SNew(SHorizontalBox)
 				+SHorizontalBox::Slot().AutoWidth()
 				[
 					SAssignNew(ProgressText, STextBlock)
 					.Visibility(EVisibility::Collapsed)
 					.Text(FText::FromString(TEXT("1/1")))
					.ToolTipText(LOCTEXT("Progress_ToolTip", "完成数量/总共数量"))
 				]
 			]
 		]
#pragma endregion
	];

	XIAO_LOG(Log, TEXT("SBuildView::Construct Finish!"));
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION

bool SBuildView::StartReadFile(const FString& InBuildFile)
{
	TraceReader.ReadFile(TraceView, *InBuildFile, false);
	ProgressView->LoadFromTraceView(TraceView);
	OutputView->LoadFromTraceView(TraceView);

	UpdateProgress(true);

	const FString TraceName = TraceReader.m_namedTrace.IsEmpty() ? FPaths::GetBaseFilename(InBuildFile) : TraceReader.m_namedTrace;
	const FString TitleName = FString::Printf(TEXT("BuildInsight-%s"), *TraceName);
	if (auto Window = FSlateApplication::Get().FindWidgetWindow(SharedThis(this)))
	{
		Window->SetTitle(FText::FromString(TitleName));
	}
	return true;
}

bool SBuildView::StartReadNetwork(const FString& InHostIp, const int32 InListenPort)
{
	bNetwork = true;
	if (NetworkTrace.Init())
	{
		// 尝试测试三次
		for (int32 TestPort = InListenPort; TestPort < InListenPort+3; ++TestPort)
		{
			if (NetworkTrace.Connect(InHostIp, TestPort))
			{
				SchedulerPort = TestPort;
				bRealtime = true;
				break;
			}
		}

		if (!bRealtime)
		{
			return false;
		}

		bRealtime = TraceReader.StartReadClient(TraceView, NetworkTrace);
		this->SetCanTick(bRealtime);
		ProgressBar->SetVisibility(EVisibility::Visible);
		ProgressBar->SetCanTick(bRealtime);
		ProgressBar->SetFillColorAndOpacity(FColor::Magenta);
		ProgressText->SetVisibility(EVisibility::Visible);

		const FString TraceName = FString::Printf(TEXT("Connected with %s:%d"), *InHostIp, SchedulerPort);
		const FString TitleName = FString::Printf(TEXT("%s-%s"), *XiaoAppName::SBuildMonitor, *TraceName);
		if (auto Window = FSlateApplication::Get().FindWidgetWindow(SharedThis(this)))
		{
			Window->SetTitle(FText::FromString(TitleName));
		}

#if PLATFORM_WINDOWS
		const int32 HighPriority = REALTIME_PRIORITY_CLASS;
		SetCurrentProcessPriority(HighPriority);
#endif

		TickHandle = FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateLambda([this](float InDelta) {
			static double LastTime = 0.0f;
			if (bRealtime && (FPlatformTime::Seconds() - LastTime) > 0.2f)
			{
				LastTime = FPlatformTime::Seconds();
				// 只有没有任务时才进行检测 网络活跃检测 
				static double LastNeworkActiveTime;
				if ((LastTime - LastNeworkActiveTime) > 5.f)
				{
					LastNeworkActiveTime = LastTime;
					if (!NetworkTrace.IsConnected())
					{
#if PLATFORM_WINDOWS
						const int32 HighPriority = NORMAL_PRIORITY_CLASS;
						SetCurrentProcessPriority(HighPriority);
#endif
						FTSTicker::GetCoreTicker().RemoveTicker(TickHandle);
						bRealtime = false;
						UpdateProgress();
						return false;
					}
				}

				bool bChanged = false;
				if (bNetwork)
				{
					bChanged = TraceReader.UpdateReadClient(TraceView, NetworkTrace, bChanged) && bChanged;
				}
				else
				{
					uint64 MaxTime = 0;
					bChanged = TraceReader.UpdateReadNamed(TraceView, MaxTime, bChanged) && bChanged;
				}
				if (bChanged)
				{
					UpdateProgress();
				}
			}

			return true;
		}));
	}
	return bRealtime;
}

bool SBuildView::StartReadNamed(const FString& InNamedTrace)
{
	bNetwork = false;
	bRealtime = TraceReader.StartReadNamed(TraceView, *InNamedTrace, true);
	if (bRealtime)
	{
		this->SetCanTick(bRealtime);
		ProgressBar->SetVisibility(EVisibility::Visible);
		ProgressBar->SetCanTick(bRealtime);
		ProgressBar->SetFillColorAndOpacity(FColor::Magenta);
		ProgressText->SetVisibility(EVisibility::Visible);

		const FString TitleName = FString::Printf(TEXT("%s-%s"), *XiaoAppName::SBuildMonitor, *TraceReader.m_namedTrace);
		if (auto Window = FSlateApplication::Get().FindWidgetWindow(SharedThis(this)))
		{
			Window->SetTitle(FText::FromString(TitleName));
		}
	}
	return bRealtime;
}

void SBuildView::UpdateProgress(const bool bFirst)
{
	if (!bFirst)
	{
		ProgressView->UpdateView(TraceView);
		OutputView->UpdateLogList(TraceView);
	}
	else
	{
		ProgressBar->SetVisibility(EVisibility::Visible);
		ProgressText->SetVisibility(EVisibility::Visible);
	}

	ProgressBar->SetPercent(float(TraceView.progressProcessesDone) / float(TraceView.progressProcessesTotal));
	ProgressText->SetText(FText::FromString(FString::Printf(TEXT("%d/%d"), TraceView.progressProcessesDone, TraceView.progressProcessesTotal)));
	if (TraceView.progressErrorCount > 0)
	{
		ProgressBar->SetFillColorAndOpacity(FColor::Red);
	}

	if (TraceView.finished)
	{
		OutputView->AddSummaryLog(TraceView);
		if (TraceView.progressProcessesDone == TraceView.progressProcessesTotal)
		{
			ProgressBar->SetFillColorAndOpacity(FColor::Green);
		}
		ProgressBar->SetCanTick(false);
		bRealtime = false;
	}
}

bool SBuildView::GetRealtime() const
{
	return bRealtime;
}

void SBuildView::OnViewSessionGraph()
{
	if (ProgressView.IsValid())
	{
		ProgressView->bShowSessionGraph = !ProgressView->bShowSessionGraph;
		ProgressView->UpdateView(TraceView);
	}
}

bool SBuildView::IsShowingSesionGraph() const
{
	if (ProgressView.IsValid())
	{
		return ProgressView->bShowSessionGraph;
	}
	return false;
}

void SBuildView::OnViewDetailsTrack()
{
	if (ProgressView.IsValid())
	{
		ProgressView->bShowSessionDetails = !ProgressView->bShowSessionDetails;
		ProgressView->UpdateView(TraceView);
	}
}

bool SBuildView::IsShowingDetailsTrack() const
{
	if (ProgressView.IsValid())
	{
		return ProgressView->bShowSessionDetails;
	}
	return false;
}

void SBuildView::OnViewProcessorTrack()
{
	if (ProgressView.IsValid())
	{
		ProgressView->bShowProcessorTrack = !ProgressView->bShowProcessorTrack;
		ProgressView->UpdateView(TraceView);
	}
}

bool SBuildView::IsShowingProcessorTrack() const
{
	if (ProgressView.IsValid())
	{
		return ProgressView->bShowProcessorTrack;
	}
	return false;
}

void SBuildView::ShowTraceView() const
{
	TabManager->TryInvokeTab(SBuildProgressName);
}

bool SBuildView::IsTraceViewClosed() const
{
	auto Tab = TabManager->FindExistingLiveTab(SBuildProgressName);
	if (!Tab.IsValid())
	{
		return true;
	}
	return !(Tab->GetVisibility() == EVisibility::Visible);
}

void SBuildView::ShowOutputView() const
{
	TabManager->TryInvokeTab(SBuildOutputName);
}

bool SBuildView::IsOutputViewClosed() const
{
	auto Tab = TabManager->FindExistingLiveTab(SBuildOutputName);
	if (!Tab.IsValid())
	{
		return true;
	}
	return !(Tab->GetVisibility() == EVisibility::Visible);
}

void SBuildView::ConstructWidgets()
{
	if (!ProgressView.IsValid())
	{
		ProgressView = SNew(SBuildProgressView)
		.OnActionEventClicked_Lambda([this](const FName InTrackName)
		{
			if (InTrackName.IsValid())
			{
				if (IsOutputViewClosed())
				{
					ShowOutputView();
				}
				OutputView->FocusMessage(InTrackName);
			}
		})
		.bRealtime(&bRealtime);
	}

	if (!OutputView.IsValid())
	{
		OutputView = SNew(SBuildOutputView);
	}
}

TSharedRef<SDockTab> SBuildView::OnSpawnTab_ProgressView(const FSpawnTabArgs& InTab) const
{
	const TSharedRef<SDockTab> DockTab = SNew(SDockTab)
	.ShouldAutosize(false)
	.TabRole(MajorTab)
	[
		ProgressView.ToSharedRef()
	];
	return DockTab;
}

TSharedRef<SDockTab> SBuildView::OnSpawnTab_OutputView(const FSpawnTabArgs& InTab) const
{
	const TSharedRef<SDockTab> DockTab = SNew(SDockTab)
	.ShouldAutosize(false)
	.TabRole(PanelTab)
	[
		OutputView.ToSharedRef()
	];
	return DockTab;
}

#undef LOCTEXT_NAMESPACE