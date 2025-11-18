/**
  * @author cxx2020@outlook.com
  * @date 11:02 PM
 */
#include "SSettingsView.h"
#include "SlateOptMacros.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SGridPanel.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SHyperlink.h"
#include "Widgets/Input/SNumericEntryBox.h"
#include "Widgets/Input/SSpinBox.h"
#include "Widgets/SBackupListRow.h"
#include "Widgets/SEventListRow.h"
#include "Dialog/SMessageDialog.h"
#include "XiaoShareRedis.h"

#include "../Widgets/SConstrainBox.h"
#include "XiaoAppBase.h"
#include "ShareDefine.h"
#include "XiaoStyle.h"
#include "../SCoordinatorWindow.h"
#include "Const/WeChatQR.h"
#include "Widgets/SWarningBox.h"

#define LOCTEXT_NAMESPACE "SettingsView"

#define ADD_PORT_SLOT(DISPLAY_TEXT, MIN_VAL, MAX_VAL, PREDICATE, ENABLE) \
	+ SHorizontalBox::Slot().Padding(20.0f) \
	[ \
		SNew(SHorizontalBox) \
		+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center) \
		[ \
			SNew(STextBlock) \
			.Text(DISPLAY_TEXT) \
		] \
		+ SHorizontalBox::Slot().HAlign(HAlign_Left).AutoWidth() \
		[ \
			SNew(SNumericEntryBox<uint16>) \
			.MinValue(MIN_VAL).MaxValue(MAX_VAL) \
			.Delta(1) \
			.Value_Lambda([this]() { \
				return GModifySystemSettings.PREDICATE(); \
			}) \
			.OnValueCommitted_Lambda([this](const uint16 InPort, const ETextCommit::Type InType){ \
				GModifySystemSettings.set_##PREDICATE(InPort); \
			}) \
			.IsEnabled(ENABLE) \
		] \
	] \


#define ADD_CHECKBOX_SLOT(DISPLAY_TEXT, PREDICATE) \
	+ SVerticalBox::Slot().AutoHeight().Padding(10.0f) \
	[ \
		SNew(SCheckBox) \
		.IsChecked_Lambda([this]() \
			{ \
				return GModifySystemSettings.PREDICATE() ? ECheckBoxState::Checked : ECheckBoxState::Unchecked; \
			}) \
		.OnCheckStateChanged_Lambda([this](const ECheckBoxState InState) \
			{ \
				GModifySystemSettings.set_##PREDICATE(InState == ECheckBoxState::Checked ? true : false); \
			}) \
		.Content() \
		[ \
			SNew(STextBlock).Text(DISPLAY_TEXT) \
		] \
	] \


#define ADD_PERCENTBOX_SLOT(DISPLAY_TEXT, TOOLTIP_TEXT, PREDICATE) \
	+ SHorizontalBox::Slot().Padding(10.0f) \
	[ \
		SNew(SHorizontalBox) \
		+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center) \
		[ \
			SNew(STextBlock) \
			.Text(DISPLAY_TEXT) \
			.ToolTipText(TOOLTIP_TEXT) \
		] \
		+ SHorizontalBox::Slot().AutoWidth() \
		[ \
			SNew(SNumericEntryBox<float>) \
			.MinValue(0.0f).MaxValue(100) \
			.Value_Lambda([this]() { \
				return GModifySystemSettings.PREDICATE(); \
			}) \
			.OnValueCommitted_Lambda([this](const float InValue, const ETextCommit::Type) { \
				GModifySystemSettings.set_##PREDICATE(InValue); \
			}) \
		] \
	]

namespace
{
	static const FText SSystemUpdate = LOCTEXT("SystemUpdate_Text", "系统设置更新完成");
	static const FText SSystemError = LOCTEXT("SystemError_Text", "系统设置更新失败");
	static const FText SSystemParseError = LOCTEXT("SystemParseError_Text", "系统设置参数解析失败");
	static const FText SRedisError = LOCTEXT("CoordinatorError_Text", "协调器访问失败");
	static const FString SMb(TEXT("MB"));
	static const FString SGb(TEXT("GB"));
}


using namespace XiaoRedis;


BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SSettingsView::Construct(const FArguments& InArgs)
{
	XIAO_LOG(Log, TEXT("SSettingsView::Construct::Begin"));
	GLog->Flush();

	OnQueueNotification = InArgs._OnQueueNotification;
	DiskUnitArray.Add(MakeShared<FText>(FText::FromString(SMb)));
	DiskUnitArray.Add(MakeShared<FText>(FText::FromString(SGb)));

	EventArray.Add(MakeShared<FRedisEventDesc>(
		UTF8_TO_TCHAR(Channel::SProcessException.c_str()),
		TEXT(
		"message FException\n"
		"{\n"
			"\tstring TimeStamp = 1;\n"
			"\tstring BuildId = 2;\n"
			"\tstring ExecHost = 3;\n"
			"\tstring Mac = 4;\n"
			"\tint32 ExitCode = 5;\n"
			"\tstring Application = 6;\n"
			"\tstring Arguments = 7;\n"
			"\tstring WorkingDir = 8;\n"
			"\tstring Description = 9;\n"
			"\tstring LogFile = 10;\n"
		"}"),
		LOCTEXT("ProcessException_Text", "程序执行中出现的异常代码"))
	);

	QRWeChatBrush = LoadFromBuffer(SWechatQR, TEXT("ChenXiaoXi"));
	
	ConstructWidgets();

	ChildSlot
	[
		SNew(SVerticalBox)
#pragma region Body
		+SVerticalBox::Slot().FillHeight(0.8f)
		[
			SNew(SScrollBox).Orientation(Orient_Vertical)
			+ SScrollBox::Slot()
			[
				SNew(SVerticalBox)
#pragma region Network
				+ SVerticalBox::Slot().Padding(10.0f)
				[
					SNew(SBorder).Padding(10.0f)
					.IsEnabled_Lambda([]() { return GCurrentUser.Role != 2; })
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot().AutoHeight().Padding(10.0f)
						[
							SNew(STextBlock)
								.Text(LOCTEXT("Network_Text", "网络设置")).TextStyle(&XiaoH3TextStyle)
						]

						/*+ SVerticalBox::Slot().AutoHeight().Padding(10.0f)
						[
							SNew(STextBlock).Text(LOCTEXT("NetworkTip_Text", "每次保存只能有一个端口可以修改"))
						]*/

#pragma region Grid
						+ SVerticalBox::Slot().AutoHeight().Padding(10.0f)
						[
							SNew(SHorizontalBox)
								ADD_PORT_SLOT(LOCTEXT("AgentPort_Text", "代理连接端口"), 1024, 49151, agentserviceport, false)
								// ADD_PORT_SLOT(LOCTEXT("CoordiUI_Text", "管理UI端口"), 1024, 49151, webuiport)
								ADD_PORT_SLOT(LOCTEXT("CoordiServerPort_Text", "调度服务端口"), 1024, 49151, coordiserviceport, false)
								// ADD_PORT_SLOT(LOCTEXT("LicensePort_Text", "许可端口"), 1024, 49151, licenseserviceport, false)
						]
									
#pragma endregion
						+ SVerticalBox::Slot().AutoHeight().Padding(10.0f)
						[
							SNew(SHorizontalBox)
							+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
							[
								SNew(STextBlock)
									.Text(LOCTEXT("AgentStatus_Text", "状态同步频率"))
									.ToolTipText(LOCTEXT("gentStatus_ToolTip", "代理同步机器状态的频率，最小为1s，推荐为3s，最大为10s"))
							]
							+ SHorizontalBox::Slot().AutoWidth().HAlign(HAlign_Left)
							[
								SNew(SConstrainBox).MaxWidth(50.0f).MinWidth(50.0f)
								[
									SNew(SNumericEntryBox<uint16>)
									.AllowSpin(true)
									.Delta(1)
									.MinValue(1).MaxValue(10)
									.Value_Lambda([this]()
										{
											return GModifySystemSettings.syncfreq();
										})
									.OnValueCommitted_Lambda([this](const uint16 InPort, const ETextCommit::Type InType)
										{
											GModifySystemSettings.set_syncfreq(InPort);
										})
								]
							]
						]

						ADD_CHECKBOX_SLOT(LOCTEXT("EncryptCom_Text", "是否加密传输内容"), bencypttransport)		
					]
				]
#pragma endregion		
#pragma region SystemLimit
				+ SVerticalBox::Slot().Padding(10.0f).AutoHeight()
				[
					SNew(SBorder).Padding(10.0f)
					.IsEnabled_Lambda([]() { return GCurrentUser.Role != 2; })
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot().AutoHeight().Padding(10.0f)
						[
							SNew(STextBlock).Text(LOCTEXT("SystemLimit_Text", "系统限制")).TextStyle(&XiaoH3TextStyle)
						]

						+ SVerticalBox::Slot().AutoHeight()
						[
							SNew(SHorizontalBox)
							ADD_PORT_SLOT(LOCTEXT("MaxInitiator_Text", "同时允许多少发起者"), 1, MAX_uint32, maxinitiatornum, true)
							ADD_PORT_SLOT(LOCTEXT("MaximumCores_Text", "构建允许最多的核心"), 1, MAX_uint32, maxcorenum, true)
							ADD_PORT_SLOT(LOCTEXT("MaxHelpNum_Text", "构建允许最多的协助"), 1, MAX_uint32, maxconnum, true)
						]
					]
				]
#pragma endregion
#pragma region AgentSettings
				+ SVerticalBox::Slot().Padding(10.0f).AutoHeight()
				[
					SNew(SBorder).Padding(10.0f)
					.IsEnabled_Lambda([]() { return GCurrentUser.Role != 2; })
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot().AutoHeight().Padding(10.0f)
						[
							SNew(STextBlock)
								.Text(LOCTEXT("HelperCores_Text", "协助核心"))
								.TextStyle(&XiaoH3TextStyle)
						]

						+ SVerticalBox::Slot().AutoHeight().Padding(10.0f)
						[
							SNew(STextBlock).Text(LOCTEXT("HelperCoresTip_Text", "代理满足下列要求时才能被作为协助机器"))
						]

#pragma region Grid
						+ SVerticalBox::Slot().AutoHeight().Padding(10.0f)
						[
							SNew(SHorizontalBox)
							+ SHorizontalBox::Slot().Padding(10.0f)
							[
								SNew(SHorizontalBox)
								+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
								[
									SNew(STextBlock).Text(LOCTEXT("DiskSpace_Text", "硬盘空间"))
								]
								+ SHorizontalBox::Slot().AutoWidth().HAlign(HAlign_Left)
								[
									SNew(SConstrainBox).MaxWidth(200.0f).MinWidth(200.0f)
									[
										SNew(SHorizontalBox)
										+ SHorizontalBox::Slot().AutoWidth().HAlign(HAlign_Left).MinWidth(10.0f)
										[
											SNew(SNumericEntryBox<float>)
											.MinValue(0.0f).MaxValue(MAX_uint16)
											.Value_Lambda([this]()
											{
												TSharedPtr<FText> UnitText = DiskUnitArray[1];
												if (DiskSpaceUnitBox.IsValid())
												{
													UnitText = DiskSpaceUnitBox->GetSelectedItem();
												}
												return ToShowVal(UnitText, GModifySystemSettings.harddiskminimal());
											})
											.OnValueCommitted_Lambda([this](const float InValue, const ETextCommit::Type)
											{
												GModifySystemSettings.set_harddiskminimal(ToRealVal(DiskSpaceUnitBox->GetSelectedItem(), InValue));
											})
										]
										+ SHorizontalBox::Slot().AutoWidth().HAlign(HAlign_Left)
										[
											SAssignNew(DiskSpaceUnitBox, SComboBox<TSharedPtr<FText>>)
											.OptionsSource(&DiskUnitArray)
											.InitiallySelectedItem(DiskUnitArray[1])
											.OnGenerateWidget_Lambda([](const TSharedPtr<FText> InText)
											{
												return SNew(STextBlock).Text(*InText);
											})
											.OnSelectionChanged_Lambda([this](const TSharedPtr<FText> InText, ESelectInfo::Type)
											{
												const float ShowVal = ToShowVal(InText, GModifySystemSettings.harddiskminimal());
												GModifySystemSettings.set_harddiskminimal(ToRealVal(InText, ShowVal));
											})
											.Content()
											[
												SNew(STextBlock).Text_Lambda([this]()
												{
													return *this->DiskSpaceUnitBox->GetSelectedItem();
												})
											]
										]
									]
								]
							]

							+ SHorizontalBox::Slot().Padding(10.0f)
							[
								SNew(SHorizontalBox)
								+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
								[
									SNew(STextBlock).Text(LOCTEXT("PhysicalMemory_Text", "物理内存"))
								]
								+ SHorizontalBox::Slot().AutoWidth().HAlign(HAlign_Left)
								[
									SNew(SConstrainBox).MaxWidth(200.0f).MinWidth(200.0f)
									[
										SNew(SHorizontalBox)
										+ SHorizontalBox::Slot().AutoWidth().HAlign(HAlign_Left).MinWidth(10.0f)
										[
											SNew(SNumericEntryBox<float>)
											.MinValue(0.0f).MaxValue(MAX_uint16)
											.Value_Lambda([this](){
												TSharedPtr<FText> UnitText = DiskUnitArray[1];
												if (PhysicalMemoryUnitBox.IsValid())
												{
													UnitText = PhysicalMemoryUnitBox->GetSelectedItem();
												}
												return ToShowVal(UnitText, GModifySystemSettings.physicalmemory());
											})
											.OnValueCommitted_Lambda([this](const float InValue, const ETextCommit::Type){
												GModifySystemSettings.set_physicalmemory(ToRealVal(PhysicalMemoryUnitBox->GetSelectedItem(), InValue));
											})
										]
										+ SHorizontalBox::Slot().AutoWidth().HAlign(HAlign_Left)
										[
											SAssignNew(PhysicalMemoryUnitBox, SComboBox<TSharedPtr<FText>>)
											.OptionsSource(&DiskUnitArray)
											.InitiallySelectedItem(DiskUnitArray[1])
											.OnGenerateWidget_Lambda([](const TSharedPtr<FText> InText){
												return SNew(STextBlock).Text(*InText);
											})
											.OnSelectionChanged_Lambda([this](const TSharedPtr<FText> InText, ESelectInfo::Type){
												const float ShowVal = ToShowVal(InText, GModifySystemSettings.physicalmemory());
												GModifySystemSettings.set_physicalmemory(ToRealVal(InText, ShowVal));
											})
											.Content()
											[
												SNew(STextBlock).Text_Lambda([this]()
												{
													return *this->PhysicalMemoryUnitBox->GetSelectedItem();
												})
											]
										]
									]
								]
							]
							
							ADD_PERCENTBOX_SLOT(LOCTEXT("AvailableCPUMinimal_Text", "可用CPU(%)"), LOCTEXT("AvailableCPUMinimal_ToolTip", "作为协助者必须有至少额外的cpu百分比"), cpuavailableminimal)
							ADD_PERCENTBOX_SLOT(LOCTEXT("AvailableDiskMinimal_Text", "可用磁盘(%)"), LOCTEXT("AvailableDiskMinimal_ToolTip", "作为协助者必须有至少额外的磁盘百分比"), diskavamin)
							ADD_PERCENTBOX_SLOT(LOCTEXT("AvailableNetMinimal_Text", "可用网络(%)"), LOCTEXT("AvailableNetMinimal_ToolTip", "作为协助者必须有至少额外的网络百分比"), networkavamin)
							ADD_PERCENTBOX_SLOT(LOCTEXT("AvailableGpuMinimal_Text", "可用Gpu(%)"), LOCTEXT("AvailableGpuUMinimal_ToolTip", "作为协助者必须有至少额外的Gpu百分比"), gpuavamin)
						]
#pragma endregion
										
#pragma region AgentSettings
						+ SVerticalBox::Slot().AutoHeight().Padding(10.0f)
						[
							SNew(STextBlock)
								.Text(LOCTEXT("AgentSet_Text", "代理设置")).TextStyle(&XiaoH3TextStyle)
						]

						ADD_CHECKBOX_SLOT(LOCTEXT("AllowDiffArch_Text", "是否允许CPU架构不同"), bignorearch)
						ADD_CHECKBOX_SLOT(LOCTEXT("AllowAgents_Text", "是否允许代理设置作为协助者"), benablehelper)

						+ SVerticalBox::Slot().AutoHeight().Padding(10.0f)
						[
							SNew(SHorizontalBox)
							+ SHorizontalBox::Slot().AutoWidth()
							[
								SAssignNew(ScheduleCleanupCheckBox, SCheckBox)
								.IsChecked_Lambda([]() {
									return GModifySystemSettings.bscheduleclean() ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
								}) 
								.OnCheckStateChanged_Lambda([](const ECheckBoxState InState) { 
									GModifySystemSettings.set_bscheduleclean(InState == ECheckBoxState::Checked ? true : false);
								})
								.Content()
								[
									SNew(STextBlock)
									.Text(LOCTEXT("ScheduleClean_Text", "是否开启定时清理代理CAS数据"))
								]
							]

							+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(10.0f, 0.0f)
							[
								SNew(STextBlock)
									.Text(LOCTEXT("ScheduleTime_Text", "清理周期(天)"))
							]
							+ SHorizontalBox::Slot().AutoWidth()
							[
								SAssignNew(ScheduleTimeSpinbox, SSpinBox<uint32>).MinValue(1).MaxValue(14)
									.OnValueChanged_Lambda([](const uint32 InValue) { GModifySystemSettings.set_scheduletime(InValue); })
									.Value_Lambda([]() { return GModifySystemSettings.scheduletime(); })
									.IsEnabled_Lambda([]() { return GModifySystemSettings.bscheduleclean(); })
							]
						] 
#pragma endregion
					]
				]
#pragma endregion
#pragma region EventSubscribe
				+ SVerticalBox::Slot().Padding(10.0f).Padding(10.0f).AutoHeight()
				[
					SNew(SBorder)
					.Padding(10.0f)
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Left).Padding(10.0f)
						[
							SNew(STextBlock)
								.Text(LOCTEXT("SubscribeEvent_Text", "事件订阅"))
								.TextStyle(&XiaoH3TextStyle)
						]
						+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Left).Padding(10.0f)
						[
							SNew(STextBlock)
								.Text(LOCTEXT("SubscribeEventToolTip_Text", "XiaoBuild联合编译系统在运行中所触发的事件"))
						]
						+ SVerticalBox::Slot().AutoHeight().Padding(5.0f, 30.0f)
						[
							SAssignNew(EventListView, SListView<TSharedPtr<FRedisEventDesc>>)
							.ListItemsSource(&EventArray)
							.Orientation(Orient_Vertical)
							.SelectionMode(ESelectionMode::Type::Single)
							.EnableAnimatedScrolling(true)
							// .ItemHeight(50.0f)
							.AllowOverscroll(EAllowOverscroll::Yes)
							.OnGenerateRow_Raw(this, &SSettingsView::OnGenerateEventRow)
							.HeaderRow(
								SNew(SHeaderRow)

								+ SHeaderRow::Column(GEventTableColumnIDChannel)
								.FillWidth(.18f)
								.HAlignHeader(EHorizontalAlignment::HAlign_Center)
								.VAlignHeader(VAlign_Center)
								.DefaultLabel(LOCTEXT("EventChannel_Text", "频道"))
								.DefaultTooltip(LOCTEXT("EventChannel_Tooltip", "可监听的频道"))

								+ SHeaderRow::Column(GEventTableColumnIDMessage)
								.FillWidth(.18f)
								.HAlignHeader(EHorizontalAlignment::HAlign_Center)
								.VAlignHeader(VAlign_Center)
								.DefaultLabel(LOCTEXT("EventMessage_Text", "Protobuf"))
								.DefaultTooltip(LOCTEXT("EventMessage_Tooltip", "Google的Protobuf协议定义的消息"))

								+ SHeaderRow::Column(GEventTableColumnIDDesc)
								.FillWidth(.18f)
								.HAlignHeader(EHorizontalAlignment::HAlign_Center)
								.VAlignHeader(VAlign_Center)
								.DefaultLabel(LOCTEXT("EventDesc_Text", "描述"))
								.DefaultTooltip(LOCTEXT("EventDesc_Tooltip", "事件描述"))
							)
						]
					]
				]
