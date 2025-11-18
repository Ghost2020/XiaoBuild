/**
  * @author cxx2020@outlook.com
  * @date 11:02 PM
*/
#include "SAgentView.h"
#include "SlateOptMacros.h"
#include "Misc/Optional.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Input/SComboBox.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Input/SSearchBox.h"
#include "Widgets/Input/SNumericEntryBox.h"
#include "SSimpleButton.h"
#include "../Widgets/AgentListRow.h"
#include "../Widgets/SConstrainBox.h"
#include "../Dialogs/SManageBuildGroups.h"
#include "../../Monitor/Tracks/UbaNetworkTrace.h"
#include "agent.pb.h"
#include "XiaoStyle.h"
#include "XiaoShareRedis.h"
#include "ShareDefine.h"

#include <algorithm>
#include <cctype> 

#define LOCTEXT_NAMESPACE "SAgentView"


namespace 
{
	const FText NotValidAgent = LOCTEXT("NotValidAgent_Text", "代理获取无效!");
	const FText SUpdateAgent = LOCTEXT("UpdateAgent_Text", "更新代理完成!");
	const FText UnknownError = LOCTEXT("UnknownError_Text", "未知错误!");
	const FText SerializeError = LOCTEXT("SerializeError_Text", "序列化错误!");
}


#define ADD_BATCH_CHECKBOX(PREDICATE, VISIBILITY_LAMBDA, DISPLAY_TEXT) \
	MenuBuilder.AddWidget( \
		SNew(SCheckBox).Padding(10) \
		.IsEnabled_Raw(this, &SAgentView::IsEnableEdit) \
		.IsChecked_Lambda([this]() \
		{ \
			return GetSelectedAgentsCheck([](const TSharedPtr<FAgentProto>& InProto) { \
				return InProto.IsValid() ? InProto->PREDICATE() : false; \
			}); \
		}) \
		.OnCheckStateChanged_Lambda([this](const ECheckBoxState InState) \
		{ \
			const auto State = InState == ECheckBoxState::Checked ? true : false; \
			SetSeletecedAgentsParams([State](const TSharedPtr<FAgentProto>& InProto) { \
				if (InProto.IsValid()) { \
					InProto->set_##PREDICATE(State); \
				} \
			}); \
		}) \
		.Visibility_Lambda(VISIBILITY_LAMBDA) \
		.Content() \
		[ \
			SNew(STextBlock).Text(DISPLAY_TEXT) \
		], \
		FText::GetEmpty(), false, true \
	)


#define ADD_NUMEIRX_BOX(DISPLAY_TEXT, MAX_VAL, DEFAULT_DEF_INDEX, PREDICATE) \
	MenuBuilder.AddWidget( \
		SNew(SHorizontalBox) \
		+ SHorizontalBox::Slot().Padding(10).VAlign(VAlign_Center) \
		[ \
			SNew(STextBlock).Text(DISPLAY_TEXT) \
		] \
		+ SHorizontalBox::Slot().Padding(10).FillWidth(0.5) \
		[ \
			SNew(SNumericEntryBox<int32>) \
			.IsEnabled_Raw(this, &SAgentView::IsEnableEdit) \
			.AllowSpin(true) \
			.MinValue(-1).MaxValue(MAX_VAL) \
			.Value_Lambda([this]() \
			{ \
				return GetSelectedAgentsVal(DEFAULT_DEF_INDEX, [](const TSharedPtr<FAgentProto>& InProto) { \
					return InProto.IsValid() ? InProto->PREDICATE() : false; \
				}); \
			}) \
			.OnValueCommitted_Lambda([this](int32 InValue, ETextCommit::Type InType) \
			{ \
				if (InType != ETextCommit::Type::OnCleared && InValue >= 1) \
				{ \
					SetSeletecedAgentsParams([InValue](const TSharedPtr<FAgentProto>& InProto) { \
						if (InProto.IsValid()) \
						{ \
							InProto->set_##PREDICATE(InValue); \
						} \
					}); \
				} \
			}) \
		], \
		FText::GetEmpty(), false, true \
	); \

#define ADD_COMBO_BOX(DISPLAY_TEXT, OPTION_SOURCE, PREDICATE, DEFAULT_DEF_INDEX) \
	MenuBuilder.AddWidget( \
		SNew(SHorizontalBox) \
		+ SHorizontalBox::Slot().Padding(10).VAlign(VAlign_Center) \
		[ \
			SNew(STextBlock).Text(DISPLAY_TEXT) \
		] \
		+ SHorizontalBox::Slot().Padding(10).AutoWidth() \
		[ \
			SNew(SComboBox<TSharedPtr<FText>>) \
			.IsEnabled_Raw(this, &SAgentView::IsEnableEdit) \
			.OptionsSource(&OPTION_SOURCE) \
			.OnGenerateWidget_Lambda([](const TSharedPtr<FText> InText) \
			{ \
				return SNew(STextBlock).Text(InText.IsValid() ? *InText : FText::GetEmpty()); \
			}) \
			.OnSelectionChanged_Lambda([this](const TSharedPtr<FText> InText, ESelectInfo::Type) \
			{ \
				if (InText->IsEmpty()) \
				{ \
					return; \
				} \
				const int32 Priority = OPTION_SOURCE.Find(InText); \
				SetSeletecedAgentsParams([Priority](const TSharedPtr<FAgentProto>& InProto) { \
					if (InProto.IsValid()) \
					{ \
						InProto->set_##PREDICATE(Priority); \
					} \
				}); \
			}) \
			.Content() \
			[ \
				SNew(STextBlock) \
				.Text_Lambda([this]() \
				{ \
					return *OPTION_SOURCE[GetSelectedAgentsVal(DEFAULT_DEF_INDEX, [](const TSharedPtr<FAgentProto>& InProto) { \
						return InProto.IsValid() ? InProto->PREDICATE() : 0; \
					})]; \
				}) \
			] \
		], \
		FText::GetEmpty(), false, true \
	); \


static const FString SAgentTableConfigPath = FPaths::ConvertRelativePathToFull(FPlatformProcess::UserSettingsDir(), TEXT("XiaoBuild/coordi_agent_table.json"));
static const FString SAgentTableColumeName(TEXT("columns"));

