/**
  * @author cxx2020@outlook.com
  * @date 11:02 PM
 */
#include "SAgentSettingsView.h"
#include "SlateOptMacros.h"
#include "Widgets/Input/SNumericEntryBox.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Notifications/SErrorText.h"
#include "XiaoStyle.h"
#include "ShareWidget.h"
#include "XiaoShare.h"
#include "XiaoShareNetwork.h"
#include "SInstallationFolderView.h"

#define LOCTEXT_NAMESPACE "SAgentSettingsView"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SAgentSettingsView::Construct(const FArguments& InArgs)
{
	ChildSlot
	[
		SNew(SVerticalBox)
		TOP_WIDGET

		+SVerticalBox::Slot().AutoHeight()
		[
			SAssignNew(ErrorText, SErrorText)
		]
	
		+SVerticalBox::Slot().AutoHeight()
		[
			SNew(SVerticalBox)
			+SVerticalBox::Slot().FIR_PADDING
			[
				SNew(STextBlock)
				.Text(LOCTEXT("AgentSettings_Text", "代理设置"))
			]
		]

		+SVerticalBox::Slot().AutoHeight()
		[
			SNew(SBorder)
		]

		+SVerticalBox::Slot().THR_PADDING
		[
			SNew(SHorizontalBox)
			+SHorizontalBox::Slot().MaxWidth(100.0f)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("AgentServicePort_Text", "代理服务端口"))
			]
			+SHorizontalBox::Slot().AutoWidth()
			[
				SAssignNew(AgentServicePort, SNumericEntryBox<uint16>).MinDesiredValueWidth(45.0f)
				.MinValue(1024).MaxValue(49151)
				.Value_Lambda([]()
				{
					return GInstallSettings.AgentListenPort;
				})
				.OnValueChanged_Lambda([this](const uint16 InPort)
				{
					GInstallSettings.HelpListenPort = GInstallSettings.AgentListenPort = InPort;
					bAgentServerPort = XiaoNetwork::IsPortAvailable(GInstallSettings.AgentListenPort);
					ErrorText->SetError(bAgentServerPort ? FText::GetEmpty() : Xiao::SSamePortError);
					OnCheckButtonClicked();
				})
			]
			+SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
			[
				SNew(SImage)
				.Image_Lambda([this]()
				{
					return GetBrush(bAgentServerPort);
				})
			]
		]

		+ SVerticalBox::Slot().THR_PADDING
		[
			SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().MaxWidth(100.0f)
				[
					SNew(STextBlock)
						.Text(LOCTEXT("SchedulerServerPort_Text", "调度服务端口"))
				]
				+ SHorizontalBox::Slot().AutoWidth()
				[
					SAssignNew(AgentServicePort, SNumericEntryBox<uint16>).MinDesiredValueWidth(45.0f)
						.MinValue(1024).MaxValue(49151)
						.Value_Lambda([]()
							{
								return GInstallSettings.SchedulerServerPort;
							})
						.OnValueChanged_Lambda([this](const uint16 InPort)
							{
								GInstallSettings.HelpListenPort = GInstallSettings.AgentListenPort = InPort;
								bSchedulerServerPort = XiaoNetwork::IsPortAvailable(GInstallSettings.SchedulerServerPort);
								ErrorText->SetError(bSchedulerServerPort ? FText::GetEmpty() : Xiao::SSamePortError);
								OnCheckButtonClicked();
							})
				]
				+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
				[
					SNew(SImage)
						.Image_Lambda([this]()
							{
								return GetBrush(bSchedulerServerPort);
							})
				]
		]

		+SVerticalBox::Slot().THR_PADDING
		[
			SNew(SHorizontalBox)
			+SHorizontalBox::Slot().AutoWidth()
			[
				SAssignNew(AutomaticallyOpenCheckBox, SCheckBox)
				.IsChecked(true)
				.IsEnabled(false)
				.OnCheckStateChanged_Lambda([](const ECheckBoxState InState)
				{
					GInstallSettings.bAutoOpenFirewall = InState == ECheckBoxState::Checked ? true : false;
				})
			]
			+SHorizontalBox::Slot().AutoWidth()
			[
				SNew(STextBlock).Text(LOCTEXT("AutomaticallyOpen_Text", "根据Windows防火墙自动打开需要的端口"))
			]
		]

		+SVerticalBox::Slot().FIR_PADDING
		[
			SNew(STextBlock).Text(LOCTEXT("FileCache_Text", "文件缓存"))
		]

		+SVerticalBox::Slot().THR_PADDING
		[
			SNew(STextBlock).Text(LOCTEXT("MaximumSize_Text", "UBA代理最大文件缓存大小"))
		]
		+SVerticalBox::Slot().THR_PADDING
		[
			SNew(SHorizontalBox)
			+SHorizontalBox::Slot().AutoWidth()
			[
				SAssignNew(MaxFileSize, SNumericEntryBox<uint32>).MinDesiredValueWidth(1024.0f)
				.Value_Lambda([]()
				{
					return GInstallSettings.FileCache;
				})
				.OnValueChanged_Lambda([](const uint32 InValue)
				{
					GInstallSettings.FileCache = InValue;
				})
			]
			+SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
			[
				SNew(STextBlock).Text(FText::FromString(TEXT("MB")))
			]
		]
	];

	OnCheckButtonClicked();
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION

TWeakPtr<SWizardView> SAgentSettingsView::GetNext()
{
	if(!InstallationFolderView.IsValid())
	{
		InstallationFolderView = SNew(SInstallationFolderView, false);
	}
	return InstallationFolderView;
}

bool SAgentSettingsView::OnCanNext()
{
	return bAgentServerPort && bSchedulerServerPort && bHelperPort;
}

void SAgentSettingsView::OnCheckButtonClicked()
{
	bAgentServerPort = XiaoNetwork::IsPortAvailable(GInstallSettings.AgentListenPort);
	bSchedulerServerPort = XiaoNetwork::IsPortAvailable(GInstallSettings.SchedulerServerPort);
	bHelperPort = XiaoNetwork::IsPortAvailable(GInstallSettings.HelpListenPort);
}

#undef LOCTEXT_NAMESPACE