#pragma endregion
#pragma region BackCoordi
				+ SVerticalBox::Slot().Padding(10.0f).Padding(10.0f).AutoHeight()
				[
					SNew(SBorder)
					.Padding(10.0f).IsEnabled_Lambda([]() { return GCurrentUser.Role != 2; })
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Left).Padding(10.0f)
						[
							SNew(STextBlock)
								.Text(LOCTEXT("BackCoordi_Text", "后备机器"))
								.TextStyle(&XiaoH3TextStyle)
						]
						+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Left).Padding(10.0f)
						[
							SNew(STextBlock)
								.Text(LOCTEXT("ACoordiToUse_Text", "副机设置，如果主机协调器掉线了，副机顶上"))
						]
						+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
						[
							SNew(SWarningBox)
								.Message(LOCTEXT("NoBackup_Text", "当前系统没有配置后备服务器(或者备用服务器没有启动)，请尽快配置，提高系统的可靠性"))
								.Visibility_Lambda([this]() {
									uint32 ActiveNum = 0;
									for (const auto& Backup : BackupArray)
									{
										if (Backup.IsValid() && Backup->Status)
										{
											++ActiveNum;
										}
									}
									return ActiveNum <= 1 ? EVisibility::Visible : EVisibility::Collapsed;
								})
								.Content()
								[
									SNew(SHyperlink).Text(LOCTEXT("HelpBack_Text", "  [如何配置备份机器]  "))
										.OnNavigate_Lambda([]() {
											GetHelp(GLocalization == TEXT("zh-CN") ? TEXT("后备机器") : TEXT(""));
										})
								]
						]
						+ SVerticalBox::Slot().AutoHeight().Padding(5.0f, 30.0f)
						[
							SAssignNew(BackupListView, SListView<TSharedPtr<FRedisServerDesc>>)
							.ListItemsSource(&BackupArray)
							.Orientation(Orient_Vertical)
							.SelectionMode(ESelectionMode::Type::Single)
							.EnableAnimatedScrolling(true)
							// .ItemHeight(50.0f)
							.AllowOverscroll(EAllowOverscroll::Yes)
							.OnGenerateRow_Raw(this, &SSettingsView::OnGenerateBackRow)
							.OnContextMenuOpening_Raw(this, &SSettingsView::OnContextMenuOpening)
							.HeaderRow(
								SNew(SHeaderRow)

								+ SHeaderRow::Column(GBackupTableColumnIDIp)
								.FillWidth(.18f)
								.HAlignHeader(EHorizontalAlignment::HAlign_Center)
								.VAlignHeader(VAlign_Center)
								.DefaultLabel(LOCTEXT("Ip_Text", "Ip"))
								.DefaultTooltip(LOCTEXT("Ip_Tooltip", "IP地址"))
								.InitialSortMode(EColumnSortMode::Type::Ascending)

								+ SHeaderRow::Column(GBackupTableColumnIDRole)
								.FillWidth(.18f)
								.HAlignHeader(EHorizontalAlignment::HAlign_Center)
								.VAlignHeader(VAlign_Center)
								.DefaultLabel(LOCTEXT("Role_Text", "职能"))
								.DefaultTooltip(LOCTEXT("Role_Tooltip", "所承担的功能"))
								.InitialSortMode(EColumnSortMode::Type::Ascending)
								.OnSort_Raw(this, &SSettingsView::OnTableSort)
								.SortMode_Raw(this, &SSettingsView::GetSortModeForColumn, GBackupTableColumnIDRole)

								+ SHeaderRow::Column(GBackupTableColumnIDPriority)
								.FillWidth(.18f)
								.HAlignHeader(EHorizontalAlignment::HAlign_Center)
								.VAlignHeader(VAlign_Center)
								.DefaultLabel(LOCTEXT("Priority_Text", "优先级"))
								.DefaultTooltip(LOCTEXT("Priority_Tooltip", "当主服务器掉线时，从服务器顶替的优先级，值越小优先级越高"))
								.InitialSortMode(EColumnSortMode::Type::Ascending)
								.OnSort_Raw(this, &SSettingsView::OnTableSort)
								.SortMode_Raw(this, &SSettingsView::GetSortModeForColumn, GBackupTableColumnIDPriority)

								+ SHeaderRow::Column(GBackupTableColumnIDStatus)
								.FillWidth(.18f)
								.HAlignHeader(EHorizontalAlignment::HAlign_Center)
								.VAlignHeader(VAlign_Center)
								.DefaultLabel(LOCTEXT("Status_Text", "状态"))
								.DefaultTooltip(LOCTEXT("Status_Tooltip", "当前运行状态"))
								.InitialSortMode(EColumnSortMode::Type::Ascending)
								.OnSort_Raw(this, &SSettingsView::OnTableSort)
								.SortMode_Raw(this, &SSettingsView::GetSortModeForColumn, GBackupTableColumnIDStatus)

								+ SHeaderRow::Column(GBackupTableColumnIDDelete)
								.FillWidth(.18f)
								.HAlignHeader(EHorizontalAlignment::HAlign_Center)
								.VAlignHeader(VAlign_Center)
								.DefaultLabel(LOCTEXT("Delete_Text", "删除"))
								.DefaultTooltip(LOCTEXT("Delete_Tooltip", "将当前机器从后备服务器中去除"))
								.InitialSortMode(EColumnSortMode::Type::Ascending)
							)
						]
					]
				]
#pragma endregion
#pragma region Product
					+ SVerticalBox::Slot().Padding(10.0f).Padding(10.0f).AutoHeight()
					[
						SNew(SBorder)
						.Padding(10.0f).IsEnabled_Lambda([]() { return GCurrentUser.Role != 2; })
						[
							SNew(SVerticalBox)
							+ SVerticalBox::Slot().AutoHeight().Padding(10.0f)
							[
								SNew(STextBlock)
									.Text(LOCTEXT("Product_Text", "产品介绍")).TextStyle(&XiaoH3TextStyle)
							]
							+ SVerticalBox::Slot().AutoHeight().Padding(10.0f)
							[
								SNew(SCheckBox)
								.IsChecked_Lambda([this](){
									return GModifySystemSettings.bhelperenhance() ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
								})
								.OnCheckStateChanged_Lambda([this](const ECheckBoxState InState){
									GModifySystemSettings.set_bhelperenhance(InState == ECheckBoxState::Checked ? true : false);
								})
								.Content()
								[
									SNew(STextBlock).Text(LOCTEXT("HelpUsImprove_Text", "是否帮助帮助我们改善产品和服务"))
								]
							]

							+ SVerticalBox::Slot().AutoHeight().Padding(10.0f)
							[
								SNew(STextBlock)
									.Text(LOCTEXT("Connnect_Text", "联系方式:"))
							]

							// Email
							+ SVerticalBox::Slot().AutoHeight().Padding(10.0f)
							[
								SNew(SHorizontalBox)
								+ SHorizontalBox::Slot().AutoWidth()
								[
									SNew(STextBlock)
										.Text(LOCTEXT("Email_Text", "Email  :"))
								]
								+ SHorizontalBox::Slot().AutoWidth().HAlign(HAlign_Left)
								[
									SNew(SEditableTextBox)
										.IsReadOnly(true)
										.Text(FText::FromString(TEXT("cxx2020@outlook.com")))
								]
							]

							// Discord
							+ SVerticalBox::Slot().AutoHeight().Padding(10.0f)
							[
								SNew(SHorizontalBox)
								+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
								[
									SNew(STextBlock)
									.Text(LOCTEXT("Discord_Text", "Discord:"))
								]
								+ SHorizontalBox::Slot().AutoWidth().HAlign(HAlign_Left).VAlign(VAlign_Center)
								[
									SNew(SHyperlink)
									.Text(FText::FromString(XiaoUrl::SXiaoRedisChannel))
									.OnNavigate_Lambda([]() {
										FString Error;
										FPlatformProcess::LaunchURL(*XiaoUrl::SXiaoRedisChannel, TEXT(""), &Error);
									})
								]
							]

							// Github
							+ SVerticalBox::Slot().AutoHeight().Padding(10.0f)
							[
								SNew(SHorizontalBox)
								+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
								[
									SNew(STextBlock)
									.Text(LOCTEXT("Github_Text", "Github:"))
								]
								+ SHorizontalBox::Slot().AutoWidth().HAlign(HAlign_Left).VAlign(VAlign_Center)
								[
									SNew(SHyperlink)
									.Text(FText::FromString(XiaoUrl::SXiaoBuildWeb))
									.OnNavigate_Lambda([]() {
										FString Error;
										FPlatformProcess::LaunchURL(*XiaoUrl::SXiaoBuildWeb, TEXT(""), &Error);
									})
								]
							]

							// Gitee
							+ SVerticalBox::Slot().AutoHeight().Padding(10.0f)
							[
								SNew(SHorizontalBox)
								+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
								[
									SNew(STextBlock)
										.Text(LOCTEXT("Gitee", "Gitee:"))
								]
								+ SHorizontalBox::Slot().AutoWidth().HAlign(HAlign_Left).VAlign(VAlign_Center)
								[
									SNew(SHyperlink)
									.Text(FText::FromString(XiaoUrl::SGiteeWeb))
									.OnNavigate_Lambda([]() {
									FString Error;
									FPlatformProcess::LaunchURL(*XiaoUrl::SGiteeWeb, TEXT(""), &Error);
										})
								]
							]

							// QQ Group
							+ SVerticalBox::Slot().AutoHeight().Padding(10.0f)
							[
								SNew(SHorizontalBox)
									+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
									[
										SNew(STextBlock)
											.Text(LOCTEXT("QuestionFeedback_Text", "QQ群:"))
									]
									+ SHorizontalBox::Slot().AutoWidth().HAlign(HAlign_Left).VAlign(VAlign_Center)
									[
										SNew(SEditableTextBox)
											.IsReadOnly(true)
											.Text(FText::FromString(TEXT("910420853")))
									]
							]

							// Wechat
							+ SVerticalBox::Slot().AutoHeight().Padding(10.0f)
							[
								SNew(SHorizontalBox)
								+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
								[
									SNew(STextBlock)
										.Text(LOCTEXT("Wechat_Text", "WeChat:"))
								]
								+ SHorizontalBox::Slot().AutoWidth().HAlign(HAlign_Left).VAlign(VAlign_Center)
								[
									SNew(SEditableTextBox)
										.IsReadOnly(true)
										.Text(FText::FromString(TEXT("c794569465")))
								]
								+ SHorizontalBox::Slot().AutoWidth().HAlign(HAlign_Left)
								[
									SNew(SImage).Image(QRWeChatBrush.Get()).DesiredSizeOverride(FVector2D(200, 200))
								]
							]
						]
					]
#pragma endregion
			]
		]
#pragma endregion
#pragma region Foot
		+SVerticalBox::Slot().AutoHeight()
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().HAlign(HAlign_Fill)
			[
				SNullWidget::NullWidget
			]
			+ SHorizontalBox::Slot().HAlign(HAlign_Right).AutoWidth()
			[
				SNew(SBorder)
				.Visibility_Lambda([this]() { return GetButtonVisibility(); })
				.BorderImage(FXiaoStyle::Get().GetBrush("NoBorder"))
				[
					SAssignNew(DiscardAllButton, SButton).Text(LOCTEXT("DiscardAll_Text", "放弃修改"))
					.Visibility_Raw(this, &SSettingsView::GetButtonVisibility)
					.IsEnabled_Raw(this, &SSettingsView::GetDiscardButtonEnable)
					.OnPressed_Raw(this, &SSettingsView::OnDiscardModify)
					.ButtonStyle(FXiaoStyle::Get(), "FlatButton.Primary")
					.TextStyle(FXiaoStyle::Get(), "FlatButton.DefaultTextStyle")
					
				]
			]
			+ SHorizontalBox::Slot().HAlign(HAlign_Right).AutoWidth()
			[
				SNew(SBorder)
				.Visibility_Lambda([this]() { return GetButtonVisibility(); })
				.BorderImage(FXiaoStyle::Get().GetBrush("NoBorder"))
				[
					SAssignNew(SaveAllButton, SButton).Text(LOCTEXT("SaveAll_Text", "保存修改"))
					.Visibility_Raw(this, &SSettingsView::GetButtonVisibility)
					.IsEnabled_Raw(this, &SSettingsView::GetSaveButtonEnable)
					.OnPressed_Raw(this, &SSettingsView::OnCommitModify)
					.ButtonStyle(FXiaoStyle::Get(), "FlatButton.Warning")
					.TextStyle(FXiaoStyle::Get(), "FlatButton.DefaultTextStyle")
				]
			]
		]
		
#pragma endregion 
	];

	XIAO_LOG(Log, TEXT("SSettingsView::Construct::Finish"));
	GLog->Flush();
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SSettingsView::OnUpdate(const bool bInRebuild) const
{
	OnQuerySystem();
	OnQueryBackup();
}

void SSettingsView::OnQuerySystem() const
{
	if (!XiaoRedis::IsConnected() || GetDiscardButtonEnable())
	{
		return;
	}

	try
	{
		const auto Option = XiaoRedis::SRedisClient->get(XiaoRedis::String::SSystemSettings);
		if (Option.has_value())
		{
			const std::string Protobuf = Option.value();
			if (GModifySystemSettings.ParseFromString(Protobuf))
			{
				Assign(GModifySystemSettings, GOriginalSystemSettings);
			}
			else
			{
				(void)OnQueueNotification.ExecuteIfBound(-1, SSystemParseError);
			}
		}
	}
	CATCH_REDIS_EXCEPTRION()
}

void SSettingsView::OnQueryBackup() const
{
	if (!IsConnected())
	{
		return;
	}

	try
	{
		const int64 BackupNum = SRedisClient->hlen(Hash::SCacheList);
		if (BackupNum <= 0)
		{
			return;
		}

		bool bNeedRefresh = false;

		TSet<FString> ExistIps;

		std::unordered_map<std::string, std::string> Result;
		SRedisClient->hgetall(Hash::SCacheList, std::inserter(Result, Result.begin()));
		for (const auto& Iter : Result)
		{
			FRedisServerDesc TempDesc;
			const FString JsonContent = UTF8_TO_TCHAR(Iter.second.c_str());
			if (!TempDesc.FromJson(JsonContent))
			{
				XIAO_LOG(Warning, TEXT("Parse FRedisServerDesc failed!"));
				continue;
			}

			ExistIps.Add(TempDesc.Host);
			bool bAlreadyExist = Ip2Redis.Contains(TempDesc.Host);
			if (!bAlreadyExist)
			{
				auto Backup = MakeShared<FRedisServerDesc>();

				BackupArray.Add(Backup);
				Ip2Redis.Add(MakeTuple(TempDesc.Host, Backup));
				bNeedRefresh = true;
			}

			*Ip2Redis[TempDesc.Host].Pin() = TempDesc;

			if (bAlreadyExist)
			{
				auto Row = StaticCastSharedPtr<SBackupRow>(BackupListView->WidgetFromItem(Ip2Redis[TempDesc.Host].Pin()));
				if (Row.IsValid())
				{
					Row->Invalidate(EInvalidateWidgetReason::Paint);
				}
			}
		}

		TSet<FString> NeedDeleteSet;
		for (const auto& Iter : Ip2Redis)
		{
			if (!ExistIps.Contains(Iter.Key))
			{
				bNeedRefresh = true;
				NeedDeleteSet.Add(Iter.Key);
			}
		}
		for (const FString& Ip : NeedDeleteSet)
		{
			BackupArray.Remove(Ip2Redis[Ip].Pin());
			Ip2Redis.Remove(Ip);
		}

		if (bNeedRefresh)
		{
			BackupArray.Sort([](const TSharedPtr<FRedisServerDesc>& L, const TSharedPtr<FRedisServerDesc>& R) { return L->Role && !R->Role; });
			BackupListView->RequestListRefresh();
		}
		return;
	}
	CATCH_REDIS_EXCEPTRION()
}

TSharedRef<ITableRow> SSettingsView::OnGenerateEventRow(const TSharedPtr<FRedisEventDesc> InDesc, const TSharedRef<STableViewBase>& InTableView) const
{
	return SNew(SEventRow, InTableView).EventDesc(InDesc);
}

void SSettingsView::ConstructWidgets()
{
	XiaoRedis::SetDefaultParams(GModifySystemSettings);
	Assign(GModifySystemSettings, GOriginalSystemSettings);
}

EVisibility SSettingsView::GetButtonVisibility() const
{
	return GetSaveButtonEnable() ? EVisibility::Visible : EVisibility::Collapsed;
}

bool SSettingsView::GetDiscardButtonEnable() const
{
	return IsEqual(GOriginalSystemSettings, GModifySystemSettings);
}

bool SSettingsView::GetSaveButtonEnable() const
{
	return GetDiscardButtonEnable();
}

void SSettingsView::OnDiscardModify()
{
	Assign(GOriginalSystemSettings, GModifySystemSettings);
}

void SSettingsView::OnCommitModify()
{
	if (const auto Window = StaticCastSharedPtr<SCoordinatorWindow>(FXiaoAppBase::GApp->GetMainWindow().Pin()))
	{
		Window->SetLockedState(true);
		FXiaoAppBase::GApp->AddNextTickTask(FSimpleDelegate::CreateLambda([this]()
		{
			const TSharedRef<SMessageDialog> Dialog = SNew(SMessageDialog)
				.Title(LOCTEXT("ConfirmTitle", "确定"))
				.Icon(FAppStyle::Get().GetBrush("Icons.WarningWithColor.Large"))
				.Message(LOCTEXT("ConfirmModity_Message", "确定提交参数的修改?"))
				.UseScrollBox(false)
				.AutoCloseOnButtonPress(true)
				.Buttons(
					{
						SMessageDialog::FButton(LOCTEXT("ConfirmButton", "确定"))
						.SetOnClicked(FSimpleDelegate::CreateLambda([this]()
						{
							try
							{
								XiaoRedis::SRedisClient->set(XiaoRedis::String::SSystemSettings, GModifySystemSettings.SerializeAsString());
								Assign(GModifySystemSettings, GOriginalSystemSettings);
								(void)OnQueueNotification.ExecuteIfBound(0, SSystemUpdate);
								return;
							}
							CATCH_REDIS_EXCEPTRION();
							(void)OnQueueNotification.ExecuteIfBound(-1, SSystemError);
						})),
						SMessageDialog::FButton(LOCTEXT("CancelButton", "取消")).SetPrimary(true).SetFocus()
					});
			Dialog->GetOnWindowClosedEvent().AddLambda([](const TSharedRef<SWindow>&)
			{
				if (auto Window = StaticCastSharedPtr<SCoordinatorWindow>(FXiaoAppBase::GApp->GetMainWindow().Pin()))
				{
					Window->SetLockedState(false);
				}
			});
			Dialog->ShowModal();
		}));
	}
}

TSharedRef<ITableRow> SSettingsView::OnGenerateBackRow(const TSharedPtr<FRedisServerDesc> InDesc, const TSharedRef<STableViewBase>& InTableView) const
{
	return SNew(SBackupRow, InTableView)
		.BackupDesc(InDesc)
		.OnRoleChange_Raw(this, &SSettingsView::OnRoleChange)
		.OnBackupChange_Raw(this, &SSettingsView::OnUpdateBackup)
		.OnBackupDelete_Raw(this, &SSettingsView::OnDeleteBackup);
}

TSharedPtr<SWidget> SSettingsView::OnContextMenuOpening()
{
	const auto Items = this->BackupListView->GetSelectedItems();
	if (Items.Num() > 0)
	{
		FMenuBuilder MenuBuilder(true, nullptr);

		MenuBuilder.AddMenuEntry(
			LOCTEXT("DeleteBackup_Text", "删除后备机器"),
			FText::GetEmpty(),
			FSlateIcon(),
			FUIAction(
				FExecuteAction::CreateLambda([this]() {
					const auto _Items = this->BackupListView->GetSelectedItems();
					if (_Items.Num() > 0)
					{
						OnDeleteBackup(_Items[0]);
					}
					})
			)
		);
	}
	return SNullWidget::NullWidget;
}

