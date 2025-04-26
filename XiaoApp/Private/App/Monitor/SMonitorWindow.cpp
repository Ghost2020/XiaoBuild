/**
  * @author cxx2020@outlook.com
  * @date 11:02 PM
 */
#include "SMonitorWindow.h"
#include "XiaoStyle.h"
#include "Misc/CommandLine.h"
#include "Views/SBuildView.h"
#include "Views/SLocalHistoryView.h"
#include "Views/SNetworkView.h"

#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Async/TaskGraphInterfaces.h"

#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Layout/SWidgetSwitcher.h"
#include "SSimpleButton.h"
#include "SlateOptMacros.h"

#define LOCTEXT_NAMESPACE "MainWindow"


static constexpr int32 GMonitorWindow_Width = 700;
static constexpr int32 GMonitorWindow_Height = 400;

SMonitorWindow::SMonitorWindow()
{
	XiaoIPC::Permissions.set_unrestricted();
}

SMonitorWindow::~SMonitorWindow()
{
}

static bool GetFile(const FDragDropEvent& InDragDropEvent, FString& OutFile)
{
	const TSharedPtr<FExternalDragOperation> DragDropOp = InDragDropEvent.GetOperationAs<FExternalDragOperation>();
	if(DragDropOp.IsValid() && DragDropOp->HasFiles())
	{
		const TArray<FString> Files = DragDropOp->GetFiles();
		if(Files.Num() == 1)
		{
			const FString FileExtension = FPaths::GetExtension(Files[0], true);
			if(FileExtension.EndsWith(TEXT("uba")))
			{
				OutFile = Files[0];
				return true;
			}
		}
	}
	return false;
}

FReply SMonitorWindow::OnDragOver(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent)
{
	if(FString Temp; GetFile(DragDropEvent, Temp))
	{
		return FReply::Handled();
	}
	return SWindow::OnDragOver(MyGeometry, DragDropEvent);
}

FReply SMonitorWindow::OnDrop(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent)
{
	if(FString Temp; GetFile(DragDropEvent, Temp))
	{
		bImportFile = BuildView->StartReadFile(Temp);
		if (bImportFile)
		{
			ViewSwitcher->SetActiveWidget(BuildView.ToSharedRef());
		}
		return bImportFile ? FReply::Handled() : FReply::Unhandled();
	}
	return SWindow::OnDrop(MyGeometry, DragDropEvent);
}

