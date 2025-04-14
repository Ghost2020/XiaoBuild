/**
  * @author cxx2020@outlook.com
  * @date 10:49 AM
 */
#include "SAgentGeneralView.h"

#include "ShareDefine.h"
#include "SlateOptMacros.h"
#include "Widgets/Layout/SExpandableArea.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SNumericEntryBox.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/STextComboBox.h"
#include "Widgets/Input/SComboBox.h"

#include "Dialogs/SProgressWindow.h"

#include "XiaoAgent.h"
#include "XiaoStyle.h"
#include "XiaoShareField.h"

#define LOCTEXT_NAMESPACE "AgentGeneral"

namespace 
{
	static const FName SIDIndex("Index");
	static const FName SIDType("Type");
	static const FName SIDPath("Path");
	static const FName SIDUBTFlag("UBTFlag");
	static const FName SIDPluginFlag("PluginFlag");

	static const FText SUnrealEditorRuning = LOCTEXT("DetectUnrealEditor_Text", "检测到有Unreal编辑器正在运行，操作前请先关闭");

	static int SIndex = 0;
}

class SComponentListRow final : public SMultiColumnTableRow<TSharedPtr<FInstallFolder>>
{
public:
	SLATE_BEGIN_ARGS(SComponentListRow) {}
		SLATE_ARGUMENT(TSharedPtr<FInstallFolder>, FolderDesc)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InOwnerTableView)
	{
		check(InArgs._FolderDesc.IsValid());
		FolderDesc = InArgs._FolderDesc;

		SMultiColumnTableRow::Construct(FSuperRowType::FArguments(), InOwnerTableView);
	}

	virtual TSharedRef<SWidget> GenerateWidgetForColumn(const FName& InColumnName) override
	{
		if (FolderDesc.IsValid())
		{
			if (const auto Folder = FolderDesc.Pin())
			{
				if (InColumnName == SIDIndex)
				{
					return V_CENTER_WIGET(SNew(STextBlock)
						.Text(FText::FromString(FString::Printf(TEXT("%ld"), ++SIndex))));
				}
				if (InColumnName == SIDType)
				{
					return V_CENTER_WIGET(SNew(STextBlock)
						.Text(Folder->Type ? LOCTEXT("Source_Text", "源码引擎") : LOCTEXT("NotSource_Text", "非源码引擎")));
				}
				if (InColumnName == SIDPath)
				{
					return SNew(STextBlock)
						.Text(FText::FromString(Folder->Folder));
				}
				if (InColumnName == SIDUBTFlag)
				{
					return SNew(SHorizontalBox)
						+ SHorizontalBox::Slot().HAlign(HAlign_Center).VAlign(VAlign_Center)
						[
							SNew(SCheckBox)
								.IsChecked(Folder->bInstall ? ECheckBoxState::Checked : ECheckBoxState::Unchecked)
								.OnCheckStateChanged_Lambda([this](ECheckBoxState InState)
								{
									if (FolderDesc.IsValid())
									{
										FolderDesc.Pin()->bInstall = true ? InState == ECheckBoxState::Checked : false;
										if (FolderDesc.Pin()->bInstall)
										{
											FolderDesc.Pin()->bPluginInstall = true;
										}
									}
								})
						];
				}
				if (InColumnName == SIDPluginFlag)
				{
					return SNew(SHorizontalBox)
						+ SHorizontalBox::Slot().HAlign(HAlign_Center).VAlign(VAlign_Center)
						[
							SNew(SCheckBox)
								.IsChecked(Folder->bPluginInstall ? ECheckBoxState::Checked : ECheckBoxState::Unchecked)
								.IsEnabled_Lambda([this]() 
								{
									return FolderDesc.IsValid() ? FolderDesc.Pin()->bInstall : false;	
								})
								.OnCheckStateChanged_Lambda([this](ECheckBoxState InState)
								{
									if (FolderDesc.IsValid())
									{
										FolderDesc.Pin()->bPluginInstall = true ? InState == ECheckBoxState::Checked : false;
									}
								})
						];
				}
			}
		}
		return SNullWidget::NullWidget;
	}

private:
	TWeakPtr<FInstallFolder> FolderDesc = nullptr;
};


static void GetDiff(const TArray<TSharedPtr<FInstallFolder>>& InL, const TArray<TSharedPtr<FInstallFolder>>& InR, TArray<TSharedPtr<FInstallFolder>>& OutDiffs)
{
	for (int Index = 0; Index < InL.Num(); ++Index)
	{
		if (InL[Index]->bInstall != InR[Index]->bInstall || InL[Index]->bPluginInstall != InR[Index]->bPluginInstall)
		{
			OutDiffs.Add(InL[Index]);
		}
	}
}

