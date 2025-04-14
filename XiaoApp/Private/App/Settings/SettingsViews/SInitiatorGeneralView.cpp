/**
  * @author cxx2020@outlook.com
  * @date 10:49 AM
 */


#include "SInitiatorGeneralView.h"
#include "ShareDefine.h"
#include "SlateOptMacros.h"
#include "Widgets/Layout/SExpandableArea.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SSeparator.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SNumericEntryBox.h"
#include "Widgets/Input/SComboBox.h"

#include "XiaoAgent.h"
#include "XiaoStyle.h"

#define LOCTEXT_NAMESPACE "InitiatorGeneral"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SInitiatorGeneralView::Construct(const FArguments& InArgs, FInitiatorGeneral* InSettings)
{
	Settings = InSettings;
	
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
					SNew(STextBlock)
					.Text(LOCTEXT("Distribution_Text", "分发"))
				]
				
				+ SVerticalBox::Slot().SEC_PADDING
				[
					SAssignNew(RestartRemoteCheckBox, SCheckBox)
					.IsChecked_Lambda([this]() {
						return Settings->bRestartRemoteProcess ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
					})
					.OnCheckStateChanged_Lambda([this](const ECheckBoxState InState)
					{
						this->Settings->bRestartRemoteProcess = InState == ECheckBoxState::Checked ? true : false;
					})
					.Content()
					[
						SNew(STextBlock)
							.Text(LOCTEXT("RestartRemote_Text", "尽可能在本地计算机上重新启动远程程序"))
					]
				]

				+ SVerticalBox::Slot().SEC_PADDING
				[
					SAssignNew(AvoidTaskCheckBox, SCheckBox)
					.IsChecked_Lambda([this]() {
						return Settings->bAvoidTaskExecution ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
					})
					.OnCheckStateChanged_Lambda([this](const ECheckBoxState InState)
					{
						this->Settings->bAvoidTaskExecution = InState == ECheckBoxState::Checked ? true : false;
					})
					.Content()
					[
						SNew(STextBlock)
							.Text(LOCTEXT("AvoidTask_Text", "尽可能避免在本地上执行任务"))
					]				
				]

				+ SVerticalBox::Slot().SEC_PADDING
				[
					SAssignNew(EnableStandaloneCheckBox, SCheckBox)
					.IsChecked_Lambda([this]() {
						return Settings->bEnableStandoneMode ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
					})
					.OnCheckStateChanged_Lambda([this](const ECheckBoxState InState)
					{
						this->Settings->bEnableStandoneMode = InState == ECheckBoxState::Checked ? true : false;
					})
					.Content()
					[
						SNew(STextBlock)
							.Text(LOCTEXT("EnableStandalone_Text", "启动独立模式"))
					]
				]

				+ SVerticalBox::Slot().AutoHeight()[H_SEPATATOR]

				+ SVerticalBox::Slot().FIR_PADDING
				.VAlign(VAlign_Top)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("CPUAllocation_Text", "CPU分配"))
				]

				+ SVerticalBox::Slot().SEC_PADDING
				[
					SAssignNew(LimitCoreCheckBox, SCheckBox)
					.IsChecked_Lambda([this]() {
						return Settings->bLimitLogicCore ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
					})
					.OnCheckStateChanged_Lambda([this](const ECheckBoxState InState)
					{
						this->Settings->bLimitLogicCore = InState == ECheckBoxState::Checked ? true : false;
					})
					.Content()
					[
						SNew(STextBlock)
							.Text(LOCTEXT("LimitMaximum_Text", "将构建中能够使用的最大核心数量限制为"))
					]
				]

				+ SVerticalBox::Slot().SEC_PADDING
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot().AutoWidth()
					[
						SAssignNew(LimitMaxCoreBox, SNumericEntryBox<uint16>)
						.AllowSpin(true)
						.MinValue(0)
						.MaxValue(FPlatformMisc::NumberOfCores())
						.Value_Lambda([this]() {
							return Settings->LimitMaximumCoreNum;
						})
					]
					+ SHorizontalBox::Slot().AutoWidth().HAlign(HAlign_Left)
					[
						SNew(STextBlock)
						.Text(LOCTEXT("LogicCore_Text", "逻辑核心数"))
					]
				]
				+ SVerticalBox::Slot().SEC_PADDING
				[
					SNew(SBorder)
					.Padding(5.0f)
					.BorderBackgroundColor(FColor(255,0,0))
					.Visibility_Lambda([this] ()
					{
						return this->LimitCoreCheckBox->IsChecked() ? EVisibility::Visible : EVisibility::Collapsed;
					})
					[
						SNew(STextBlock)
						.Text(LOCTEXT("LimitCoreWarning_Text", "限制构建中使用的最大核心数量可能会降低构建性能.\n启动时不能低于机器的核心数"))
					]
				]
			]
		]
	];
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION

#undef LOCTEXT_NAMESPACE
