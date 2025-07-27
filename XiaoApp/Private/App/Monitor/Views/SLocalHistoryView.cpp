/**
  * @author cxx2020@outlook.com
  * @datetime 2024 -8:15 PM
 */
#include "SLocalHistoryView.h"

#include "SlateOptMacros.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Input/SComboBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "XiaoStyle.h"
#include "XiaoShareField.h"
#include "XiaoCompressor.h"
#include "XiaoLog.h"
#include "XiaoAgent.h"
#include "XiaoInstall.h"
#include "../Tracks/UbaTraceReader.h"
#include "../../../ShareDefine.h"
#include "Dialogs/SProgressWindow.h"

#define LOCTEXT_NAMESPACE "SLocalHistoryView"

static const FString STraceDir = 
#if PLATFORM_WINDOWS
FPaths::Combine(FPlatformMisc::GetEnvironmentVariable(TEXT("ProgramData")), TEXT("XiaoBuild/Traces"));
#else
FPlatformMisc::GetEnvironmentVariable(TEXT("~/XiaoBuild/Traces"));
#endif

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SLocalHistoryView::Construct(const FArguments& InArgs)
{
	XIAO_LOG(Log, TEXT("SLocalHistoryView::Construct::Begin"));
	GLog->Flush();

	OnItemDoubleClick = InArgs._OnItemDoubleClick;

	FTextBlockStyle NoResultStyle = XiaoH2TextStyle;
	NoResultStyle.SetColorAndOpacity(XiaoRed);
		
	InitData();

	FAuto Auto;
	Auto.SizeRule = FSizeParam::ESizeRule::SizeRule_StretchContent;
	
	ChildSlot
	[
		SNew(SOverlay)
		+ SOverlay::Slot().HAlign(HAlign_Fill).VAlign(VAlign_Fill)
		[
			SNew(SVerticalBox)
#pragma region Table
			+ SVerticalBox::Slot().HAlign(HAlign_Fill).VAlign(VAlign_Fill).Padding(10.0f, 10.0f, 10.0f, 5.0f)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().HAlign(HAlign_Fill).VAlign(VAlign_Fill)
				[
				/*SNew(SScrollBox)
				.Orientation(Orient_Horizontal)
				+ SScrollBox::Slot().HAlign(HAlign_Fill).VAlign(VAlign_Fill).SizeParam(Auto)
				[*/
					SAssignNew(BuildListView, SListView<TSharedPtr<FBuildHistoryDesc>>)
					.ListItemsSource(&BuildArray)
					.Orientation(Orient_Vertical)
					.SelectionMode(ESelectionMode::Type::Multi)
					.EnableAnimatedScrolling(true)
					// .ItemHeight(50.0f)
					.Cursor(EMouseCursor::Type::Crosshairs)
					.AllowOverscroll(EAllowOverscroll::Yes)
					.OnGenerateRow_Raw(this, &SLocalHistoryView::OnGenerateRow)
					.OnContextMenuOpening_Raw(this, &SLocalHistoryView::OnContextMenu)
					.HeaderRow(
						SNew(SHeaderRow)
						+SHeaderRow::Column(GHistoryColumnIDStatus)
						.FixedWidth(70.0f)
						.HAlignHeader(EHorizontalAlignment::HAlign_Center)
						.VAlignHeader(VAlign_Center)
						.DefaultLabel(LOCTEXT("Status_Text", "构建状态"))
						.InitialSortMode(EColumnSortMode::Type::Ascending)
						.OnSort_Raw(this, &SLocalHistoryView::OnSortTable)
						.SortMode_Raw(this, &SLocalHistoryView::GetSortModeForColumn, GHistoryColumnIDStatus)
						
						+SHeaderRow::Column(GHistoryColumnIDStartTime)
						.FixedWidth(100.0f)
						.HAlignHeader(EHorizontalAlignment::HAlign_Center)
						.VAlignHeader(VAlign_Center)
            		    .DefaultLabel(LOCTEXT("tartTime_Text", "开始时间"))
            		    .DefaultTooltip(LOCTEXT("tartTime_Tooltip", "构建启动时间"))
            		    .InitialSortMode(EColumnSortMode::Type::Ascending)
						.OnSort_Raw(this, &SLocalHistoryView::OnSortTable)
						.SortMode_Raw(this, &SLocalHistoryView::GetSortModeForColumn, GHistoryColumnIDStartTime)
		
            		    +SHeaderRow::Column(GHistoryColumnIDDuration)
						.FillWidth(.18f)
						.HAlignHeader(EHorizontalAlignment::HAlign_Center)
						.VAlignHeader(VAlign_Center)
						.DefaultLabel(LOCTEXT("Duration_Text", "耗时"))
						.DefaultTooltip(LOCTEXT("Duration_Tooltip", "耗时"))
						.InitialSortMode(EColumnSortMode::Type::Ascending)
						.OnSort_Raw(this, &SLocalHistoryView::OnSortTable)
						.SortMode_Raw(this, &SLocalHistoryView::GetSortModeForColumn, GHistoryColumnIDDuration)
						
						+SHeaderRow::Column(GHistoryColumnIDMessage)
						.FillWidth(.18f)
						.HAlignHeader(EHorizontalAlignment::HAlign_Center)
						.VAlignHeader(VAlign_Center)
						.DefaultLabel(LOCTEXT("Information_Text", "重要信息"))
						.InitialSortMode(EColumnSortMode::Type::Ascending)
						.OnSort_Raw(this, &SLocalHistoryView::OnSortTable)
						.SortMode_Raw(this, &SLocalHistoryView::GetSortModeForColumn, GHistoryColumnIDMessage)
		
						+SHeaderRow::Column(GHistoryColumnIDVersion)
						.FillWidth(.18f)
						.HAlignHeader(EHorizontalAlignment::HAlign_Center)
						.VAlignHeader(VAlign_Center)
						.DefaultLabel(LOCTEXT("Version_Text", "版本"))
						.DefaultTooltip(LOCTEXT("Version_Tooltip", "UBA版本"))
						.InitialSortMode(EColumnSortMode::Type::Ascending)
						.OnSort_Raw(this, &SLocalHistoryView::OnSortTable)
						.SortMode_Raw(this, &SLocalHistoryView::GetSortModeForColumn, GHistoryColumnIDVersion)
		
						+SHeaderRow::Column(GHistoryColumnIDType)
						.FillWidth(.18f)
						.HAlignHeader(EHorizontalAlignment::HAlign_Center)
						.VAlignHeader(VAlign_Center)
						.DefaultLabel(LOCTEXT("Type_Text", "项目类型"))
						.DefaultTooltip(LOCTEXT("Type_Tooltip", "项目类型"))
						.InitialSortMode(EColumnSortMode::Type::Ascending)
						.OnSort_Raw(this, &SLocalHistoryView::OnSortTable)
						.SortMode_Raw(this, &SLocalHistoryView::GetSortModeForColumn, GHistoryColumnIDType)
		
						+SHeaderRow::Column(GHistoryColumnIDFilePath)
						.FillWidth(0.5f)
						.HAlignHeader(EHorizontalAlignment::HAlign_Center)
						.VAlignHeader(VAlign_Center)
						.DefaultLabel(LOCTEXT("Solution_Text", "项目路径"))
						.DefaultTooltip(LOCTEXT("Solution_Tooltip", "项目路径"))
						.InitialSortMode(EColumnSortMode::Type::Ascending)
						.OnSort_Raw(this, &SLocalHistoryView::OnSortTable)
						.SortMode_Raw(this, &SLocalHistoryView::GetSortModeForColumn, GHistoryColumnIDFilePath)
					)
				]
				/*]*/
			]	
#pragma endregion
#pragma region Foot
			+ SVerticalBox::Slot().AutoHeight()
			.VAlign(VAlign_Bottom)
			[
				SNew(SHorizontalBox)
				+SHorizontalBox::Slot().HAlign(HAlign_Left)
				[
					SNew(SHorizontalBox)
					+SHorizontalBox::Slot()
					[
						SNew(SHorizontalBox)
						+SHorizontalBox::Slot()
						[
							SNew(STextBlock).Text_Lambda([this] ()
							{
								return FText::FromString(FString::FromInt(BuildArray.Num()));
							})
						]
						+SHorizontalBox::Slot()
						[
							SNew(STextBlock).Text(LOCTEXT("BuildFoot_Text", "次构建"))
						]
					]
	
					+SHorizontalBox::Slot()
					[
						SNew(SHorizontalBox)
						+SHorizontalBox::Slot()
						[
							SNew(STextBlock).Text_Lambda([this]()
							{
								return FText::FromString(","+ FString::FromInt(BuildListView->GetNumItemsSelected()));
							})
						]
						+SHorizontalBox::Slot()
						[
							SNew(STextBlock).Text(LOCTEXT("SelectedFoot_Text", "个选中"))
						]
					]
				]

				+SHorizontalBox::Slot().HAlign(HAlign_Right)
				[
					SNew(SHorizontalBox)
					+SHorizontalBox::Slot()
					[
						SNew(SHorizontalBox)
						+SHorizontalBox::Slot()
						[
							SNew(STextBlock).Text_Lambda([this]()
							{
								return FText::FromString(FString::FromInt(FailureNum));
							})
						]
						+SHorizontalBox::Slot()
						[
							SNew(STextBlock).Text(LOCTEXT("FailedFoot_Text", "次失败"))
						]
					]
	
					+SHorizontalBox::Slot().HAlign(HAlign_Left)
					[
						SNew(SHorizontalBox)
						+SHorizontalBox::Slot()
						[
							SNew(STextBlock).Text_Lambda([this]()
							{
								return FText::FromString(FString::FromInt(CancelNum));
							})
						]
						+SHorizontalBox::Slot()
						[
							SNew(STextBlock).Text(LOCTEXT("CancelFoot_Text", "次取消"))
						]
					]
				]
			]
#pragma endregion
#pragma region ToolTip
			+ SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Fill)
			.Padding(0.0f)
			[
				SNew(SBorder)
				.BorderImage(FXiaoStyle::Get().GetBrush("WhiteBrush"))
				.BorderBackgroundColor(FLinearColor(0.05f, 0.1f, 0.2f, 1.0f))
				.HAlign(HAlign_Center)
				[
					SNew(STextBlock)
					.Margin(FMargin(4.0f, 1.0f, 4.0f, 1.0f))
					.Text(LOCTEXT("NoFileSeclectTooltip_Text", "-- 请选择一个上面表格中的文件进行查看! --"))
					.ToolTipText(LOCTEXT("NoFileSeclectTooltip_ToolTip", "当前环境中没有选中任何文件"))
					.ColorAndOpacity(FLinearColor(1.0f, 0.75f, 0.5f, 1.0f))
					.Visibility_Lambda([this]()
					{
						return (bHasHistory && !bHasData) ? EVisibility::Visible : EVisibility::Collapsed;
					})
				]
			]
#pragma endregion
		]
		+ SOverlay::Slot()
		[
			SNew(SHorizontalBox).Visibility_Lambda([this]()
			{
				return this->bHasHistory ? EVisibility::Collapsed : EVisibility::Visible;
			})
			+ SHorizontalBox::Slot().HAlign(HAlign_Center).VAlign(VAlign_Center)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight().VAlign(VAlign_Center).HAlign(HAlign_Center).Padding(50.0f, 20.0f, 50.0f, 20.0f)
				[
					SNew(SImage).Image(FXiaoStyle::Get().GetBrush("empty_search"))
				]
				+ SVerticalBox::Slot().AutoHeight().VAlign(VAlign_Center).HAlign(HAlign_Center).Padding(50.0f, 20.0f, 50.0f, 20.0f)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("NoHistoryFound_Text", "没有构建历史记录"))
					.ToolTipText(LOCTEXT("NoHistoryFound_ToolTip", "当前构建历史记录的目录不没有文件，可能没未使用过UBAC进行联合编译吧！"))
					.TextStyle(&NoResultStyle)
				]
			]
		]
	];

	ColumnIdToSort = GHistoryColumnIDStartTime;

	XIAO_LOG(Log, TEXT("SLocalHistoryView::Construct::Finish"));
	GLog->Flush();
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION

bool SLocalHistoryView::GetSelectedFiles(TArray<FString>& OutFiles) const
{
	if(BuildListView.IsValid())
	{
		for(const auto& Item : BuildListView->GetSelectedItems())
		{
			if(Item.IsValid())
			{
				if(FPaths::FileExists(Item->FilePath))
				{
					OutFiles.Add(Item->FilePath);
				}
			}
		}

		return OutFiles.Num() > 0 ? true : false;
	}
	return false;
}

void SLocalHistoryView::InitData()
{
	const TSharedRef<SDockTab> DockTab = SNew(SDockTab).TabRole(MajorTab);
	TabManager = FGlobalTabmanager::Get()->NewTabManager(DockTab);
}

TSharedRef<ITableRow> SLocalHistoryView::OnGenerateRow(const TSharedPtr<FBuildHistoryDesc> InDesc, const TSharedRef<STableViewBase>& InTableView) const
{
	return SNew(SHistoryListRow, InTableView)
	.BuildDesc(InDesc)
	.OnItemDoubleClick(this->OnItemDoubleClick);
}

TSharedPtr<SWidget> SLocalHistoryView::OnContextMenu()
{
	const auto Items = this->BuildListView->GetSelectedItems();
	FMenuBuilder MenuBuilder(true, nullptr);
	MenuBuilder.SetSearchable(false);
	if(Items.Num() > 0)
	{
		MenuBuilder.AddMenuEntry(
			LOCTEXT("OpenSelecFile_Text", "查看记录"),
			LOCTEXT("OpenSelecFile_Tooltip", "重新启动一个程序来显示所选记录"),
			FSlateIcon(),
			FUIAction(
				FExecuteAction::CreateLambda([Items]()
				{
					for(const auto& Iter : Items)
					{
						if(Iter.IsValid())
						{
							const FString Params = FString::Printf(TEXT("-app=%s -file=%s"), *XiaoAppName::SBuildMonitor, *Iter->FilePath);
							RunXiaoApp(XiaoAppName::SBuildApp, Params, false, true, false, true);
						}
					}
				})
			)
		);

		MenuBuilder.AddMenuEntry(
			LOCTEXT("DeleteSelectedFile_Text", "删除选中的内容"),
			LOCTEXT("DeleteSelectedFile_Tooltip", "将选中的历史内容删除"),
			FSlateIcon(),
			FUIAction(
				FExecuteAction::CreateLambda([Items, this]()
				{
					for (const auto& Iter : Items)
					{
						if (Iter.IsValid())
						{
							IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
							if (FPaths::FileExists(Iter->FilePath))
							{
								PlatformFile.DeleteFile(*Iter->FilePath);
							}
						}
					}
					UpdateHistory(true);
				})
			)
		);
	}

	return MenuBuilder.MakeWidget();
}