SAgentView::SAgentView()
{
}

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SAgentView::Construct(const FArguments& InArgs)
{
	XIAO_LOG(Log, TEXT("SAgentView::Construct::Begin"));
	GLog->Flush();

	OnQueueNotification = InArgs._OnQueueNotification;

	ExternalScrollbar = SNew(SScrollBar)
		.AlwaysShowScrollbar(true)
		.AlwaysShowScrollbarTrack(true)
		.Orientation(EOrientation::Orient_Vertical)
		.Thickness(5.0f)
		.HideWhenNotInUse(false);
	
	InitData();

	FTextBlockStyle NoResultStyle = XiaoH2TextStyle;
	NoResultStyle.SetColorAndOpacity(XiaoRed);
	
	ChildSlot
	[
		SNew(SVerticalBox)
#pragma region TopTool
		+ SVerticalBox::Slot().VAlign(VAlign_Top).AutoHeight().Padding(50.0f, 5.0f)
		[
			SNew(SHorizontalBox).Visibility(GCurrentUser.Role != 2 ? EVisibility::Visible : EVisibility::Collapsed)
	
			+ SHorizontalBox::Slot().AutoWidth().HAlign(HAlign_Left)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().AutoWidth().Padding(5.0)
				[
					SNew(SSimpleButton)
						.Text(LOCTEXT("BuildGroups2_Text", "构建分组"))
						.Icon(FXiaoStyle::Get().GetBrush("Icons.AddGroup"))
					.OnClicked_Lambda([this] ()
					{
						const TSharedRef<SManageBuildGroups> Window = SNew(SManageBuildGroups);
						FSlateApplication::Get().AddModalWindow(Window, SharedThis(this), false);
						
						return FReply::Handled();
					})
				]
				+ SHorizontalBox::Slot().AutoWidth().Padding(5.0).HAlign(HAlign_Left)
				[
					SNew(SComboButton)
					.OnGetMenuContent_Lambda([this]()
					{
						FMenuBuilder MenuBuilder(true, nullptr);
						for(auto& Column : this->TableHeader->GetColumns())
						{
							MenuBuilder.AddMenuEntry(
								Column.DefaultText,
								Column.DefaultTooltip,
								FSlateIcon(),
								FUIAction(
									FExecuteAction::CreateLambda([&Column, this]()
									{
										const bool NewState = !Column.bIsVisible;
										this->TableHeader->SetShowGeneratedColumn(Column.ColumnId, NewState);
										this->OnGenerateTable();
									}),
									FCanExecuteAction(),
									FIsActionChecked::CreateLambda([&Column]()
									{
										return Column.bIsVisible;
									})
								),
								NAME_None,
								EUserInterfaceActionType::ToggleButton
							);
						}
						MenuBuilder.SetSearchable(false);
						return MenuBuilder.MakeWidget();
					})
					.ButtonContent()
					[
						SNew(STextBlock).Text(LOCTEXT("ManageColumns_Text", "管理条目"))
					]
				]

				+SHorizontalBox::Slot().HAlign(HAlign_Left).VAlign(VAlign_Center)
				[
					SNew(SConstrainBox).MinWidth(300.0f).MaxWidth(300.0f)
					[
						SAssignNew(SearchBox, SSearchBox).HintText(LOCTEXT("Search_Text", "搜索"))
						.HintText(LOCTEXT("SearchHint_Text", "当前仅支持\"描述\"和\"登录用户\"搜索"))
						.OnTextCommitted_Raw(this, &SAgentView::OnSearchChange)
						.OnTextChanged_Lambda([this](const FText& InFiler) {
							if (InFiler.IsEmpty())
							{
								OnSearchChange(InFiler, ETextCommit::OnCleared);
							}
						})
					]
				]

				/*+ SHorizontalBox::Slot()
				[
					SNew(SSpacer)
				]

				+ SHorizontalBox::Slot().AutoWidth().Padding(5.0).HAlign(HAlign_Right)
				[
					SNew(SSimpleButton).Icon(FXiaoStyle::Get().GetBrush("Icons.Update"))
					.OnClicked_Lambda([this]()
					{
						OnUpdate(false);
						return FReply::Handled();
					})
				]*/
			]
		]
#pragma endregion
#pragma region TableBody
		+ SVerticalBox::Slot().HAlign(HAlign_Fill).VAlign(VAlign_Fill).Padding(25.0f, 10.0f, 50.0f, 5.0f)
		[
			SNew(SOverlay)
			+SOverlay::Slot().HAlign(HAlign_Fill)
			[
				SNew(SScrollBox)
				.Orientation(Orient_Horizontal)
				+ SScrollBox::Slot().AutoSize()
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot().VAlign(VAlign_Fill).HAlign(HAlign_Fill)
					[
						SAssignNew(AgentsListView, SListView<TSharedPtr<FAgentProto>>)
						.ListItemsSource(&FiltedArray)
						.SelectionMode(ESelectionMode::Multi)
						.ScrollbarVisibility(EVisibility::Visible)
						.EnableAnimatedScrolling(true)
						// .ItemHeight(128.0f)
						.ExternalScrollbar(ExternalScrollbar)
						.AllowOverscroll(EAllowOverscroll::Yes)
						.OnGenerateRow_Raw(this, &SAgentView::OnGenerateRow)
						.OnContextMenuOpening_Raw(this, &SAgentView::OnGenerateContextMenu)
						.OnMouseButtonDoubleClick_Raw(this, &SAgentView::OnMouseDoubleClick)
						.HeaderRow(
							SAssignNew(TableHeader, SHeaderRow)
							+ SHeaderRow::Column(S_ColumnIdIndex).DefaultLabel(LOCTEXT("AgentIndex_Text", "索引")).DefaultTooltip(LOCTEXT("Index_ToopTip", "序号")).FixedWidth(45.0).HAlignHeader(HAlign_Center).VAlignHeader(VAlign_Center)
							+ SHeaderRow::Column(S_ColumnIdInfor).OverflowPolicy(ETextOverflowPolicy::Ellipsis).DefaultLabel(LOCTEXT("Information_Text", "综合信息")).DefaultTooltip(LOCTEXT("Information_ToopTip", "系统上代理机器的综合信息")).FillSized(300.0).InitialSortMode(EColumnSortMode::Ascending).OnSort_Raw(this, &SAgentView::OnTableSort).SortMode_Raw(this, &SAgentView::GetSortModeForColumn, S_ColumnIdInfor).HAlignHeader(HAlign_Center).VAlignHeader(VAlign_Center)
							+ SHeaderRow::Column(S_ColumnIdAvaCpu).DefaultLabel(LOCTEXT("AvaCpu_Text", "CPU%")).DefaultTooltip(LOCTEXT("AvaCpu_ToopTip", "CPU利用率")).FillSized(80.0).InitialSortMode(EColumnSortMode::Ascending).OnSort_Raw(this, &SAgentView::OnTableSort).SortMode_Raw(this, &SAgentView::GetSortModeForColumn, S_ColumnIdAvaCpu).HAlignHeader(HAlign_Center).VAlignHeader(VAlign_Center)
							+ SHeaderRow::Column(S_ColumnIdAvailableMemory).DefaultLabel(LOCTEXT("AvailableMemory_Text", "可用内存")).DefaultTooltip(LOCTEXT("AvailableMemory_ToopTip", "当前系统剩余可利用的物理内存")).FillSized(100.0).InitialSortMode(EColumnSortMode::Ascending).OnSort_Raw(this, &SAgentView::OnTableSort).SortMode_Raw(this, &SAgentView::GetSortModeForColumn, S_ColumnIdAvailableMemory).HAlignHeader(HAlign_Center).VAlignHeader(VAlign_Center)
							+ SHeaderRow::Column(S_ColumnIdFreeDiskSpace).DefaultLabel(LOCTEXT("FreeDiskSpace_Text", "剩余硬盘空间")).DefaultTooltip(LOCTEXT("FreeDiskSpace_ToopTip", "当前系统盘的存储空间使用情况")).FillSized(120.0).InitialSortMode(EColumnSortMode::Ascending).OnSort_Raw(this, &SAgentView::OnTableSort).SortMode_Raw(this, &SAgentView::GetSortModeForColumn, S_ColumnIdFreeDiskSpace).HAlignHeader(HAlign_Center).VAlignHeader(VAlign_Center)
							+ SHeaderRow::Column(S_ColumnIdAvaDisk).Visibility(EVisibility::Collapsed).DefaultLabel(LOCTEXT("AvaDisk_Text", "磁盘%")).DefaultTooltip(LOCTEXT("AvaDisk_ToopTip", "所有物理驱动器的总利用率")).FillSized(80.0).InitialSortMode(EColumnSortMode::Ascending).OnSort_Raw(this, &SAgentView::OnTableSort).SortMode_Raw(this, &SAgentView::GetSortModeForColumn, S_ColumnIdAvaDisk).HAlignHeader(HAlign_Center).VAlignHeader(VAlign_Center)
							+ SHeaderRow::Column(S_ColumnIdAvaNet).Visibility(EVisibility::Collapsed).DefaultLabel(LOCTEXT("AvaNet_Text", "网络%")).DefaultTooltip(LOCTEXT("AvaNet_ToopTip", "当前主要网络上的网络利用率")).FillSized(80.0).InitialSortMode(EColumnSortMode::Ascending).OnSort_Raw(this, &SAgentView::OnTableSort).SortMode_Raw(this, &SAgentView::GetSortModeForColumn, S_ColumnIdAvaNet).HAlignHeader(HAlign_Center).VAlignHeader(VAlign_Center)
							+ SHeaderRow::Column(S_ColumnIdAvaGpu).Visibility(EVisibility::Collapsed).DefaultLabel(LOCTEXT("AvaGpu_Text", "GPU%")).DefaultTooltip(LOCTEXT("AvaGpu_ToopTip", "所有GPU引擎的最高利用率")).FillSized(80.0).InitialSortMode(EColumnSortMode::Ascending).OnSort_Raw(this, &SAgentView::OnTableSort).SortMode_Raw(this, &SAgentView::GetSortModeForColumn, S_ColumnIdAvaGpu).HAlignHeader(HAlign_Center).VAlignHeader(VAlign_Center)
							+ SHeaderRow::Column(S_ColumnIdRegisteredHelperCores).DefaultLabel(LOCTEXT("RegisteredHelperCores_Text", "协助核心数")).DefaultTooltip(LOCTEXT("RegisteredHelperCores_ToopTip", "能够作为协助的Processor数量")).FillSized(100.0).InitialSortMode(EColumnSortMode::Ascending).OnSort_Raw(this, &SAgentView::OnTableSort).SortMode_Raw(this, &SAgentView::GetSortModeForColumn, S_ColumnIdRegisteredHelperCores).HAlignHeader(HAlign_Center).VAlignHeader(VAlign_Center)
							+ SHeaderRow::Column(S_ColumnIdMaxConnectAgentNum).DefaultLabel(LOCTEXT("MaxAgentCon_Text", "最大代理连接数")).DefaultTooltip(LOCTEXT("MaxAgentCon_ToopTip", "作为发起者时最大能够请求的协助者数量")).FillSized(125.0).InitialSortMode(EColumnSortMode::Ascending).OnSort_Raw(this, &SAgentView::OnTableSort).SortMode_Raw(this, &SAgentView::GetSortModeForColumn, S_ColumnIdMaxConnectAgentNum).HAlignHeader(HAlign_Center).VAlignHeader(VAlign_Center)
							+ SHeaderRow::Column(S_ColumnIdMaxProcessors).DefaultLabel(LOCTEXT("MaxProcessors_Text", "最大运行进程数")).DefaultTooltip(LOCTEXT("MaxProcessors_ToopTip", "作为发起者时最大能够调动的进程数量")).FillSized(150.0).InitialSortMode(EColumnSortMode::Ascending).OnSort_Raw(this, &SAgentView::OnTableSort).SortMode_Raw(this, &SAgentView::GetSortModeForColumn, S_ColumnIdMaxProcessors).HAlignHeader(HAlign_Center).VAlignHeader(VAlign_Center)
							+ SHeaderRow::Column(S_ColumnIdMaxLocalProcessors).DefaultLabel(LOCTEXT("LocalMaxProcessors_Text", "最大本地运行Cores")).DefaultTooltip(LOCTEXT("LocalMaxProcessors_ToopTip", "作为发起者时,本地会运行的Processors")).FillSized(150.0).InitialSortMode(EColumnSortMode::Ascending).OnSort_Raw(this, &SAgentView::OnTableSort).SortMode_Raw(this, &SAgentView::GetSortModeForColumn, S_ColumnIdMaxLocalProcessors).HAlignHeader(HAlign_Center).VAlignHeader(VAlign_Center)
							+ SHeaderRow::Column(S_ColumnIdDesc).DefaultLabel(LOCTEXT("AgentDesc_Text", "描述")).DefaultTooltip(LOCTEXT("AgentDesc_ToopTip", "代理机器的描述")).FillSized(150.0).InitialSortMode(EColumnSortMode::Ascending).OnSort_Raw(this, &SAgentView::OnTableSort).SortMode_Raw(this, &SAgentView::GetSortModeForColumn, S_ColumnIdDesc).HAlignHeader(HAlign_Center).VAlignHeader(VAlign_Center)
							+ SHeaderRow::Column(S_ColumnIdGroup).DefaultLabel(LOCTEXT("BuildGroup_Text", "分组")).DefaultTooltip(LOCTEXT("BuildGroup_ToopTip", "代理分组")).FillSized(120.0).InitialSortMode(EColumnSortMode::Ascending).OnSort_Raw(this, &SAgentView::OnTableSort).SortMode_Raw(this, &SAgentView::GetSortModeForColumn, S_ColumnIdGroup).HAlignHeader(HAlign_Center).VAlignHeader(VAlign_Center)
							+ SHeaderRow::Column(S_ColumnIdBuildPriority).DefaultLabel(LOCTEXT("BuildPriority_Text", "构建优先级")).DefaultTooltip(LOCTEXT("BuildPriority_ToopTip", "。。。")).FillSized(120.0).InitialSortMode(EColumnSortMode::Ascending).OnSort_Raw(this, &SAgentView::OnTableSort).SortMode_Raw(this, &SAgentView::GetSortModeForColumn, S_ColumnIdBuildPriority).HAlignHeader(HAlign_Center).VAlignHeader(VAlign_Center)
							+ SHeaderRow::Column(S_ColumnIdAssignmentPriority).DefaultLabel(LOCTEXT("AssignmentPriority_Text", "分配优先级")).DefaultTooltip(LOCTEXT("AssignmentPriority_ToopTip", "作为协助者时被有限分配的等级")).FillSized(120.0).InitialSortMode(EColumnSortMode::Ascending).OnSort_Raw(this, &SAgentView::OnTableSort).SortMode_Raw(this, &SAgentView::GetSortModeForColumn, S_ColumnIdAssignmentPriority).HAlignHeader(HAlign_Center).VAlignHeader(VAlign_Center)
							+ SHeaderRow::Column(S_ColumnIdLogLevel).DefaultLabel(LOCTEXT("LogLevel_Text", "日志等级")).DefaultTooltip(LOCTEXT("LogLevel_ToopTip", "所有程序运行时输出日志的等级")).FillSized(100.0).InitialSortMode(EColumnSortMode::Ascending).OnSort_Raw(this, &SAgentView::OnTableSort).SortMode_Raw(this, &SAgentView::GetSortModeForColumn, S_ColumnIdLogLevel).HAlignHeader(HAlign_Center).VAlignHeader(VAlign_Center)
							+ SHeaderRow::Column(S_ColumnIdBuildCache).DefaultLabel(LOCTEXT("BuildCache_Text", "构建缓存")).DefaultTooltip(LOCTEXT("BuildCache_ToopTip", "。。。")).FillSized(80.0).InitialSortMode(EColumnSortMode::Ascending).OnSort_Raw(this, &SAgentView::OnTableSort).SortMode_Raw(this, &SAgentView::GetSortModeForColumn, S_ColumnIdBuildCache).HAlignHeader(HAlign_Center).VAlignHeader(VAlign_Center)
							+ SHeaderRow::Column(S_ColumnIdUsedHelpersCache).DefaultLabel(LOCTEXT("UsedHelpersCache_Text", "协助缓存")).DefaultTooltip(LOCTEXT("UsedHelpersCache_ToopTip", "。。。")).FillSized(80.0).InitialSortMode(EColumnSortMode::Ascending).OnSort_Raw(this, &SAgentView::OnTableSort).SortMode_Raw(this, &SAgentView::GetSortModeForColumn, S_ColumnIdUsedHelpersCache).HAlignHeader(HAlign_Center).VAlignHeader(VAlign_Center)
							+ SHeaderRow::Column(S_ColumnIdPhysicalCores).DefaultLabel(LOCTEXT("PhysicalCores_Text", "物理核心数量")).DefaultTooltip(LOCTEXT("PhysicalCores_ToopTip", "")).FillSized(100.0).InitialSortMode(EColumnSortMode::Ascending).OnSort_Raw(this, &SAgentView::OnTableSort).SortMode_Raw(this, &SAgentView::GetSortModeForColumn, S_ColumnIdPhysicalCores).HAlignHeader(HAlign_Center).VAlignHeader(VAlign_Center)
							+ SHeaderRow::Column(S_ColumnIdLogicCores).DefaultLabel(LOCTEXT("LogicCores_Text", "逻辑核心数量")).DefaultTooltip(LOCTEXT("LogicCores_ToopTip", "在支持")).FillSized(100.0).InitialSortMode(EColumnSortMode::Ascending).OnSort_Raw(this, &SAgentView::OnTableSort).SortMode_Raw(this, &SAgentView::GetSortModeForColumn, S_ColumnIdLogicCores).HAlignHeader(HAlign_Center).VAlignHeader(VAlign_Center)
							+ SHeaderRow::Column(S_ColumnIdCpuInfo).DefaultLabel(LOCTEXT("CpuInfo_Text", "CPU详情")).FillSized(400.0).InitialSortMode(EColumnSortMode::Ascending).OnSort_Raw(this, &SAgentView::OnTableSort).SortMode_Raw(this, &SAgentView::GetSortModeForColumn, S_ColumnIdCpuInfo).HAlignHeader(HAlign_Center).VAlignHeader(VAlign_Center)					
							+ SHeaderRow::Column(S_ColumnIdCpuArch).DefaultLabel(LOCTEXT("CpuArch_Text", "CPU架构")).FillSized(100.0).InitialSortMode(EColumnSortMode::Ascending).OnSort_Raw(this, &SAgentView::OnTableSort).SortMode_Raw(this, &SAgentView::GetSortModeForColumn, S_ColumnIdCpuArch).HAlignHeader(HAlign_Center).VAlignHeader(VAlign_Center)
							+ SHeaderRow::Column(S_ColumnIdGpuInfo).DefaultLabel(LOCTEXT("GpuArch_Text", "GPU详情")).FillSized(200.0).InitialSortMode(EColumnSortMode::Ascending).OnSort_Raw(this, &SAgentView::OnTableSort).SortMode_Raw(this, &SAgentView::GetSortModeForColumn, S_ColumnIdGpuInfo).HAlignHeader(HAlign_Center).VAlignHeader(VAlign_Center)
							+ SHeaderRow::Column(S_ColumnIdLastConnected).DefaultLabel(LOCTEXT("LastConnected_Text", "上次连接")).DefaultTooltip(LOCTEXT("LastConnected_ToopTip", "最近一次连接的日期")).FillSized(200.0).InitialSortMode(EColumnSortMode::Ascending).OnSort_Raw(this, &SAgentView::OnTableSort).SortMode_Raw(this, &SAgentView::GetSortModeForColumn, S_ColumnIdLastConnected).HAlignHeader(HAlign_Center).VAlignHeader(VAlign_Center)
							+ SHeaderRow::Column(S_ColumnIdLoggedOnUser).DefaultLabel(LOCTEXT("LoggedOnUser_Text", "登录用户")).DefaultTooltip(LOCTEXT("LoggedOnUser_ToopTip", "。。。")).FillSized(200.0).InitialSortMode(EColumnSortMode::Ascending).OnSort_Raw(this, &SAgentView::OnTableSort).SortMode_Raw(this, &SAgentView::GetSortModeForColumn, S_ColumnIdLoggedOnUser).HAlignHeader(HAlign_Center).VAlignHeader(VAlign_Center)						
						    + SHeaderRow::Column(S_ColumnIdMacAddress).DefaultLabel(LOCTEXT("MACAddress_Text", "Mac 地址")).DefaultTooltip(LOCTEXT("MACAddress_ToopTip", "")).FillSized(225.0).HAlignHeader(HAlign_Center).VAlignHeader(VAlign_Center)
							+ SHeaderRow::Column(S_ColumnIdNetwork).DefaultLabel(LOCTEXT("Network_Text", "网络速率")).DefaultTooltip(LOCTEXT("Network_ToopTip", "")).FillSized(225.0).InitialSortMode(EColumnSortMode::Ascending).OnSort_Raw(this, &SAgentView::OnTableSort).SortMode_Raw(this, &SAgentView::GetSortModeForColumn, S_ColumnIdNetwork).HAlignHeader(HAlign_Center).VAlignHeader(VAlign_Center)
							+ SHeaderRow::Column(S_ColumnIdOSSystem).DefaultLabel(LOCTEXT("OSSystem_Text", "操作系统")).DefaultTooltip(LOCTEXT("OSSystem_ToopTip", "")).FillSized(250.0).InitialSortMode(EColumnSortMode::Ascending).OnSort_Raw(this, &SAgentView::OnTableSort).SortMode_Raw(this, &SAgentView::GetSortModeForColumn, S_ColumnIdOSSystem).HAlignHeader(HAlign_Center).VAlignHeader(VAlign_Center)
							+ SHeaderRow::Column(S_ColumnIdRoutingIP).DefaultLabel(LOCTEXT("RoutingIP_Text", "路由IP")).DefaultTooltip(LOCTEXT("RoutingIP_ToopTip", "局域网内ip地址")).FillSized(125.0).InitialSortMode(EColumnSortMode::Ascending).OnSort_Raw(this, &SAgentView::OnTableSort).SortMode_Raw(this, &SAgentView::GetSortModeForColumn, S_ColumnIdRoutingIP).HAlignHeader(HAlign_Center).VAlignHeader(VAlign_Center)
							+ SHeaderRow::Column(S_ColumeIdListenPort).DefaultLabel(LOCTEXT("HelpPort_Text", "代理端口")).DefaultTooltip(LOCTEXT("HelpPort_ToopTip", "代理监听的端口")).FillSized(125.0).InitialSortMode(EColumnSortMode::Ascending).OnSort_Raw(this, &SAgentView::OnTableSort).SortMode_Raw(this, &SAgentView::GetSortModeForColumn, S_ColumeIdListenPort).HAlignHeader(HAlign_Center).VAlignHeader(VAlign_Center)
							+ SHeaderRow::Column(S_ColumnIdPortMappedAddress).DefaultLabel(LOCTEXT("PortMappedAddress_Text", "服务端口映射")).DefaultTooltip(LOCTEXT("PortMappedAddress_ToopTip", "[Ip:Port]代理服务通过端口映射的地址(或者是公网地址)")).FillSized(125.0).InitialSortMode(EColumnSortMode::Ascending).OnSort_Raw(this, &SAgentView::OnTableSort).SortMode_Raw(this, &SAgentView::GetSortModeForColumn, S_ColumnIdPortMappedAddress).HAlignHeader(HAlign_Center).VAlignHeader(VAlign_Center)
						    + SHeaderRow::Column(S_ColumnIdUpDownTime).DefaultLabel(LOCTEXT("UpDownTime_Text", "上/下行时间")).DefaultTooltip(LOCTEXT("UpDownTime_ToopTip", "")).FillSized(150.0).InitialSortMode(EColumnSortMode::Ascending).OnSort_Raw(this, &SAgentView::OnTableSort).SortMode_Raw(this, &SAgentView::GetSortModeForColumn, S_ColumnIdUpDownTime).HAlignHeader(HAlign_Center).VAlignHeader(VAlign_Center)
							+ SHeaderRow::Column(S_ColumnIdVersion).DefaultLabel(LOCTEXT("Version_Text", "版本")).DefaultTooltip(LOCTEXT("Version_ToopTip", "")).FillSized(150.0).InitialSortMode(EColumnSortMode::Ascending).OnSort_Raw(this, &SAgentView::OnTableSort).SortMode_Raw(this, &SAgentView::GetSortModeForColumn, S_ColumnIdVersion).HAlignHeader(HAlign_Center).VAlignHeader(VAlign_Center)
							+ SHeaderRow::Column(S_ColumeIdResetState).DefaultLabel(LOCTEXT("ResetState_Text", "重置状态")).DefaultTooltip(LOCTEXT("ResetState_ToopTip", "在代理的状态处于不正常时尝试重置")).FillSized(50.0).HAlignHeader(HAlign_Center).VAlignHeader(VAlign_Center).InitialSortMode(EColumnSortMode::Ascending)
						)		
					]
					+ SHorizontalBox::Slot().HAlign(HAlign_Right).AutoWidth()
					[
						ExternalScrollbar.ToSharedRef()
					]
				]
			]
			+ SOverlay::Slot()
			[
				SNew(SHorizontalBox).Visibility_Lambda([this]()
				{
					return this->FiltedArray.Num() > 0 ? EVisibility::Collapsed : EVisibility::Visible;
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
						SNew(STextBlock).Text(LOCTEXT("NoResultsFound_Text", "没有符合条件的结果"))
						.TextStyle(&NoResultStyle)
					]
					+ SVerticalBox::Slot().AutoHeight().VAlign(VAlign_Center).HAlign(HAlign_Center).Padding(50.0f, 20.0f, 50.0f, 20.0f)
					[
						SNew(SButton).Text(LOCTEXT("ShowAllAgents_Text", "显示所有代理"))
						.OnPressed_Lambda([this]()
						{
							OnSearchChange(FText::GetEmpty(), ETextCommit::Type::Default);
							SearchBox->SetText(FText::GetEmpty());
						})
					]
				]
			]
		]