FReply SMonitorWindow::OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent)
{
	if(InKeyEvent.GetKey() == EKeys::F11)
	{
		OnToggleFullScreen();
	}
	return SWindow::OnKeyDown(MyGeometry, InKeyEvent);
}

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SMonitorWindow::Construct(const FArguments& Args)
{
	XIAO_LOG(Log, TEXT("SMonitorWindow::Construct::Begiun"));
	GLog->Flush();
	ConstructWidgets();

	SWindow::Construct(SWindow::FArguments()
		.Title(LOCTEXT("Windows_Title", "构建追踪"))
		.ClientSize(FVector2D(GMonitorWindow_Width * 2, GMonitorWindow_Height * 2))
		[
			SNew(SOverlay)
			+ SOverlay::Slot()
			[
				SNew(SVerticalBox)
#pragma region TopMenu
				+ SVerticalBox::Slot()
				.AutoHeight().VAlign(VAlign_Top)
				[
					MakeMainMenu()
				]
#pragma endregion
#pragma region LeftMenu
				+ SVerticalBox::Slot().VAlign(VAlign_Fill).Padding(10.0f)
				[
					SNew(SSplitter)
					.Orientation(Orient_Horizontal)

					+ SSplitter::Slot()
					.Resizable(false)
					.SizeRule(SSplitter::ESizeRule::SizeToContent)
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot().VAlign(VAlign_Top).AutoHeight().Padding(5.0f)
						[
							SAssignNew(ButtonBuild, SSimpleButton)
							.Icon(FXiaoStyle::Get().GetBrush("Icons.build"))
							.OnClicked_Lambda([this]()
							{
								Update(0);
								return FReply::Handled();
							})
						]
						+ SVerticalBox::Slot().VAlign(VAlign_Top).AutoHeight().Padding(5.0f)
						[
							SAssignNew(ButtonHistory, SSimpleButton).Visibility(BuildView->GetRealtime() ? EVisibility::Collapsed : EVisibility::Visible)
							.Icon(FXiaoStyle::Get().GetBrush("Icons.history"))
							.OnClicked_Lambda([this]()
							{
								Update(1);
								return FReply::Handled();
							})
							.Visibility_Lambda([this]()
							{
								return BuildView->GetRealtime() ? EVisibility::Collapsed : EVisibility::Visible;
							})
						]
						+ SVerticalBox::Slot().VAlign(VAlign_Top).AutoHeight().Padding(5.0f)
						[
							SAssignNew(ButtonNetwork, SSimpleButton).Visibility(BuildView->GetRealtime() ? EVisibility::Collapsed : EVisibility::Visible)
							.Icon(FXiaoStyle::Get().GetBrush("Icons.network"))
							.OnClicked_Lambda([this]()
							{
								Update(2);
								return FReply::Handled();
							})
							.Visibility_Lambda([this]()
							{
								return BuildView->GetRealtime() ? EVisibility::Collapsed : EVisibility::Visible;
							})
						]

						+ SVerticalBox::Slot()
						[
							SNullWidget::NullWidget
						]

						+ SVerticalBox::Slot().VAlign(VAlign_Bottom).AutoHeight().Padding(5.0f)
						[
							SAssignNew(HelpButton, SSimpleButton)
							.Icon(FXiaoStyle::Get().GetBrush("help"))
							.OnClicked_Lambda([this]()
							{
								GetHelp(GLocalization == TEXT("zh-CN") ? TEXT("3.构建实时分析工具") : TEXT("3.Real-time Build Insight Tool"));
								return FReply::Handled();
							})
						]
					]
#pragma endregion
#pragma region View
					+ SSplitter::Slot().SizeRule(SSplitter::ESizeRule::FractionOfParent)
					[
						SAssignNew(ViewSwitcher, SWidgetSwitcher)
						+ SWidgetSwitcher::Slot()[BuildView.ToSharedRef()]
						+ SWidgetSwitcher::Slot()[HistoryView.ToSharedRef()]
						+ SWidgetSwitcher::Slot()[NetworkView.ToSharedRef()]
					]
#pragma endregion
			]

			]
			+ SOverlay::Slot().HAlign(HAlign_Fill).VAlign(VAlign_Fill)
			[
				SAssignNew(MaskBorder, SBorder)
				.BorderBackgroundColor(FColor(0.0f, 0.0f, 0.0f, 125.0f))
				.Visibility(EVisibility::Collapsed)
			]
		]
	);

	OnWindowClosed.BindRaw(this, &SMonitorWindow::OnExit);

	// 加载外部文件
	FString File = TEXT("");
	FString HostIp = TEXT("127.0.0.1");
	int32 ListenPort = SOriginalAgentSettings.UbaScheduler.Port;
	const FString CommandLine = FCommandLine::Get();
	FString TraceFile;
	if (FParse::Value(*CommandLine, TEXT("-file="), File))
	{
		File = File.Replace(TEXT("\""), TEXT(""));
		FPaths::NormalizeFilename(File);
		if (FPaths::FileExists(File))
		{
			bImportFile = BuildView->StartReadFile(File);
		}
	}
	else if (FParse::Value(*CommandLine, TEXT("-host="), HostIp))	
	{
		HostIp.ReplaceInline(TEXT("\""), TEXT(""));
		FParse::Value(*CommandLine, TEXT("-port="), ListenPort);
		bImportFile = BuildView->StartReadNetwork(HostIp, ListenPort);
	}
	else if (FParse::Value(*CommandLine, TEXT("-named="), TraceFile))
	{
		bImportFile = BuildView->StartReadNamed(TraceFile);
	}

	if (bImportFile)
	{
		ViewSwitcher->SetActiveWidget(BuildView.ToSharedRef());
		return;
	}
	
	if (FParse::Param(*CommandLine, TEXT("network")))
	{
		XIAO_LOG(Log, TEXT("UpdateNetwork"));
		ViewSwitcher->SetActiveWidget(NetworkView.ToSharedRef());
		NetworkView->UpdateNetwork(true);
		SetTitle(LOCTEXT("Network_Title", "网络测试"));
	}
	else if (FParse::Param(*CommandLine, TEXT("history")) || (!BuildView->GetRealtime() && !bImportFile))
	{
		XIAO_LOG(Log, TEXT("UpdateHistory"));
		ViewSwitcher->SetActiveWidget(HistoryView.ToSharedRef());
		HistoryView->UpdateHistory();
		SetTitle(LOCTEXT("History_Title", "历史记录"));
	}
	else if (BuildView->GetRealtime())
	{
		XIAO_LOG(Log, TEXT("UpdateTrace"));
		ViewSwitcher->SetActiveWidget(BuildView.ToSharedRef());
		SetTitle(LOCTEXT("Build_Title", "构建追踪"));
	}

	XIAO_LOG(Log, TEXT("SMonitorWindow::Construct::Finish"));
	GLog->Flush();
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SMonitorWindow::ConstructWidgets()
{
	GMonitorSettings.Load();
	
	if(!BuildView.IsValid())
	{
		BuildView = SNew(SBuildView)
		.OwnerWindow(SharedThis(this));
	}
	if(!HistoryView.IsValid())
	{
		HistoryView = SNew(SLocalHistoryView)
		.OnItemDoubleClick_Lambda([this](const TWeakPtr<FBuildHistoryDesc> InDesc)
		{
			const FString MonPath = InDesc.Pin()->FilePath;
			if (FPaths::FileExists(MonPath))
			{
				bImportFile = BuildView->StartReadFile(MonPath);
				if(bImportFile)
				{
					ViewSwitcher->SetActiveWidget(BuildView.ToSharedRef());
				}
			}
		});
	}
	if(!NetworkView)
	{
		NetworkView = SNew(SNetworkView);
	}
}