void SLocalHistoryView::OnSortTable(const EColumnSortPriority::Type InPriority, const FName& InName, EColumnSortMode::Type InMode)
{
	static const TArray<FName> ColumnIds =
	{
		GHistoryColumnIDStatus,
		GHistoryColumnIDStartTime,
		GHistoryColumnIDDuration,
		GHistoryColumnIDMessage,
		GHistoryColumnIDAutoRecovery,
		GHistoryColumnIDVersion,
		GHistoryColumnIDType
	};

	ColumnIdToSort = InName;
	ActiveSortMode = InMode;

	TArray<FName> ColumnIdsBySortOrder = {InName};
	for (const FName& Id : ColumnIds)
	{
		if (Id != InName)
		{
			ColumnIdsBySortOrder.Add(Id);
		}
	}

	BuildArray.Sort([ColumnIdsBySortOrder, InMode](const TSharedPtr<FBuildHistoryDesc>& Left, const TSharedPtr<FBuildHistoryDesc>& Right)
	{
		int32 CompareResult = 0;
		for (const FName& ColumnId : ColumnIdsBySortOrder)
		{
			if (ColumnId == GHistoryColumnIDStatus)
			{
				CompareResult = (Left->BuildStatus == Right->BuildStatus) ? 0 : (Left->BuildStatus < Right->BuildStatus ? -1 : 1);
			}
			else if(ColumnId == GHistoryColumnIDStartTime)
			{
				CompareResult = (Left->StartTime == Right->StartTime) ? 0 : (Left->StartTime < Right->StartTime ? -1 : 1);
			}
			else if(ColumnId == GHistoryColumnIDDuration)
			{
				CompareResult = (Left->Duration == Right->Duration) ? 0 : (Left->Duration < Right->Duration ? -1 : 1);
			}
			else if(ColumnId == GHistoryColumnIDMessage)
			{
				CompareResult = (Left->ErrorNum == Right->ErrorNum) ? 0 : (Left->ErrorNum < Right->ErrorNum ? -1 : 1);
			}
			else if(ColumnId == GHistoryColumnIDAutoRecovery)
			{
				CompareResult = (Left->AutoRecoveryNum == Right->AutoRecoveryNum) ? 0 : (Left->AutoRecoveryNum < Right->AutoRecoveryNum ? -1 : 1);
			}
			else if(ColumnId == GHistoryColumnIDType)
			{
				CompareResult = (Left->Type == Right->Type) ? 0 : (Left->Type < Right->Type ? -1 : 1);
			}

			if (CompareResult != 0)
			{
				return (InMode == EColumnSortMode::Ascending) ? (CompareResult < 0) : (CompareResult > 0);
			}
		}
		return InMode == EColumnSortMode::Ascending ? true : false;
	});

	BuildListView->RequestListRefresh();
}

EColumnSortMode::Type SLocalHistoryView::GetSortModeForColumn(const FName InColumnId) const
{
	return (InColumnId == ColumnIdToSort) ? ActiveSortMode : EColumnSortMode::None;
}

void SLocalHistoryView::OnShowBuild() const
{
	if(TArray<FString> Files; GetSelectedFiles(Files))
	{
		for(const auto& File : Files)
		{
			const FString Param = FString::Printf(TEXT("-file=%s"), *File);
			RunXiaoApp(XiaoAppName::SBuildMonitor, Param, false, true, false, true);
		}
	}
}

bool SLocalHistoryView::OnCanShowBuild() const
{
	if(BuildListView.IsValid())
	{
		return BuildListView->GetSelectedItems().Num() > 0 ? true : false;
	}
	return false;
}

void SLocalHistoryView::OnReplayBuild() const
{
	if(TArray<FString> Files; GetSelectedFiles(Files))
	{
		for(const auto& File : Files)
		{
			const FString Param = FString::Printf(TEXT("-app=%s -file=%s, -replay=true"), *XiaoAppName::SBuildMonitor, *File);
			RunXiaoApp(XiaoAppName::SBuildApp, Param, false, true, false, true);
		}
	}
}

void SLocalHistoryView::OnClearHistory()
{
	TArray<FString> UbaFiles;
	IFileManager::Get().FindFiles(UbaFiles, *STraceDir, TEXT(".uba"));
	for(const auto& File : UbaFiles)
	{
		const FString FilePath = FPaths::Combine(STraceDir, File);
		IFileManager::Get().Delete(*FilePath, true, false, false);
	}

	BuildArray.Empty();
	bHasHistory = BuildArray.Num() > 0;
	BuildListView->RequestListRefresh();
}

void SLocalHistoryView::UpdateHistory(const bool bInForce)
{
	static double LastTime;
	if (!bInForce && (FPlatformTime::Seconds() - LastTime) < 300.0f)
	{
		return;
	}
	LastTime = FPlatformTime::Seconds();
	FailureNum = 0;
	CancelNum = 0;

	TArray<FString> UbaFiles;
	IFileManager::Get().FindFiles(UbaFiles, *STraceDir, TEXT(".uba"));
	for (int Index = 0; Index < UbaFiles.Num(); ++Index)
	{
		UbaFiles[Index] = FPaths::ConvertRelativePathToFull(FPaths::Combine(STraceDir, UbaFiles[Index]));
	}

	static FText ParseTraceFile = LOCTEXT("ParseTrace_Text", "解析Trace文件");
	auto Window = SNew(SProgressWindow).AllAmount(UbaFiles.Num()).TiTile(ParseTraceFile);
	FSlateApplication::Get().AddWindow(Window, true);

	TSet<FString> PathSet;
	for(const FString& FilePath : UbaFiles)
	{
		PathSet.Add(FilePath);

		if (!Path2Desc.Contains(FilePath))
		{
			XIAO_LOG(Log, TEXT("TraceReader ReadFile::Begin::%s"), *FilePath);
			GLog->Flush();

			Window->EnterProgressFrame(1.0f, FText::FromString(ParseTraceFile.ToString()));

			Xiao::FTraceView TraceView;
			Xiao::FTraceReader TraceReader;
			TraceReader.ReadFile(TraceView, *FilePath, false);
			auto Desc = MakeShared<FBuildHistoryDesc>(TraceView, FilePath);
			BuildArray.Add(Desc);
			Path2Desc.Add(MakeTuple(FilePath, Desc));
		}
		if (BuildArray.Last()->BuildStatus == -1)
		{
			++FailureNum;
		}

		XIAO_LOG(Log, TEXT("TraceReader ReadFile::Finish::%s"), *FilePath);
		GLog->Flush();
	}

	FSlateApplication::Get().DestroyWindowImmediately(Window);

	TSet<FString> NeedDelete;
	for (const auto& Trace : BuildArray)
	{
		if (!PathSet.Contains(Trace->FilePath))
		{
			NeedDelete.Add(Trace->FilePath);
		}
	}
	for (const FString& Path : NeedDelete)
	{
		BuildArray.Remove(Path2Desc[Path]);
		Path2Desc.Remove(Path);
	}

	OnSortTable(EColumnSortPriority::Type::Primary, GHistoryColumnIDStartTime, EColumnSortMode::Type::Descending);

	bHasHistory = BuildArray.Num() > 0;
}

#undef LOCTEXT_NAMESPACE