#pragma endregion
#pragma region TableFoot
		+ SVerticalBox::Slot().AutoHeight().VAlign(VAlign_Bottom).Padding(10)
		[
			SNew(SHorizontalBox)
			+SHorizontalBox::Slot().HAlign(HAlign_Left).VAlign(VAlign_Center).AutoWidth()
			[
				SNew(SHorizontalBox)
				+SHorizontalBox::Slot().AutoWidth().Padding(20, 0.0f, 10.0f, 0.0f)
				[
					SNew(STextBlock).Text_Lambda([this]() 
					{
						return FText::FromString(LOCTEXT("TotalAgentNum_Text", "代理数量: ").ToString() + FString::FromInt(AgentsArray.Num()));
					}).ToolTipText(LOCTEXT("TotalAgentNum_ToolTipText", "全部注册代理数量"))
				]

				+ SHorizontalBox::Slot().AutoWidth().Padding(10, 0.0f)
				[
					SNew(STextBlock).Text_Lambda([this]()
					{
						return FText::FromString(LOCTEXT("AvailiableAgentNum_Text", "可用数量: ").ToString() + FString::FromInt(AvailableAgentNum));
					}).ToolTipText(LOCTEXT("AvailiableAgentNum_ToolTipText", "全部已注册代理数量(排除了不在线或其他原因)"))
				]
	
				+SHorizontalBox::Slot().AutoWidth().Padding(10, 0.0f)
				[
					SNew(STextBlock).Text_Lambda([this]()
					{
						return FText::FromString(LOCTEXT("SelectAgentNum_Text", "选中数量: ").ToString() + FString::FromInt(AgentsListView->GetSelectedItems().Num()));
					})
				]
			]

			+ SHorizontalBox::Slot()
			[
				SNew(SSpacer)
			]

			+ SHorizontalBox::Slot().AutoWidth().HAlign(HAlign_Center).VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text_Lambda([this]() 
				{
					const FText MaxTroryCores = LOCTEXT("TryMaxCores_Text", "理论MaxCore(s): ");
					const FText MaxTrueCore = LOCTEXT("TrueyMaxCores_Text", "  实际MaxCore(s): ");
					const FString String = MaxTroryCores.ToString() + FString::FromInt(MaxTheoryHelpCore) + MaxTrueCore.ToString() + FString::FromInt(MaxTrueHelpCore);
					return FText::FromString(String);
				})
				.ToolTipText(LOCTEXT("TreoryMaxCores_ToolTip", "系统同一时刻最大能够调动的核心数量"))
			]

			+ SHorizontalBox::Slot()
			[
				SNew(SSpacer)
			]
	
			+ SHorizontalBox::Slot().AutoWidth().HAlign(HAlign_Right).VAlign(VAlign_Center)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().AutoWidth().Padding(5.0).VAlign(VAlign_Center)
				[
					SNew(STextBlock)
						.Text(LOCTEXT("ShowNum_Text", "显示:"))
				]
				+ SHorizontalBox::Slot().AutoWidth().Padding(5.0)
				[
					SNew(SComboBox<TSharedPtr<FString>>)
					.OptionsSource(&ShowOptionList)
					.InitiallySelectedItem(ShowOptionList[0])
					.HasDownArrow(true)
					.Method(EPopupMethod::UseCurrentWindow)
					.OnGenerateWidget_Lambda([](const TSharedPtr<FString>& InItem)
					{
						return SNew(STextBlock).Text(FText::FromString(*InItem));
					})
					[
						SAssignNew(ShowNumText, STextBlock).Text(FText::FromString(*ShowOptionList[0]))
					]
					.OnSelectionChanged_Lambda([&] (const TSharedPtr<FString>& InItem, ESelectInfo::Type InType)
					{
						this->ShowNumText->SetText(FText::FromString(*InItem));
					})
				]
			]
			+ SHorizontalBox::Slot().AutoWidth().HAlign(HAlign_Right)
			[
				SNew(SSimpleButton)
				.OnClicked_Raw(this, &SAgentView::OnTableSave)
				.Icon(FXiaoStyle::Get().GetBrush(TEXT("Icons.download")))
			]
		]