static void AssiganArray(const TArray<TSharedPtr<FInstallFolder>>& InL, TArray<TSharedPtr<FInstallFolder>>& OutR)
{
	for (int Index = 0; Index < OutR.Num(); ++Index)
	{
		OutR[Index]->bInstall = InL[Index]->bInstall;
		OutR[Index]->bPluginInstall = InL[Index]->bPluginInstall;
	}
}

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SAgentGeneralView::Construct(const FArguments& InArgs)
{
	uint32 SelectLocIndex = 0;
	bool Got = false;
	for (const auto& Iter : GLauguage2Loc)
	{
		LauguageOptionList.Add(MakeShared<FString>(Iter.Key));
		if (Iter.Value == SOriginalAgentSettings.Localization)
		{
			Got = true;
		}
		if (!Got)
		{
			++SelectLocIndex;
		}
	}
	if (!Got)
	{
		SelectLocIndex = 0;
	}
	const auto& SelectLoc = LauguageOptionList[SelectLocIndex];

	Update();
	
	ChildSlot
	[
		SNew(SBorder)
		.Padding(5.0)
		[
			SNew(SScrollBox).Orientation(Orient_Vertical)
			+ SScrollBox::Slot()
			[
				SNew(SVerticalBox)

				+ SVerticalBox::Slot().FIR_PADDING
				.VAlign(VAlign_Top)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.HAlign(HAlign_Left)
					[
						SNew(STextBlock)
							.Text(LOCTEXT("SystemSwich_Text", "系统服务"))
					]
				]

				+ SVerticalBox::Slot().SEC_PADDING
				[
					SAssignNew(XiaoBuildCheckBox, SCheckBox)
						.IsChecked_Lambda([]() {
						return SModifiedAgentSettings.bEnableUbac ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
							})
						.OnCheckStateChanged_Lambda([this](const ECheckBoxState InState) {
							if (InState == ECheckBoxState::Unchecked)
							{
								if (IsUnrealEditorRuning())
								{
									FXiaoStyle::DoModel(SUnrealEditorRuning, true);
									return;
								}
								if (FXiaoStyle::DoModel(LOCTEXT("ToggleUbac_Text", "是否取消UBAC系统的使用?")))
								{
									SModifiedAgentSettings.bEnableUbac = false;
								}
								else
								{
									XiaoBuildCheckBox->SetIsChecked(ECheckBoxState::Checked);
									return;
								}
							}
							else
							{
								SModifiedAgentSettings.bEnableUbac = true;
							}
							InstallUBT(SModifiedAgentSettings.bEnableUbac);
							Update(true);
						}
						).Content()
						[
							SNew(STextBlock)
							.Text(LOCTEXT("ToggleeXiaoBuild_Text", "是否使用UBAC系统"))
							.ToolTipText(LOCTEXT("ToggleXiaoBuild_ToolTip", "是否使用UBA, 关闭则默认使用XGE进行联合编译"))
						]
				]

				+ SVerticalBox::Slot().SEC_PADDING
				[
					SAssignNew(AgentHelperCheckBox, SCheckBox)
						.IsEnabled_Lambda([]() {
							return true;
						})
						.IsChecked_Lambda([]() {
							return SModifiedAgentSettings.UbaAgent.bEnableAgent ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
						})
						.OnCheckStateChanged_Lambda([this](const ECheckBoxState InState) 
						{
							if (InState == ECheckBoxState::Unchecked)
							{
								if (FXiaoStyle::DoModel(LOCTEXT("ToggleAgentHelp_Text", "是否作为协助者?")))
								{
									SModifiedAgentSettings.UbaAgent.bEnableAgent = false;
								}
								else
								{
									AgentHelperCheckBox->SetIsChecked(ECheckBoxState::Checked);
									return;
								}
							}
							else
							{
								SModifiedAgentSettings.UbaAgent.bEnableAgent = true;
							}
						})
						.Content()
						[
							SNew(STextBlock)
							.Text(LOCTEXT("EnableAsHelper_Text", "是否开启协助"))
							.ToolTipText(LOCTEXT("EnableAsHelper_ToolTip", "是否开启作为协助机器，在网络中有联合编译需求时，作为协助机器"))
						]
				]

				+ SVerticalBox::Slot().SEC_PADDING
				[
					SNew(SHorizontalBox)
					+SHorizontalBox::Slot().AutoWidth().HAlign(HAlign_Left).VAlign(VAlign_Center)
					[
						SNew(STextBlock)
						.Text(LOCTEXT("Laugurae_Text", "本地化显示"))
					]
					+SHorizontalBox::Slot().AutoWidth().HAlign(HAlign_Left)
					[
						SNew(SComboBox<TSharedPtr<FString>>)
						.OptionsSource(&LauguageOptionList)
						.InitiallySelectedItem(SelectLoc)
						.HasDownArrow(true)
						.OnGenerateWidget_Lambda([](const TSharedPtr<FString>& InItem)
						{
							return SNew(STextBlock).Text(FText::FromString(*InItem));
						})
						[
							SAssignNew(LauguageText, STextBlock).Text(FText::FromString(*SelectLoc))
						]
						.OnSelectionChanged_Lambda([this](const TSharedPtr<FString>& InItem, ESelectInfo::Type InType)
						{
							this->LauguageText->SetText(FText::FromString(*InItem));
							SModifiedAgentSettings.Localization = GLauguage2Loc[*InItem];
							FInternationalization::Get().SetCurrentCulture(SModifiedAgentSettings.Localization);
						})
					]
				]
				+ SVerticalBox::Slot().FIR_PADDING
				.VAlign(VAlign_Top)
				[
					SNew(SHorizontalBox)
					+SHorizontalBox::Slot()
					.HAlign(HAlign_Left)
					[
						SNew(STextBlock)
						.Text(LOCTEXT("AgentService_Text", "代理服务"))
					]
				]
					
				+ SVerticalBox::Slot().SEC_PADDING
				[
					SNew(SHorizontalBox)
					+SHorizontalBox::Slot()
					[
						SNew(STextBlock)
						.Text(LOCTEXT("ServiceState_Text", "当前任务处于"))
					]
					+SHorizontalBox::Slot()
					[
						SAssignNew(AgentStateText, STextBlock)
						.Text_Raw(this, &SAgentGeneralView::OnServiceWorkState)
					]
				]

				+ SVerticalBox::Slot().SEC_PADDING
				[
					SNew(SHorizontalBox)
					+SHorizontalBox::Slot()
					[
						SNew(SButton)
						.Text(LOCTEXT("StartService_Text", "启动"))
						.IsEnabled_Raw(this, &SAgentGeneralView::OnGetStartEnable)
						.OnPressed_Raw(this, &SAgentGeneralView::OnStartService)
					]
					+SHorizontalBox::Slot()
					[
						SNew(SButton)
						.Text(LOCTEXT("StopService_Text", "停止"))
						.IsEnabled_Raw(this, &SAgentGeneralView::OnGetStopEnable)
						.OnPressed_Raw(this, &SAgentGeneralView::OnStopService)
					]
				]
	
				+ SVerticalBox::Slot().AutoHeight()[H_SEPATATOR]

				+ SVerticalBox::Slot().FIR_PADDING
				.VAlign(VAlign_Top)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.HAlign(HAlign_Left)
					[
						SNew(STextBlock)
						.Text(LOCTEXT("CoreComponent_Text", "核心组件"))
					]
				]

				+ SVerticalBox::Slot().AutoHeight().SEC_PADDING
				[
					SNew(STextBlock)
					.Text(LOCTEXT("UBT_UABC_Text", "UBT组件与UBAC插件"))
					.ToolTipText(LOCTEXT("UBT_UABC_ToolTip", "引擎是否支持源码的联合编译，以及是否支持材质的联合编译(安装UbaCompatibleController插件)"))
				]

#pragma region ComponentTable
				+ SVerticalBox::Slot().THR_PADDING.HAlign(HAlign_Fill).VAlign(VAlign_Fill)
				[
					SNew(SScrollBox)
					.Orientation(Orient_Horizontal)
					+ SScrollBox::Slot()
					[
						SAssignNew(FolderListView, SListView<TSharedPtr<FInstallFolder>>)
						.ListItemsSource(&ModiefyFolderArray)
						.Orientation(Orient_Vertical)
						.SelectionMode(ESelectionMode::Type::Multi)
						.EnableAnimatedScrolling(true)
						// .ItemHeight(50.0f)
						.AllowOverscroll(EAllowOverscroll::Yes)
						.OnGenerateRow_Raw(this, &SAgentGeneralView::OnGenerateRow)
						.HeaderRow(
							SNew(SHeaderRow)
							+ SHeaderRow::Column(SIDIndex)
							.FixedWidth(50.0f)
							.VAlignHeader(VAlign_Center)
							.DefaultLabel(LOCTEXT("Id_Text", " 序号 "))
							.InitialSortMode(EColumnSortMode::Type::Ascending)
							.OnSort_Static(&SAgentGeneralView::OnSortTable)
							.SortMode_Raw(this, &SAgentGeneralView::GetSortModeForColumn, SIDIndex)

							+ SHeaderRow::Column(SIDType)
							.FillSized(100.0f)
							.VAlignHeader(VAlign_Center)
							.DefaultLabel(LOCTEXT("Type_Text", "引擎类型"))
							.DefaultTooltip(LOCTEXT("Type_Tooltip", "引擎类型"))
							.InitialSortMode(EColumnSortMode::Type::Ascending)
							.OnSort_Static(&SAgentGeneralView::OnSortTable)
							.SortMode_Raw(this, &SAgentGeneralView::GetSortModeForColumn, SIDType)

							+ SHeaderRow::Column(SIDPath)
							.FillSized(300.0f)
							.VAlignHeader(VAlign_Center)
							.DefaultLabel(LOCTEXT("EngineRoot_Text", "引擎目录"))
							.DefaultTooltip(LOCTEXT("EngineRoot_Tooltip", "UE引擎目录"))
							.InitialSortMode(EColumnSortMode::Type::Ascending)
							.OnSort_Static(&SAgentGeneralView::OnSortTable)
							.SortMode_Raw(this, &SAgentGeneralView::GetSortModeForColumn, SIDPath)

							+ SHeaderRow::Column(SIDUBTFlag)
							.FillSized(60.0f)
							.VAlignHeader(VAlign_Center)
							.DefaultLabel(LOCTEXT("UBTFlag_Text", "UBT"))
							.DefaultTooltip(LOCTEXT("UBTFlag_Tooltip", "是否安装联合编译"))
							.InitialSortMode(EColumnSortMode::Type::Ascending)
							.OnSort_Static(&SAgentGeneralView::OnSortTable)
							.SortMode_Raw(this, &SAgentGeneralView::GetSortModeForColumn, SIDUBTFlag)

							+ SHeaderRow::Column(SIDPluginFlag)
							.FillSized(60.0f)
							.VAlignHeader(VAlign_Center)
							.DefaultLabel(LOCTEXT("PluginFlag_Text", "Plugin"))
							.DefaultTooltip(LOCTEXT("PluginFlag_Tooltip", "是否安装材质编译分发插件"))
							.InitialSortMode(EColumnSortMode::Type::Ascending)
							.OnSort_Static(&SAgentGeneralView::OnSortTable)
							.SortMode_Raw(this, &SAgentGeneralView::GetSortModeForColumn, SIDPluginFlag)
						)
					]
				]
#pragma endregion

				+ SVerticalBox::Slot().VAlign(VAlign_Bottom)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot().HAlign(HAlign_Fill)
					[
						SNullWidget::NullWidget
					]
					+ SHorizontalBox::Slot().HAlign(HAlign_Right).AutoWidth()
					[
						SNew(SBorder)
						.Visibility_Lambda([this]() { return OnCanCommit() ? EVisibility::Visible : EVisibility::Collapsed; })
						.BorderImage(FXiaoStyle::Get().GetBrush("NoBorder"))
						[
							SNew(SButton).HAlign(HAlign_Center)
							.Text(LOCTEXT("Confirm_Text", "确定"))
							.ToolTipText(LOCTEXT("Confime_ToolTip", "提交当前的修改"))
							.ButtonStyle(FXiaoStyle::Get(), "FlatButton.Warning")
							.TextStyle(FXiaoStyle::Get(), "FlatButton.DefaultTextStyle")
							.OnClicked_Raw(this, &SAgentGeneralView::OnCommit)
						]
					]

					+ SHorizontalBox::Slot().HAlign(HAlign_Right).AutoWidth()
					[
						SNew(SBorder)
						.Visibility_Lambda([this]() { return OnCanCommit() ? EVisibility::Visible : EVisibility::Collapsed; })
						.BorderImage(FXiaoStyle::Get().GetBrush("NoBorder"))
						.VAlign(VAlign_Bottom)
						[
							SNew(SButton).HAlign(HAlign_Center)
							.Text(LOCTEXT("Cancel_Text", "取消"))
							.ToolTipText(LOCTEXT("CancelToolTip_Text", "放弃当前的修改,退回到修改之前"))
							.ButtonStyle(FXiaoStyle::Get(), "FlatButton.Primary")
							.TextStyle(FXiaoStyle::Get(), "FlatButton.DefaultTextStyle")
							.OnClicked_Raw(this, &SAgentGeneralView::OnRevert)
						]
					]
				]
			]
		]
	];

	SetCanTick(true);
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION


void SAgentGeneralView::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	static constexpr float SUpdateNeedTime = 1.0f;
	TotalTime += InDeltaTime;
	if (TotalTime > SUpdateNeedTime)
	{
		TotalTime = 0.0f;
		bAgentServiceState = IsAppRunning(XiaoAppName::SBuildAgentService);
	}
}

void SAgentGeneralView::Update(const bool bInRebuildTable)
{
	OriginlFolderArray.Reset();
	ModiefyFolderArray.Reset();
	GetAllEngineFolder(OriginlFolderArray);
	GetEngineStates(OriginlFolderArray);
	for (const auto& Desc : OriginlFolderArray)
	{
		ModiefyFolderArray.Add(MakeShareable(new FInstallFolder(Desc->Folder, Desc->Type, Desc->bInstall, Desc->bPluginInstall)));
		ModiefyFolderArray.Last()->EngineVersion = Desc->EngineVersion;
	}

	if (bInRebuildTable)
	{
		FolderListView->RebuildList();
	}
}

FText SAgentGeneralView::OnServiceWorkState() const
{
	return bAgentServiceState ? LOCTEXT("Running_Text", "运行中") : LOCTEXT("Stop_Text", "停止");
}

void SAgentGeneralView::OnStartService()
{
	SetServiceState(XiaoAppName::SBuildAgentService, true);
}