void SMonitorWindow::Update(const int InIndex) const
{
	static constexpr FColor ActiveColor(205, 205, 205);

	this->ViewSwitcher->SetActiveWidgetIndex(InIndex);
	auto Widget = this->ViewSwitcher->GetActiveWidget();
	if (Widget == HistoryView)
	{
		HistoryView->UpdateHistory();
	}
	if (Widget == NetworkView)
	{
		NetworkView->UpdateNetwork(true);
	}
}

void SMonitorWindow::OnExit(const TSharedRef<SWindow>& InWindow) const
{
	OnExit();
}

TSharedRef<SWidget> SMonitorWindow::MakeMainMenu()
{
	FMenuBarBuilder MenuBarBuilder = FMenuBarBuilder(nullptr);
	MenuBarBuilder.AddPullDownMenu(
		LOCTEXT("FileMenu", "文件"),
		FText::GetEmpty(),
		FNewMenuDelegate::CreateRaw(this, &SMonitorWindow::FillFileMenu)
	);

	MenuBarBuilder.AddPullDownMenu(
		LOCTEXT("View_Text","视图"),
		FText::GetEmpty(),
		FNewMenuDelegate::CreateRaw(this, &SMonitorWindow::FillViewMenu)
	);

	MenuBarBuilder.AddMenuEntry(
		LOCTEXT("About", "关于"),
		LOCTEXT("About_ToolTip", "显示软件许可以以及相关信息"),
		FSlateIcon(FXiaoStyle::Get().GetStyleSetName(), "About"),
		FUIAction(FExecuteAction::CreateLambda([]() { RunXiaoApp(XiaoAppName::SBuildApp, FString::Printf(TEXT("-app=%s"), *XiaoAppName::SBuildAbout), false, true, false, true); })),
		NAME_None,
		EUserInterfaceActionType::Button
	);
	return MenuBarBuilder.MakeWidget();
}

void SMonitorWindow::FillFileMenu(FMenuBuilder& MenuBuilder)
{
	MenuBuilder.AddMenuEntry(
		LOCTEXT("Open", "打开构建..."),
		LOCTEXT("Open_ToolTip", "选择构建文件，显示构建的过程."),
		FSlateIcon(FXiaoStyle::Get().GetStyleSetName(), "LoadBuild"),
		FUIAction(
			FExecuteAction::CreateSP(this, &SMonitorWindow::OnLoadBuildFile)
		),
		NAME_None,
		EUserInterfaceActionType::Button
	);

	MenuBuilder.AddSeparator();
	
	MenuBuilder.AddMenuEntry(
		LOCTEXT("Exit", "退出"),
		LOCTEXT("Exitp_ToolTip", "退出当前程序"),
		FSlateIcon(FXiaoStyle::Get().GetStyleSetName(), "Exit"),
		FUIAction(
			FExecuteAction::CreateSP(this, &SMonitorWindow::OnExit)
		),
		NAME_None,
		EUserInterfaceActionType::Button
	);

	MenuBuilder.SetSearchable(false);
}