#pragma endregion 
	];

	FString Content;
	if (FFileHelper::LoadFileToString(Content, *SAgentTableConfigPath))
	{
		if (String2Json(Content, AgentConfigJson))
		{
			const TSharedPtr<FJsonObject>* ColumnsJson = nullptr;
			if (AgentConfigJson->TryGetObjectField(SAgentTableColumeName, ColumnsJson))
			{
				if (auto HeaderRow = AgentsListView->GetHeaderRow())
				{
					for (const auto& Iter : (*ColumnsJson)->Values)
					{
						const FName ColumeName(Iter.Key);
						HeaderRow->SetShowGeneratedColumn(ColumeName, Iter.Value->AsBool());
					}
				}
			}
		}
	}
	else
	{
		AgentConfigJson = MakeShared<FJsonObject>();
	}

	XIAO_LOG(Log, TEXT("SAgentView::Construct::Finish"));
	GLog->Flush();
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION

SAgentView::~SAgentView()
{
}

void SAgentView::InitData()
{
	ColumnIdToSort = S_ColumnIdIndex;
	
	TableShowArray.Add(MakeShared<FText>(LOCTEXT("TableShowAll_Text", "显示所有")));
	TableShowArray.Add(MakeShared<FText>(LOCTEXT("TableOnline_Text", "在线")));
	TableShowArray.Add(MakeShared<FText>(LOCTEXT("TableInProcess_Text", "在处理中")));
	TableShowArray.Add(MakeShared<FText>(LOCTEXT("TableSelectedAgents_Text", "选中的代理")));
	TableShowArray.Add(MakeShared<FText>(LOCTEXT("TableIncompatibleVersion_Text", "不兼容的版本")));
	TableShowArray.Add(MakeShared<FText>(LOCTEXT("TableUnlicensedAgents_Text", "没有许可的代理")));

	ShowOptionList.Add(MakeShared<FString>(TEXT("50")));
	ShowOptionList.Add(MakeShared<FString>(TEXT("75")));
	ShowOptionList.Add(MakeShared<FString>(TEXT("100")));

	NetworkTrace = MakeUnique<FNetworkTrace>();
}

