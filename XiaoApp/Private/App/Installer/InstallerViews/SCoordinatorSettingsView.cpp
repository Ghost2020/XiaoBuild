/**
  * @author cxx2020@outlook.com
  * @date 11:02 PM
 */
#include "SCoordinatorSettingsView.h"
#include "SlateOptMacros.h"
#include "Widgets/Input/SNumericEntryBox.h"
#include "XiaoStyle.h"
#include "ShareWidget.h"
#include "SInstallationFolderView.h"
#include "Widgets/Notifications/SErrorText.h"
#include "XiaoShareNetwork.h"


#define LOCTEXT_NAMESPACE "SCoordinatorSettingsView"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SCoordinatorSettingsView::Construct(const FArguments& InArgs)
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
				.Text(LOCTEXT("CoordinatorNetworkSettings_Text", "调度器网络设置"))
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
				.Text(LOCTEXT("CoordinatorServicePort_Text", "调度器服务端口"))
			]
			+SHorizontalBox::Slot().AutoWidth()
			[
				SAssignNew(CoordiServerPort, SNumericEntryBox<uint16>).MinDesiredValueWidth(45.0f)
				.Value_Lambda([]()
				{
					return GInstallSettings.CoordiListenPort;
				})
				.MinValue(1024).MaxValue(49151)
				.OnValueChanged_Lambda([this](const uint16 InPort)
				{
					GInstallSettings.CoordiListenPort = InPort;
					bCoordiServerPort = XiaoNetwork::IsPortAvailable(GInstallSettings.CoordiListenPort);
					if(bCoordiServerPort)
					{
						bCoordiServerPort = 
						/*GInstallSettings.CoordiListenPort != GInstallSettings.UIListenPort
						&&*/ GInstallSettings.CoordiListenPort != GInstallSettings.PerfTransport
						&& GInstallSettings.CoordiListenPort != GInstallSettings.AgentListenPort
						/*&& GInstallSettings.CoordiListenPort != GInstallSettings.LicenseListenPort*/
						&& GInstallSettings.CoordiListenPort != GInstallSettings.CacheListenPort;
					}
					ErrorText->SetError(bCoordiServerPort ? FText::GetEmpty() : Xiao::SSamePortError);
				})
			]
			+SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).HAlign(HAlign_Center)
			[
				SNew(SImage)
				.Image_Lambda([this]() 
				{
					return GetBrush(bCoordiServerPort);
				})
			]
		]
		/*+SVerticalBox::Slot().THR_PADDING
		[
			SNew(SHorizontalBox)
			+SHorizontalBox::Slot().MaxWidth(100.0f)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("XiaoUIManagerPort_Text", "前端管理端口"))
			]
			+SHorizontalBox::Slot().AutoWidth()
			[
				SAssignNew(XiaobuildManagerPort, SNumericEntryBox<uint16>).MinDesiredValueWidth(45.0f)
				.Value_Lambda([]()
				{
					return GInstallSettings.UIListenPort;
				})
				.MinValue(1024).MaxValue(49151)
				.OnValueChanged_Lambda([this](const uint16 InPort)
				{
						GInstallSettings.UIListenPort = InPort;
					bUIManagerPort = XiaoNetwork::IsPortAvailable(GInstallSettings.UIListenPort);
					if(bUIManagerPort)
					{
						bUIManagerPort = GInstallSettings.UIListenPort != GInstallSettings.CoordiListenPort
						&& GInstallSettings.UIListenPort != GInstallSettings.PerfTransport
						&& GInstallSettings.UIListenPort != GInstallSettings.AgentListenPort
						&& GInstallSettings.UIListenPort != GInstallSettings.LicenseListenPort
						&& GInstallSettings.UIListenPort != GInstallSettings.CacheListenPort;
					}
					ErrorText->SetError(bUIManagerPort ? FText::GetEmpty() : Xiao::SSamePortError);
				})
			]
			+SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).HAlign(HAlign_Center)
			[
				SNew(SImage)
				.Image_Lambda([this]()
				{
					return GetBrush(bUIManagerPort);
				})
			]
		]*/
		+SVerticalBox::Slot().THR_PADDING
		[
			SNew(SHorizontalBox)
			+SHorizontalBox::Slot().MaxWidth(100.0f)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("AgentCommunicationPort_Text", "代理传输端口"))
			]
			+SHorizontalBox::Slot().AutoWidth()
			[
				SAssignNew(AgentCommunicationPort, SNumericEntryBox<uint16>).MinDesiredValueWidth(45.0f)
				.MinValue(1024).MaxValue(49151)
				.Value_Lambda([]()
				{
					return GInstallSettings.AgentListenPort;
				})
				.OnValueChanged_Lambda([this](const uint16 InPort)
				{
						GInstallSettings.AgentListenPort = InPort;
					bAgentCommunicatePort = XiaoNetwork::IsPortAvailable(GInstallSettings.AgentListenPort);
					if(bAgentCommunicatePort)
					{
						bAgentCommunicatePort = GInstallSettings.AgentListenPort != GInstallSettings.CoordiListenPort
						/*&& GInstallSettings.AgentListenPort != GInstallSettings.UIListenPort*/
						&& GInstallSettings.AgentListenPort != GInstallSettings.PerfTransport
						/*&& GInstallSettings.AgentListenPort != GInstallSettings.LicenseListenPort*/
						&& GInstallSettings.AgentListenPort != GInstallSettings.CacheListenPort;
					}
					ErrorText->SetError(bAgentCommunicatePort ? FText::GetEmpty() : Xiao::SSamePortError);
				})
			]
			+SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).HAlign(HAlign_Center)
			[
				SNew(SImage)
				.Image_Lambda([this]()
				{
					return GetBrush(bAgentCommunicatePort);
				})
			]
		]
		/*+SVerticalBox::Slot().THR_PADDING
		[
			SNew(SHorizontalBox)
			+SHorizontalBox::Slot().MaxWidth(100.0f)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("LicenseServicePort_Text", "许可服务端口"))
			]
			+SHorizontalBox::Slot().AutoWidth()
			[
				SAssignNew(LicenseServicePort, SNumericEntryBox<uint16>).MinDesiredValueWidth(45.0f)
				.MinValue(1024).MaxValue(49151)
				.Value_Lambda([]()
				{
					return GInstallSettings.LicenseListenPort;
				})
				.OnValueChanged_Lambda([this](const uint16 InPort)
				{
						GInstallSettings.LicenseListenPort = InPort;
					bLicenseServicePort = XiaoNetwork::IsPortAvailable(GInstallSettings.LicenseListenPort);
					if(bLicenseServicePort)
					{
						bLicenseServicePort = GInstallSettings.LicenseListenPort != GInstallSettings.CoordiListenPort
						&& GInstallSettings.LicenseListenPort != GInstallSettings.UIListenPort
						&& GInstallSettings.LicenseListenPort != GInstallSettings.PerfTransport
						&& GInstallSettings.LicenseListenPort != GInstallSettings.AgentListenPort
						&& GInstallSettings.LicenseListenPort != GInstallSettings.CacheListenPort;
					}
					ErrorText->SetError(bLicenseServicePort ? FText::GetEmpty() : Xiao::SSamePortError);
				})
			]
			+SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).HAlign(HAlign_Center)
			[
				SNew(SImage)
				.Image_Lambda([this]()
				{
					return GetBrush(bLicenseServicePort);
				})
			]
		]*/
		+SVerticalBox::Slot().THR_PADDING
		[
			SNew(SHorizontalBox)
			+SHorizontalBox::Slot().AutoWidth()
			[
				SAssignNew(AutomaticallyOpenCheckBox, SCheckBox)
				.IsChecked(true)
				.OnCheckStateChanged_Lambda([](const ECheckBoxState InState)
				{
					GInstallSettings.bAutoOpenFirewall = InState == ECheckBoxState::Checked ? true : false;
				})
			]
			+SHorizontalBox::Slot().AutoWidth()
			[
				SNew(STextBlock).Text(LOCTEXT("AutomaticallyOpen_Text", "自动打开防火墙的端口"))
			]
		]
	];

	OnCheckButtonClicked();
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION

TWeakPtr<SWizardView> SCoordinatorSettingsView::GetNext()
{
	if (GInstallSettings.InstallType & EComponentTye::CT_Agent || GInstallSettings.InstallType & EComponentTye::CT_Coordinator)
	{
		if (!InstallationFolderView.IsValid())
		{
			InstallationFolderView = SNew(SInstallationFolderView, true);
		}
		return InstallationFolderView;
	}
	return nullptr;
}

bool SCoordinatorSettingsView::OnCanNext()
{
	return CheckCoordiPort();
}

bool SCoordinatorSettingsView::CheckCoordiPort() const
{
	return bCoordiServerPort /*&& bUIManagerPort*/ && bMessageTransPort && bAgentCommunicatePort/* && bLicenseServicePort*/;
}

void SCoordinatorSettingsView::OnCheckButtonClicked()
{
	bCoordiServerPort = XiaoNetwork::IsPortAvailable(GInstallSettings.CoordiListenPort);
	// bUIManagerPort = XiaoNetwork::IsPortAvailable(GInstallSettings.UIListenPort);
	bMessageTransPort = XiaoNetwork::IsPortAvailable(GInstallSettings.PerfTransport);
	bAgentCommunicatePort = XiaoNetwork::IsPortAvailable(GInstallSettings.AgentListenPort);
	// bLicenseServicePort = XiaoNetwork::IsPortAvailable(GInstallSettings.LicenseListenPort);
}

#undef LOCTEXT_NAMESPACE