bool SAgentGeneralView::OnGetStartEnable() const
{
	return !OnGetStopEnable();
}

void SAgentGeneralView::OnStopService()
{
	SetServiceState(XiaoAppName::SBuildAgentService, false);
}

bool SAgentGeneralView::OnGetStopEnable() const
{
	return IsAppRunning(XiaoAppName::SBuildAgentService);
}

TSharedRef<ITableRow> SAgentGeneralView::OnGenerateRow(const TSharedPtr<FInstallFolder> InDesc, const TSharedRef<STableViewBase>& InTableView) const
{
	return SNew(SComponentListRow, InTableView).FolderDesc(InDesc);
}

void SAgentGeneralView::OnSortTable(const EColumnSortPriority::Type InPriority, const FName& InName, EColumnSortMode::Type InMode)
{
}

EColumnSortMode::Type SAgentGeneralView::GetSortModeForColumn(const FName InColumnId) const
{
	return (InColumnId == ColumnIdToSort) ? ActiveSortMode : EColumnSortMode::None;
}

bool SAgentGeneralView::OnCanCommit() const
{
	for (int Index = 0; Index < ModiefyFolderArray.Num(); ++Index)
	{
		if (ModiefyFolderArray[Index]->bInstall != OriginlFolderArray[Index]->bInstall)
		{
			return true;
		}

		if (ModiefyFolderArray[Index]->bPluginInstall != OriginlFolderArray[Index]->bPluginInstall)
		{
			return true;
		}
	}

	return false;
}