void SAgentView::RefreshIndex() const
{
	int32 Index = 0;
	for (auto& Agent : FiltedArray)
	{
		if (Agent.IsValid())
		{
			Agent->set_index(Index++);
		}
	}
}

TSharedRef<ITableRow> SAgentView::OnGenerateRow(const TSharedPtr<FAgentProto> InAgent, const TSharedRef<STableViewBase>& InTableView) const
{
	return SNew(SAgentListRow, InTableView)
		.AgentProto(InAgent)
		.OnAgentChanged_Raw(this, &SAgentView::OnAgentChange);
}

TSharedPtr<SWidget> SAgentView::OnGenerateContextMenu() const
{
	const auto Items = this->AgentsListView->GetSelectedItems();
	if((GCurrentUser.Role != 2) && Items.Num() > 0)
	{
		const auto& Item = Items[0];
		FMenuBuilder MenuBuilder(true, nullptr);
		MenuBuilder.SetSearchable(false);

		const auto AgentState = Item->status();
		if (AgentState == 1)
		{
			MenuBuilder.AddMenuEntry(
				LOCTEXT("OpenInight_Text", "查看构建过程"),
				LOCTEXT("OpenInight_ToolTip", "启动一个程序用以实时查看构建的处理进度过程"),
				FSlateIcon(FXiaoStyle::Get().GetStyleSetName(), TEXT("Icons.Trace")),
				FUIAction(
					FExecuteAction::CreateLambda([this]() {
						const auto _Items = this->AgentsListView->GetSelectedItems();
						if (_Items.Num() > 0)
						{
							this->OnMouseDoubleClick(_Items[0]);
						}
					})
				)
			);

			MenuBuilder.AddMenuEntry(
				LOCTEXT("StopBuild_Text", "停止联合编译"),
				LOCTEXT("StopBuild_ToolTip", "将当前正在进行的联合编译停止"),
				FSlateIcon(FXiaoStyle::Get().GetStyleSetName(), TEXT("Coordi.Quit")),
				FUIAction(
					FExecuteAction::CreateLambda([this]() {
						const auto _Items = this->AgentsListView->GetSelectedItems();
						if (_Items.Num() > 0)
						{
							PostStopBuild(_Items[0]);
						}
					})
				)
			);

			return MenuBuilder.MakeWidget();
		}

		FString HeaderName = UTF8_TO_TCHAR(Item->loginuser().c_str());
		if (Items.Num() > 1)
		{
			HeaderName = LOCTEXT("BatchOperation_Text", "批量操作").ToString();
		}
		MenuBuilder.AddWidget(
			SNew(STextBlock).Text(FText::FromString(HeaderName)),
			FText::GetEmpty(),
			true, 
			false
		);

		if (Items.Num() == 1 && (AgentState > 2))
		{
			MenuBuilder.AddMenuEntry(
				LOCTEXT("DeleteAgent_Text", "删除注册的代理数据"),
				LOCTEXT("DeleteAgent_ToolTip", "对于实时上已经无法连接上的代理可以清除残余数据"),
				FSlateIcon(FXiaoStyle::Get().GetStyleSetName(), TEXT("Icons.Delete")),
				FUIAction(
					FExecuteAction::CreateLambda([this]() {
						const auto _Items = this->AgentsListView->GetSelectedItems();
						if (_Items.Num() > 0)
						{
							if (FXiaoStyle::DoModel(LOCTEXT("AssureDelete_Text", "确定删除选中的代理数据吗?")))
							{
								XiaoRedis::SRedisClient->hdel(XiaoRedis::Hash::SAgentStats, _Items[0]->id());
								TSet<FString> Ids{ UTF8_TO_TCHAR(_Items[0]->id().c_str())};
								DeleteAgents(Ids, true);
							}
						}
						})
				)
			);
		}
		
		MenuBuilder.AddMenuSeparator();

		MenuBuilder.BeginSection("FirstSection");

		ADD_BATCH_CHECKBOX(benableinitator, []() { return EVisibility::Visible; }, LOCTEXT("AllowToInitiator_Text", "是否作为发起者"));
		if (Item->benableinitator())
		{
			ADD_BATCH_CHECKBOX(bfixedinitator, 
				[this]() {
					const auto _Items = this->AgentsListView->GetSelectedItems();
					if (_Items.Num() > 0 && _Items[0].IsValid())
					{
						return _Items[0]->benableinitator() ? EVisibility::Visible : EVisibility::Collapsed;
					}
					return EVisibility::Collapsed;
				},
				LOCTEXT("EnabledAsFixedInitiator_Text", "是否作为独占式发起者"));
		}
		ADD_BATCH_CHECKBOX(benablehelper, []() { return EVisibility::Visible; }, LOCTEXT("EnabledAsHelper_Text", "是否作为协助者"));
		if (Item->benablehelper())
		{
			ADD_BATCH_CHECKBOX(bfixedhelper, 
				[this]() {
					const auto _Items = this->AgentsListView->GetSelectedItems();
					if (_Items.Num() > 0 && _Items[0].IsValid())
					{
						return _Items[0]->benablehelper() ? EVisibility::Visible : EVisibility::Collapsed;
					}
					return EVisibility::Collapsed;
				},
				LOCTEXT("EnabledAsFixedHelper_Text", "是否作为固定式协助者")
			);
		}

		MenuBuilder.AddMenuSeparator();

		ADD_NUMEIRX_BOX(LOCTEXT("HelpCore_Text", "最大协助核心数:"), 8192, -1, helpercore);
		ADD_NUMEIRX_BOX(LOCTEXT("MaxCon_Text", "最大代理连接数:"), 8192, -1, maxcon);
		ADD_NUMEIRX_BOX(LOCTEXT("MaxCpu_Text", "最大调用cpu数:"), 8192, -1, maxcpu);
		ADD_NUMEIRX_BOX(LOCTEXT("LocalMaxCpu_Text", "最大参与核心:"), 8192, -1, localmaxcpu);

		MenuBuilder.AddMenuSeparator();

		MenuBuilder.AddWidget(
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().Padding(10)
			[
				SNew(STextBlock).Text(LOCTEXT("BuildGroup2_Text", "构建分组:"))
			]
			+ SHorizontalBox::Slot().Padding(10).AutoWidth()
			[
				SNew(SComboBox<TSharedPtr<FString>>)
				.IsEnabled_Raw(this, &SAgentView::IsEnableEdit)
				.OptionsSource(&GGroupArray)
				.OnGenerateWidget_Lambda([](const TSharedPtr<FString>& InText)
				{
					return SNew(STextBlock).Text(InText.IsValid() ? FText::FromString(*InText) : FText::GetEmpty());
				})
				.OnSelectionChanged_Lambda([this](const TSharedPtr<FString>& InText, ESelectInfo::Type)
				{
					if (InText->IsEmpty())
					{
						return;
					}
					const std::string Group = TCHAR_TO_UTF8(**InText);
					SetSeletecedAgentsParams([Group](const TSharedPtr<FAgentProto>& InProto) {
						if (InProto.IsValid())
						{
							InProto->set_group(Group);
						}
					});
				})
				.Content()
				[
					SNew(STextBlock)
					.Text_Lambda([this](){
						static const FString DefaultGroup = TEXT("Default");
						static const FString MultiGroup = TEXT("Multi");
						const auto _Items = this->AgentsListView->GetSelectedItems();
						if (_Items.IsEmpty())
						{
							return FText::FromString(TEXT("Unknown"));
						}
						if (_Items.Num() == 1 || !_Items[0].IsValid())
						{
							return FText::FromString(_Items[0].IsValid() ? UTF8_TO_TCHAR(_Items[0]->group().c_str()) : DefaultGroup);
						}

						const std::string Group = _Items[0]->group();
						for (int Index = 1; Index < _Items.Num(); ++Index)
						{
							const auto& _Item = _Items[Index];
							if (_Item.IsValid())
							{
								if (_Item->group() != Group)
								{
									return FText::FromString(MultiGroup);
								}
							}
						}
						return FText::FromString(UTF8_TO_TCHAR(Group.c_str()));
					})
				]
			],
			FText::GetEmpty(), false, true
		);
		ADD_COMBO_BOX(LOCTEXT("BuildPriority2_Text", "构建优先级:"), GPriorityArray, buildpriority, 6);
		ADD_COMBO_BOX(LOCTEXT("AssignPriority_Text", "分配优先级:"), GPriorityArray, allocationpriority, 6);
		ADD_COMBO_BOX(LOCTEXT("LogLevel2_Text", "日志输出等级:"), GLevelArray, loglevel, 5);
        
		MenuBuilder.EndSection();

		return MenuBuilder.MakeWidget();
	}
	return SNullWidget::NullWidget;
}

void SAgentView::OnMouseDoubleClick(const TSharedPtr<FAgentProto> InProto) const
{
	if (InProto.IsValid())
	{
		if (InProto->status() == 1)
		{
			const FString Host = UTF8_TO_TCHAR(InProto->ip().c_str());
			const int32 TracePort = InProto->traceport();
			if (TracePort > 0 && TracePort < 65535)
			{
				const FString Params = FString::Printf(TEXT("-app=%s -host=\"%s\" -port=%d"), *XiaoAppName::SBuildMonitor, *Host, TracePort);
				RunXiaoApp(XiaoAppName::SBuildApp, Params, false, true, false, true);
			}
		}
	}
}

void SAgentView::OnGenerateTable() const
{
	if(const auto Header = AgentsListView->GetHeaderRow())
	{
		Header->RefreshColumns();
		const TSharedPtr<FJsonObject>* ColumnsJson = nullptr;
		if (AgentConfigJson.IsValid())
		{
			if (!AgentConfigJson->HasField(SAgentTableColumeName))
			{
				TSharedPtr<FJsonObject> ColumeFieldObject = MakeShared<FJsonObject>();
				AgentConfigJson->SetObjectField(SAgentTableColumeName, ColumeFieldObject);
			}
			if (AgentConfigJson->TryGetObjectField(SAgentTableColumeName, ColumnsJson))
			{
				for (const auto& Colume : Header->GetColumns())
				{
					(*ColumnsJson)->SetBoolField(Colume.ColumnId.ToString(), Colume.bIsVisible);
				}
				FString Content;
				if (Json2String(AgentConfigJson, Content))
				{
					FFileHelper::SaveStringToFile(Content, *SAgentTableConfigPath);
				}
			}
		}
	}

	OnTableSort(EColumnSortPriority::Type::Primary, ColumnIdToSort, ActiveSortMode);
}

EColumnSortMode::Type SAgentView::GetSortModeForColumn(const FName InColumnId) const
{
	return (InColumnId == ColumnIdToSort) ? ActiveSortMode : EColumnSortMode::None;
}

void SAgentView::OnTableSort(const EColumnSortPriority::Type InSortPriority, const FName& InName, EColumnSortMode::Type InSortMode) const
{
	static const TArray<FName> ColumnIds =
	{
		S_ColumnIdInfor, S_ColumnIdDesc, S_ColumnIdGroup,
		S_ColumnIdAvaCpu, S_ColumnIdAvaNet, S_ColumnIdAvaGpu, 
		S_ColumnIdLastConnected, S_ColumnIdUsedHelpersCache, S_ColumnIdRegisteredHelperCores,
		S_ColumnIdMaxConnectAgentNum, S_ColumnIdMaxProcessors, S_ColumnIdMaxLocalProcessors,
		S_ColumnIdBuildCache, S_ColumnIdBuildPriority, S_ColumnIdCpuInfo, S_ColumnIdCpuArch, 
		S_ColumnIdGpuInfo,
		S_ColumnIdFreeDiskSpace, S_ColumnIdLogLevel, S_ColumnIdLoggedOnUser, S_ColumnIdAssignmentPriority,
		S_ColumnIdAvailableMemory, S_ColumnIdLogicCores, S_ColumnIdNetwork, S_ColumnIdOSSystem,
		S_ColumnIdPhysicalCores, S_ColumnIdRoutingIP, S_ColumeIdListenPort, S_ColumnIdPortMappedAddress, 
		S_ColumnIdUpDownTime, S_ColumnIdVersion
	};

	ColumnIdToSort = InName;
	ActiveSortMode = InSortMode;

	TArray<FName> ColumnIdsBySortOrder = {InName};
	for (const FName& Id : ColumnIds)
	{
		if (Id != InName)
		{
			ColumnIdsBySortOrder.Add(Id);
		}
	}

#define COMPARE(ID, PREDICATE) \
	else if (ColumnId == ID) \
	{ \
		CompareResult = (Left->PREDICATE() == Right->PREDICATE()) ? 0 : ((Left->PREDICATE() < Right->PREDICATE()) ? -1 : 1); \
	}
	
	FiltedArray.Sort([ColumnIdsBySortOrder, InSortMode](const TSharedPtr<FAgentProto>& Left, const TSharedPtr<FAgentProto>& Right)
	{
		int32 CompareResult = 0;
		for (const FName& ColumnId : ColumnIdsBySortOrder)
		{
			if (ColumnId == S_ColumnIdInfor)
			{
				CompareResult = (Left->status() == Right->status()) ? 0 : ((Left->status() < Right->status()) ? -1 : 1);
				if (CompareResult == 0)
				{
					CompareResult = (Left->bfixedinitator() == Right->bfixedinitator()) ? 0 : (Left->bfixedinitator() && !Right->bfixedinitator() ? 1 : -1);
					if (CompareResult == 0)
					{
						CompareResult = (Left->bfixedhelper() == Right->bfixedhelper()) ? 0 : (Left->bfixedhelper() && !Right->bfixedhelper() ? 1 : -1);
					}
				}
			}
			COMPARE(S_ColumnIdDesc, desc)
			COMPARE(S_ColumnIdGroup, group)
			COMPARE(S_ColumnIdAvaCpu, cpuava)
			COMPARE(S_ColumnIdAvaDisk, avadisk)
			COMPARE(S_ColumnIdAvaNet, avalnet)
			COMPARE(S_ColumnIdAvaGpu, avagpu)
			COMPARE(S_ColumnIdLastConnected, lastcon)
			else if (ColumnId == S_ColumnIdUsedHelpersCache)
			{
				const float LeftHelpCache = Left->totalhelpcache() - Left->usehelpcache();
				const float RightHelpCache = Right->totalhelpcache() - Right->usehelpcache();
				CompareResult = (LeftHelpCache == RightHelpCache) ? 0 : (LeftHelpCache < RightHelpCache ? -1 : 1);
			}
			COMPARE(S_ColumnIdRegisteredHelperCores, helpercore)
			COMPARE(S_ColumnIdMaxConnectAgentNum, maxcon)
			COMPARE(S_ColumnIdMaxProcessors, maxcpu)
			COMPARE(S_ColumnIdMaxLocalProcessors, localmaxcpu)
			COMPARE(S_ColumnIdBuildCache, bbuildcache)
			COMPARE(S_ColumnIdBuildPriority, buildpriority)
			COMPARE(S_ColumnIdCpuInfo, cpuinfo)
			COMPARE(S_ColumnIdCpuArch, cpuarch)
			COMPARE(S_ColumnIdGpuInfo, gpudesc)
			else if (ColumnId == S_ColumnIdFreeDiskSpace)
			{
				const float LeftFreeSpace = Left->usehardspace();
				const float RightFreeSpace = Right->usehardspace();
				CompareResult = (LeftFreeSpace == RightFreeSpace) ? 0 : (LeftFreeSpace < RightFreeSpace ? -1 : 1);
			}
			COMPARE(S_ColumnIdLogLevel, loglevel)
			COMPARE(S_ColumnIdLoggedOnUser, loginuser)
			COMPARE(S_ColumnIdAssignmentPriority, allocationpriority)
			else if (ColumnId == S_ColumnIdAvailableMemory)
			{
				const float LeftFreeMem = Left->totalmemory() - Left->usememory();
				const float RightFreeMem = Right->totalmemory() - Right->usememory();
				CompareResult = (LeftFreeMem == RightFreeMem) ? 0 : (LeftFreeMem < RightFreeMem ? -1 : 1);
			}
			COMPARE(S_ColumnIdLogicCores, logiccore)
			COMPARE(S_ColumnIdNetwork, networkspeed)
			COMPARE(S_ColumnIdOSSystem, opsystem)
			COMPARE(S_ColumnIdPhysicalCores, physicalcore)
			COMPARE(S_ColumnIdRoutingIP, routerip)
			COMPARE(S_ColumeIdListenPort, helperport)
			COMPARE(S_ColumnIdPortMappedAddress, portmappedaddress)
			else if (ColumnId == S_ColumnIdUpDownTime)
			{
				// TODO 需要特殊处理
				CompareResult = (Left->updowntime() == Right->updowntime()) ? 0 : (Left->updowntime() < Right->updowntime() ? -1 : 1);
			}
			COMPARE(S_ColumnIdVersion, version)
			if (CompareResult != 0)
			{
				return InSortMode == EColumnSortMode::Ascending ? CompareResult < 0 : CompareResult > 0;
			}
		}
		return InSortMode == EColumnSortMode::Ascending ? true : false;
	});

#undef COMPARE

	RefreshIndex();

	if (AgentsListView.IsValid())
	{
		AgentsListView->RequestListRefresh();
	}
}

FReply SAgentView::OnTableSave() const
{
	IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
	if (!DesktopPlatform)
	{
		XIAO_LOG(Warning, TEXT("DesktopPlatform module not found!"));
		return FReply::Handled();
	}

	FString DefaultFile = TEXT("Agents.csv");	// 默认文件名
	FString FileTypes = TEXT("Data Table CSV (*.csv)|*.csv"); // 可选文件类型
	TArray<FString> OutFilenames; // 保存路径及文件名

	bool bSave = DesktopPlatform->SaveFileDialog(
		nullptr,                          // Parent窗口句柄 (nullptr表示无父窗口)
		TEXT("Save Your File"),           // 对话框标题
		TEXT(""),						  // 默认路径
		DefaultFile,                      // 默认文件名
		FileTypes,                        // 文件类型筛选
		EFileDialogFlags::None,           // 文件对话框标志 (默认无)
		OutFilenames                      // 输出：用户选择的路径
	);

	if (!bSave || OutFilenames.Num() == 0)
	{
		return FReply::Handled();
	}

	FString Content = TEXT("Index,Status,Desc,Group,AvaCPU,HelpCores,MaxCon,MaxCpu,PhyCoreNum,LogicCoreNum,BuildPriority,AssignPriority,LogLevel,MemDetails,HardDiskSpace,BuildCache,CacheDetails,CPUDetails,CPUArch,GPUDesc,LastConnectionTime,LoginUser,MacAddress,NetSpeed,OS,RouterIP,HelperListenPort,UpDownTime\n");
	int32 Index = 0;
	for (const auto& Iter : FiltedArray)
	{
		if (Iter.IsValid())
		{
			Content += FString::Printf(TEXT("%d,%d,\"%s-%s-%s\",%s,%.1f,%d,%d,%d,%d,%d,%s,%s,%s,%.1f/%.1f GB,%.1f/%.1f GB,%s,%.1f/%.1f GB,%s,%s,%s,%s, %s,%s,%f Mbits/sec,%s,%s,%d,\"%s\"\n"),
				Index++,
				Iter->status(),
				UTF8_TO_TCHAR(Iter->desc().c_str()), Iter->bfixedinitator() ? TEXT("固定发起者") : TEXT("浮动发起者"), Iter->bfixedhelper() ? TEXT("固定协助者") : TEXT("浮动协助者"),
				UTF8_TO_TCHAR(Iter->group().c_str()),
				Iter->cpuava(),
				Iter->helpercore(),
				Iter->maxcon(),
				Iter->maxcpu(),
				Iter->physicalcore(),
				Iter->logiccore(),
				*Priority2Text(static_cast<EBuildPriority>(Iter->buildpriority())).ToString(),
				*Priority2Text(static_cast<EBuildPriority>(Iter->allocationpriority())).ToString(),
				*LogLevel2Text(static_cast<ELogLevel>(Iter->loglevel())).ToString(),
				Iter->usememory(), Iter->totalmemory(),
				Iter->usehardspace(), Iter->totalhardspace(),
				Iter->bbuildcache() ? TEXT("Y") : TEXT("N"),
				Iter->usehelpcache(), Iter->totalhelpcache(),
				UTF8_TO_TCHAR(Iter->cpuinfo().c_str()),
				UTF8_TO_TCHAR(Iter->cpuarch().c_str()),
				UTF8_TO_TCHAR(Iter->gpudesc().c_str()),
				UTF8_TO_TCHAR(Iter->lastcon().c_str()),
				UTF8_TO_TCHAR(Iter->loginuser().c_str()),
				UTF8_TO_TCHAR(Iter->macaddress().c_str()),
				Iter->networkspeed(),
				UTF8_TO_TCHAR(Iter->opsystem().c_str()),
				UTF8_TO_TCHAR(Iter->routerip().c_str()),
				Iter->helperport(),
				UTF8_TO_TCHAR(Iter->updowntime().c_str())
			);
		}
	}
	FFileHelper::SaveStringToFile(Content, *OutFilenames[0]);

	return FReply::Handled();
}

int32 SAgentView::GetSelectedAgentsVal(const int32 InDeffIndex, TFunction<int32(const TSharedPtr<FAgentProto>&)> InGetter) const
{
	const auto _Items = this->AgentsListView->GetSelectedItems();
	if (_Items.IsEmpty())
	{
		return 0;
	}
	if (_Items.Num() == 1 || !_Items[0].IsValid())
	{
		return _Items[0].IsValid() ? InGetter(_Items[0]) : 1;
	}

	const int32 Val = InGetter(_Items[0]);
	for (int Index = 1; Index < _Items.Num(); ++Index)
	{
		const auto& _Item = _Items[Index];
		if (_Item.IsValid())
		{
			if (InGetter(_Item) != Val)
			{
				return InDeffIndex;
			}
		}
	}
	return Val;
}

ECheckBoxState SAgentView::GetSelectedAgentsCheck(const TFunction<bool(const TSharedPtr<FAgentProto>&)>& InGetter) const
{
	const auto _Items = this->AgentsListView->GetSelectedItems();
	if (_Items.IsEmpty())
	{
		return ECheckBoxState::Undetermined;
	}
	if (_Items.Num() == 1 || !_Items[0].IsValid())
	{
		return _Items[0].IsValid() ? (InGetter(_Items[0]) ? ECheckBoxState::Checked : ECheckBoxState::Unchecked) : ECheckBoxState::Undetermined;
	}

	const bool bCheck = InGetter(_Items[0]);
	for (int Index = 1; Index < _Items.Num(); ++Index)
	{
		const auto& _Item = _Items[Index];
		if (_Item.IsValid())
		{
			if (InGetter(_Item) != bCheck)
			{
				return ECheckBoxState::Undetermined;
			}
		}
	}
	return bCheck ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}