void SMonitorWindow::FillViewMenu(FMenuBuilder& MenuBuilder)
{
	MenuBuilder.AddMenuEntry(
		LOCTEXT("OpenTrace_Text", "打开构建追踪Tab"),
		LOCTEXT("OpenTrace_ToolTip", "显示构建的过程."),
		FSlateIcon(FXiaoStyle::Get().GetStyleSetName(), "Icons.Trace"),
		FUIAction(
			FExecuteAction::CreateSP(BuildView.ToSharedRef(), &SBuildView::ShowTraceView),
			FCanExecuteAction::CreateSP(BuildView.ToSharedRef(), &SBuildView::IsTraceViewClosed),
			FGetActionCheckState(),
			FIsActionButtonVisible::CreateSP(this, &SMonitorWindow::GetCanSeeViewButton)
		),
		NAME_None,
		EUserInterfaceActionType::Button
	);

	MenuBuilder.AddMenuEntry(
		LOCTEXT("OpenOutput_Text", "打开输出目录Tab"),
		LOCTEXT("OpenOutput_ToolTip", "显示构建输出的信息."),
		FSlateIcon(FXiaoStyle::Get().GetStyleSetName(), "Icons.Output"),
		FUIAction(
			FExecuteAction::CreateSP(BuildView.ToSharedRef(), &SBuildView::ShowOutputView),
			FCanExecuteAction::CreateSP(BuildView.ToSharedRef(), &SBuildView::IsOutputViewClosed),
			FGetActionCheckState(),
			FIsActionButtonVisible::CreateSP(this, &SMonitorWindow::GetCanSeeViewButton)
		),
		NAME_None,
		EUserInterfaceActionType::Button
	);

	MenuBuilder.SetSearchable(false);
}

void SMonitorWindow::OnLoadBuildFile()
{
	FSlateApplication::Get().CloseToolTip();
	TArray<FString> OutFiles;
	const FText SelectFile = LOCTEXT("LoadBuild_FileDesc", "打开UbaTrace...");
	const FText FileTypes = LOCTEXT("LoadBuild_FileFilter", "ubaTrace(*.uba)|*.uba|All files (*.*)|*.*");
	if(OpenFileDialog(SelectFile.ToString(), TEXT(""), FileTypes.ToString(), OutFiles))
	{
		bImportFile = BuildView->StartReadFile(OutFiles[0]);
		if (bImportFile)
		{
			ViewSwitcher->SetActiveWidget(BuildView.ToSharedRef());
		}
	}	
}

void SMonitorWindow::OnExit() const
{
	RequestEngineExit(TEXT("XiaoBuildMonitor Closed"));
}

void SMonitorWindow::OnToggleFullScreen()
{
	bFullScreen = !bFullScreen;
	const TSharedPtr<FGenericWindow> NativeWindow = GetNativeWindow();
	if (!NativeWindow.IsValid())
	{
		return;
	}
	if (bFullScreen && !IsWindowMaximized())
	{
		NativeWindow->Maximize();
	}
	else if (!bFullScreen && IsWindowMaximized())
	{
		NativeWindow->Restore();
	}
}

void SMonitorWindow::SetLockedState(const bool InbLock) const
{
	MaskBorder->SetVisibility(InbLock ? EVisibility::Visible : EVisibility::Collapsed);
}

EVisibility SMonitorWindow::GetButtonVisibility() const
{
	return (BuildView->GetRealtime() || bImportFile) ? EVisibility::Visible : EVisibility::Collapsed;
}

bool SMonitorWindow::GetCanSeeViewButton() const
{
	return ViewSwitcher->GetActiveWidget() == BuildView;
}

#undef LOCTEXT_NAMESPACE