FReply SAgentGeneralView::OnCommit()
{
	if (IsUnrealEditorRuning())
	{
		FXiaoStyle::DoModel(SUnrealEditorRuning, true);
		return FReply::Handled();
	}
	if (FXiaoStyle::DoModel())
	{
		TArray<TSharedPtr<FInstallFolder>> Diffs;
		GetDiff(ModiefyFolderArray, OriginlFolderArray, Diffs);
		if (Diffs.Num() > 0)
		{
			AssiganArray(ModiefyFolderArray, OriginlFolderArray);

			static FText UpdateComponentText = LOCTEXT("UpdateComponent_Text", "更新组件");
			auto Window = SNew(SProgressWindow).AllAmount(Diffs.Num()).TiTile(UpdateComponentText);
			FSlateApplication::Get().AddWindow(Window, true);

			int Index = 0;
			for (const auto& Desc : Diffs)
			{
				Window->EnterProgressFrame(1.0f, FText::FromString(UpdateComponentText.ToString()));
				InstallComponent(*Desc);
			}

			FSlateApplication::Get().DestroyWindowImmediately(Window);
		}
	}
	return FReply::Handled();
}

FReply SAgentGeneralView::OnRevert()
{
	if (FolderListView.IsValid())
	{
		AssiganArray(OriginlFolderArray, ModiefyFolderArray);
		FolderListView->RebuildList();
	}
	return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE
