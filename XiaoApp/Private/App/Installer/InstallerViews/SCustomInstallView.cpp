/**
  * @author cxx2020@outlook.com
  * @date 11:02 PM
 */

#include "SCustomInstallView.h"
#include "SlateOptMacros.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SCheckBox.h"

#include "XiaoStyle.h"

#include "SConnect2AgentView.h"
#include "SCoordinatorSettingsView.h"
#include "SInstallProgressView.h"
#include "ShareWidget.h"

#define LOCTEXT_NAMESPACE "SCustomInstallView"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SCustomInstallView::Construct(const FArguments& InArgs)
{
	ChildSlot
	[
		SNew(SVerticalBox)
		+SVerticalBox::Slot().AutoHeight()
		[
			SNew(SVerticalBox)
			+SVerticalBox::Slot().FIR_PADDING
			[
				SNew(SVerticalBox)
				+SVerticalBox::Slot()
				[
					SNew(SHorizontalBox)
					+SHorizontalBox::Slot().AutoWidth()
					[
						SAssignNew(AgentCheckBox, SCheckBox)
						.IsChecked(false)
						.OnCheckStateChanged_Lambda([this](const ECheckBoxState InState)
						{
							if(InState == ECheckBoxState::Checked)
							{
								CoordinatorCheckBox->SetIsChecked(ECheckBoxState::Unchecked);
								BuildMonitorCheckBox->SetIsChecked(ECheckBoxState::Unchecked);
								BackUpCoordinatorCheckBox->SetIsChecked(ECheckBoxState::Unchecked);
								GInstallSettings.InstallType = EComponentTye::CT_Agent;
							}
							else
							{
								AgentCheckBox->SetIsChecked(true);
							}
						})
					]
					+SHorizontalBox::Slot().AutoWidth()
					[
						SNew(STextBlock).Text(LOCTEXT("IncredibuildAgent_Text", "Xiaobuild 代理"))
					]
				]
				+SVerticalBox::Slot().L_PADDING(20.0f)
				[
					SNew(STextBlock).Text(LOCTEXT("AddsYouMachine_Text", "将你的机器融入到构建集群中，加速你的工作流程"))
				]
			]

			+SVerticalBox::Slot().FIR_PADDING
			[
				SNew(SVerticalBox)
				+SVerticalBox::Slot()
				[
					SNew(SHorizontalBox)
					+SHorizontalBox::Slot().AutoWidth()
					[
						SAssignNew(CoordinatorCheckBox, SCheckBox)
						.IsChecked(false)
						.OnCheckStateChanged_Lambda([this](const ECheckBoxState InState)
						{
							if(InState == ECheckBoxState::Checked)
							{
								AgentCheckBox->SetIsChecked(ECheckBoxState::Unchecked);
								BuildMonitorCheckBox->SetIsChecked(ECheckBoxState::Unchecked);
								BackUpCoordinatorCheckBox->SetIsChecked(ECheckBoxState::Unchecked);
								GInstallSettings.InstallType = EComponentTye::CT_Coordinator;
							}
							else
							{
								CoordinatorCheckBox->SetIsChecked(true);
							}
						})
					]
					+SHorizontalBox::Slot().AutoWidth()
					[
						SNew(STextBlock).Text(LOCTEXT("IncredibuildCoordinator_Text", "Xiaobuild 调度器"))
					]
				]
				+SVerticalBox::Slot().L_PADDING(20.0f)
				[
					SNew(STextBlock).Text(LOCTEXT("ResponsibleForLissting_Text", "管理一个工作集群，每一个工作集群都需要有一个调度器来进行管理"))
				]
			]

			+SVerticalBox::Slot().FIR_PADDING
			[
				SNew(SVerticalBox)
				+SVerticalBox::Slot()
				[
					SNew(SHorizontalBox)
					+SHorizontalBox::Slot().AutoWidth()
					[
						SAssignNew(BuildMonitorCheckBox, SCheckBox)
						.IsChecked(false)
						.OnCheckStateChanged_Lambda([this](const ECheckBoxState InState)
						{
							if(InState == ECheckBoxState::Checked)
							{
								AgentCheckBox->SetIsChecked(ECheckBoxState::Unchecked);
								CoordinatorCheckBox->SetIsChecked(ECheckBoxState::Unchecked);
								BackUpCoordinatorCheckBox->SetIsChecked(ECheckBoxState::Unchecked);
								GInstallSettings.InstallType = EComponentTye::CT_Visulizer;
							}
							else
							{
								BuildMonitorCheckBox->SetIsChecked(true);
							}
						})
					]
					+SHorizontalBox::Slot().AutoWidth()
					[
						SNew(STextBlock).Text(LOCTEXT("IncredibuildMonitor_Text", "Xiaobuild 构建查看器"))
					]
				]
				+SVerticalBox::Slot().L_PADDING(20.0f)
				[
					SNew(STextBlock).Text(LOCTEXT("AllowsYouTo_Text", "允许你以可视化的方式查看你的构建进度"))
				]
			]

			+SVerticalBox::Slot().FIR_PADDING
			[
				SNew(SVerticalBox)
				+SVerticalBox::Slot()
				[
					SNew(SHorizontalBox)
					+SHorizontalBox::Slot().AutoWidth()
					[
						SAssignNew(BackUpCoordinatorCheckBox, SCheckBox)
						.IsChecked(true)
						.OnCheckStateChanged_Lambda([this](const ECheckBoxState InState)
						{
							if(InState == ECheckBoxState::Checked)
							{
								AgentCheckBox->SetIsChecked(ECheckBoxState::Unchecked);
								CoordinatorCheckBox->SetIsChecked(ECheckBoxState::Unchecked);
								BuildMonitorCheckBox->SetIsChecked(ECheckBoxState::Unchecked);
								GInstallSettings.InstallType = EComponentTye::CT_BackCoordi;
							}
							else
							{
								BackUpCoordinatorCheckBox->SetIsChecked(true);
							}
						})
					]
					+SHorizontalBox::Slot().AutoWidth()
					[
						SNew(STextBlock).Text(LOCTEXT("BackupCoordinator_Text", "Xiaobuild 备份调度器"))
					]
				]
				+SVerticalBox::Slot().L_PADDING(20.0f)
				[
					SNew(STextBlock).Text(LOCTEXT("SevesAsABackup_Text", "承担主要调度器的备份功能，当主要调度器不可用时，备份调度器将被激活承担相应的功能"))
				]
			]
		]
	];

	AgentCheckBox->SetIsChecked(ECheckBoxState::Checked);
	CoordinatorCheckBox->SetIsChecked(ECheckBoxState::Unchecked);
	BuildMonitorCheckBox->SetIsChecked(ECheckBoxState::Unchecked);
	BackUpCoordinatorCheckBox->SetIsChecked(ECheckBoxState::Unchecked);
	GInstallSettings.InstallType = EComponentTye::CT_Agent;
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION

TWeakPtr<SWizardView> SCustomInstallView::GetNext()
{
	if(GInstallSettings.InstallType & EComponentTye::CT_Agent || GInstallSettings.InstallType == EComponentTye::CT_BackCoordi)
	{
		if(!Connect2AgentView.IsValid())
		{
			Connect2AgentView = SNew(SConnect2AgentView);
		}
		return Connect2AgentView;
	}
	if(GInstallSettings.InstallType & EComponentTye::CT_Coordinator)
	{
		if(!CoordinatorSettingsView.IsValid())
		{
			CoordinatorSettingsView = SNew(SCoordinatorSettingsView);
		}
		return CoordinatorSettingsView;
	}
	else
	{
		if (!ProgressView.IsValid())
		{
			ProgressView = SNew(SInstallProgressView);
		}
		return ProgressView;
	}
}

#undef LOCTEXT_NAMESPACE