void SAgentView::SetSeletecedAgentsParams(const TFunction<void(const TSharedPtr<FAgentProto>&)>& InSetter) const
{
	const auto _Items = this->AgentsListView->GetSelectedItems();
	for (auto& Item : _Items)
	{
		if (Item.IsValid())
		{
			InSetter(Item);
		}
	}
	OnAgentsChange(_Items);
}

void SAgentView::OnAgentChange(const TWeakPtr<FAgentProto>& InAgent) const
{
	TArray<TSharedPtr<FAgentProto>> Agents{ InAgent.Pin()};
	OnAgentsChange(Agents);
}

void SAgentView::OnAgentsChange(const TArray<TSharedPtr<FAgentProto>>& InAgents) const
{
	try
	{
		for (auto& Agent : InAgents)
		{
			if (!Agent.IsValid())
			{
				XIAO_LOG(Error, TEXT("Agent is not valid"));
				(void)OnQueueNotification.ExecuteIfBound(-1, NotValidAgent);
				return;
			}

			const std::string AgentId = Agent->id();
			if (AgentId.size() > 0)
			{
				std::string Protobuf;
				if (Agent->SerializeToString(&Protobuf))
				{
					const auto Result = XiaoRedis::SRedisClient->hset(XiaoRedis::Hash::SAgentStats, AgentId, Protobuf);
				}
				else
				{
					(void)OnQueueNotification.ExecuteIfBound(-1, SerializeError);
				}
			}
		}

		OnUpdate(false);
		(void)OnQueueNotification.ExecuteIfBound(0, SUpdateAgent);
		return;
	}
	CATCH_REDIS_EXCEPTRION();
	(void)OnQueueNotification.ExecuteIfBound(-1, FText::FromString(UTF8_TO_TCHAR(SRedisMessage.c_str())));
}

void SAgentView::OnSearchChange(const FText& InFiler, ETextCommit::Type InCommitType)
{
	if (InCommitType != ETextCommit::OnEnter && InCommitType != ETextCommit::OnCleared && InCommitType != ETextCommit::Default)
	{
		return;
	}

	const FString FilterStr = InFiler.ToString();

	FiltedArray.Empty();
	if (InFiler.IsEmpty())
	{
		FiltedArray = AgentsArray;
	}
	else
	{
		for (const auto& Agent : AgentsArray)
		{
			if (Agent.IsValid())
			{
				const FString Desc = UTF8_TO_TCHAR((Agent->desc().empty() ? Agent->loginuser() : Agent->desc()).c_str());
				if (FilterStr.IsEmpty() || Desc.Contains(FilterStr))
				{
					FiltedArray.Add(Agent);
				}
			}
		}
	}

	OnGenerateTable();
}

void SAgentView::DeleteAgents(const TSet<FString>& InAgentIds, const bool bRebuild) const
{
	for (const FString& Id : InAgentIds)
	{
		if (auto Agent = Id2Agent[Id].Pin())
		{
			AgentsArray.Remove(Agent);
			FiltedArray.Remove(Agent);
		}
		Id2Agent.Remove(Id);
	}

	RefreshIndex();

	if (bRebuild)
	{
		OnGenerateTable();
	}
}

void SAgentView::PostStopBuild(const TSharedPtr<FAgentProto>& InProto) const
{
	if (InProto.IsValid())
	{
		const FString Host = UTF8_TO_TCHAR(InProto->ip().c_str());
		const int32 TracePort = InProto->traceport();
		if (NetworkTrace.IsValid())
		{
			// 另外启动一个线程去运行会触发UE的崩溃
			/*static bool bRequesting = false;
			if (!bRequesting)
			{
				bRequesting = true;
				AsyncTask(ENamedThreads::Type::GameThread_Local, [this, Host, TracePort]() {*/
					NetworkTrace->RequestStopInitiator(Host, TracePort);
			/*		bRequesting = false;
				});
			}*/
		}
	}
}

void SAgentView::OnUpdate(const bool bRebuild) const
{
	bool Rebuild = bRebuild;

	if (!XiaoRedis::IsConnected())
	{
		return;
	}

	try
	{
		// 更新频率
		const auto FreqOptional = XiaoRedis::SRedisClient->get(XiaoRedis::String::SAgentUpdateFreqStr);
		if (FreqOptional.has_value())
		{
			GSleepUpdate = std::atof(FreqOptional.value().c_str());
		}

		// 代理信息
		std::unordered_map<std::string, std::string> AgentStats;
		AvailableAgentNum = 0;
		MaxTheoryHelpCore = 0;
		MaxTrueHelpCore = 0;
		XiaoRedis::SRedisClient->hgetall(XiaoRedis::Hash::SAgentStats, std::inserter(AgentStats, AgentStats.begin()));
		TSet<FString> IdSet;
		for (const auto& Iter : AgentStats)
		{
			const FString UniqueId = UTF8_TO_TCHAR(Iter.first.c_str());
			
			if (UniqueId.IsEmpty())
			{
				continue;
			}

			const bool bAlreadyExist = Id2Agent.Contains(UniqueId);
			if (!bAlreadyExist)
			{
				auto AgentProto = MakeShared<FAgentProto>();
				Id2Agent.Add({UniqueId, AgentProto});
				AgentsArray.Add(AgentProto);
				FiltedArray.Add(AgentProto);
				Rebuild = true;
			}

			auto Item = Id2Agent[UniqueId].Pin();
			if (!Item.IsValid())
			{
				continue;
			}
			if (!Item->ParseFromString(Iter.second))
			{
				XIAO_LOG(Error, TEXT("ParseFromString failed::%s"), UTF8_TO_TCHAR(Iter.second.c_str()));
				continue;
			}
			if (Iter.first != Item->macaddress())
			{
				XIAO_LOG(Error, TEXT("Not valid agent data Guid[%s] != Mac[%hs]"), *UniqueId, Item->macaddress().c_str());
				continue;
			}

			IdSet.Add(UniqueId);

			if (!GID2User.Contains(UniqueId))
			{
				SBaseView::FUserIp UserIp;
				UserIp.LoginUser = UTF8_TO_TCHAR(Item->loginuser().c_str());
				UserIp.IP = UTF8_TO_TCHAR(Item->ip().c_str());
				GID2User.Add({UniqueId, UserIp});
			}

			MaxTheoryHelpCore += Item->helpercore();
			const EAgentStatus AgentStatus = static_cast<EAgentStatus>(Item->status());
			if (Item->benablehelper())
			{
				if (AgentStatus == EAgentStatus::Status_Ready)
				{
					MaxTrueHelpCore += Item->helpercore();
				}
			}

			if (AgentStatus <= EAgentStatus::Status_Helping)
			{
				++AvailableAgentNum;
			}

			const FString GroupStr = UTF8_TO_TCHAR(Item->group().c_str());
			bool bExist = false;
			for (const auto& Group : GGroupArray)
			{
				if (Group.IsValid())
				{
					if (GroupStr == *Group)
					{
						bExist = true;
						break;
					}
				}
			}
			if (!bExist && !GroupStr.IsEmpty())
			{
				GGroupArray.Add(MakeShared<FString>(GroupStr));
			}

			if (bAlreadyExist)
			{
				auto Row = StaticCastSharedPtr<SAgentListRow>(AgentsListView->WidgetFromItem(Item));
				if (Row.IsValid())
				{
					Row->Invalidate(EInvalidateWidgetReason::Paint);
				}
			}
		}

		TSet<FString> NeedDeleteIds;
		for (const auto& Iter : Id2Agent)
		{
			if (!IdSet.Contains(Iter.Key))
			{
				NeedDeleteIds.Add(Iter.Key);
				Rebuild = true;
			}
		}

		DeleteAgents(NeedDeleteIds, Rebuild);
	}
	CATCH_REDIS_EXCEPTRION();
}

bool SAgentView::IsEnableEdit() const
{
	const auto _Items = this->AgentsListView->GetSelectedItems();
	if (_Items.Num() > 0)
	{
		if (_Items[0].IsValid())
		{
			const auto AgentStatus = _Items[0]->status();
			return GCurrentUser.Role != 2 && AgentStatus != 1 && AgentStatus != 2;
		}
	}
	return false;
}


#undef ADD_BATCH_CHECKBOX
#undef ADD_NUMEIRX_BOX
#undef ADD_COMBO_BOX

#undef LOCTEXT_NAMESPACE