void SSettingsView::OnTableSort(const EColumnSortPriority::Type InSortPriority, const FName& InName, EColumnSortMode::Type InSortMode) const
{
	static const TArray<FName> ColumnIds = { GBackupTableColumnIDRole, GBackupTableColumnIDPriority, GBackupTableColumnIDStatus };

	ColumnIdToSort = InName;
	ActiveSortMode = InSortMode;

	TArray<FName> ColumnIdsBySortOrder = { InName };
	for (const FName& Id : ColumnIds)
	{
		if (Id != InName)
		{
			ColumnIdsBySortOrder.Add(Id);
		}
	}

	BackupArray.Sort([ColumnIdsBySortOrder, InSortMode](const TSharedPtr<FRedisServerDesc>& Left, const TSharedPtr<FRedisServerDesc>& Right)
	{
		int32 CompareResult = 0;
		for (const FName& ColumnId : ColumnIdsBySortOrder)
		{
			if (ColumnId == GBackupTableColumnIDRole)
			{
				CompareResult = (Left->Role == Right->Role) ? 0 : ((Left->Role && !Right->Role) ? 1 : -1);
			}
			else if (ColumnId == GBackupTableColumnIDPriority)
			{
				CompareResult = (Left->Priority == Right->Priority) ? 0 : ((Left->Priority < Right->Priority) ? 1 : -1);
			}
			else if (ColumnId == GBackupTableColumnIDStatus)
			{
				CompareResult = (Left->Status == Right->Status) ? 0 : ((Left->Status && !Right->Status) ? 1 : -1);
			}

			if (CompareResult != 0)
			{
				return InSortMode == EColumnSortMode::Ascending ? CompareResult < 0 : CompareResult > 0;
			}
		}

		return InSortMode == EColumnSortMode::Ascending ? true : false;
	});

	BackupListView->RebuildList();
	BackupListView->RequestListRefresh();
}

EColumnSortMode::Type SSettingsView::GetSortModeForColumn(const FName InColumnId) const
{
	return (InColumnId == ColumnIdToSort) ? ActiveSortMode : EColumnSortMode::None;
}

void SSettingsView::OnRoleChange(const TWeakPtr<FRedisServerDesc>& InDesc) const
{
	// 查找当前Master
	for (auto& Node : BackupArray)
	{
		if (Node.IsValid() && Node->Role)
		{
			Node->Role = false;
			OnUpdateBackup(Node);
			break;
		}
	}
	// 升级当前选中的节点
	OnUpdateBackup(InDesc);
	BackupListView->RequestListRefresh();
}

void SSettingsView::OnUpdateBackup(const TWeakPtr<FRedisServerDesc>& InDesc) const
{
	if (!InDesc.IsValid())
	{
		XIAO_LOG(Error, TEXT("OnUpdateBackup InDesc is not valid!"));
		return;
	}

	if (!IsConnected())
	{
		(void)OnQueueNotification.ExecuteIfBound(-1, SRedisError);
		return;
	}

	try
	{
		std::unordered_map<std::string, std::string> Result;
		SRedisClient->hgetall(Hash::SCacheList, std::inserter(Result, Result.begin()));
		for (const auto& Iter : Result)
		{
			const FString Json = UTF8_TO_TCHAR(Iter.second.c_str());
			FRedisServerDesc Desc;
			if (Desc.FromJson(Json))
			{
				if (Desc.Host == InDesc.Pin()->Host)
				{
					const std::string Content = TCHAR_TO_UTF8(*InDesc.Pin()->ToJson());
					SRedisClient->hset(Hash::SCacheList, Iter.first, Content);
					return;
				}
			}
		}
	
		return;
	}
	CATCH_REDIS_EXCEPTRION();
	(void)OnQueueNotification.ExecuteIfBound(-1, SSystemError);
}

void SSettingsView::OnDeleteBackup(const TWeakPtr<FRedisServerDesc>& InDesc) const
{
	if (!InDesc.IsValid())
	{
		XIAO_LOG(Error, TEXT("OnUpdateBackup InDesc is not valid!"));
		return;
	}

	if (!IsConnected())
	{
		(void)OnQueueNotification.ExecuteIfBound(-1, SRedisError);
		return;
	}

	try
	{
		std::unordered_map<std::string, std::string> Result;
		SRedisClient->hgetall(Hash::SCacheList, std::inserter(Result, Result.begin()));
		for (const auto& Iter : Result)
		{
			const FString Json = UTF8_TO_TCHAR(Iter.second.c_str());
			FRedisServerDesc Desc;
			if (Desc.FromJson(Json))
			{
				if (Desc.Host == InDesc.Pin()->Host)
				{
					if (Ip2Redis.Contains(Desc.Host))
					{
						BackupArray.Remove(Ip2Redis[Desc.Host].Pin());
						Ip2Redis.Remove(Desc.Host);
					}
					SRedisClient->hdel(Hash::SCacheList, Iter.first);
					BackupListView->RequestListRefresh();
					return;
				}
			}
		}
		return;
	}
	CATCH_REDIS_EXCEPTRION();
	(void)OnQueueNotification.ExecuteIfBound(-1, SSystemError);
}

float SSettingsView::ToShowVal(const TSharedPtr<FText>& InText, const float& InRealData)
{
	if (InText->ToString() == SMb)
	{
		return InRealData * 1024.0f;
	}
	return InRealData;
}

float SSettingsView::ToRealVal(const TSharedPtr<FText>& InText, const float& InShowData)
{
	if (InText->ToString() == SMb)
	{
		return InShowData / 1024.0f;
	}
	return InShowData;
}

#undef ADD_PERCENTBOX_SLOT
#undef ADD_CHECKBOX_SLOT
#undef ADD_PORT_SLOT
#undef LOCTEXT_NAMESPACE
