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


#define ADD_PORT_BOX(DISPLAY_TEXT, PORT, PORT_FLAG) \
	+ SVerticalBox::Slot().THR_PADDING \
	[ \
		SNew(SHorizontalBox) \
		+ SHorizontalBox::Slot().MaxWidth(100.0f) \
		[ \
			SNew(STextBlock) \
			.Text(DISPLAY_TEXT) \
		] \
		+ SHorizontalBox::Slot().AutoWidth() \
		[ \
			SNew(SNumericEntryBox<uint16>).MinDesiredValueWidth(45.0f) \
			.Value_Lambda([](){ \
				return GInstallSettings.PORT; \
			}) \
			.MinValue(1024).MaxValue(49151) \
			.OnValueChanged_Lambda([this](const uint16 InPort){ \
				GInstallSettings.PORT = InPort; \
				PORT_FLAG = XiaoNetwork::IsPortAvailable(GInstallSettings.PORT); \
				if (PORT_FLAG) \
				{ \
					PORT_FLAG = PortIsSafe(); \
				} \
				ErrorText->SetError(PORT_FLAG ? FText::GetEmpty() : Xiao::SSamePortError); \
			}) \
		] \
		+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).HAlign(HAlign_Center) \
		[ \
			SNew(SImage) \
			.Image_Lambda([this]() { \
				return GetBrush(PORT_FLAG); \
			}) \
		] \
	]



BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SCoordinatorSettingsView::Construct(const FArguments& InArgs)
{
	Ports = { GInstallSettings.CoordiListenPort, GInstallSettings.AgentListenPort, GInstallSettings.CacheServicePort };

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

		ADD_PORT_BOX(LOCTEXT("CoordinatorServicePort_Text", "调度服务端口"), CoordiListenPort, bCoordiServerPort)
		ADD_PORT_BOX(LOCTEXT("AgentCommunicationPort_Text", "代理传输端口"), AgentListenPort, bAgentCommunicatePort)
		ADD_PORT_BOX(LOCTEXT("CacheServicePort_Text", "缓存服务端口"), CacheServicePort, bCacheServicePort)
		
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
	return bCoordiServerPort && bAgentCommunicatePort && bCacheServicePort;
}

void SCoordinatorSettingsView::OnCheckButtonClicked()
{
	bCoordiServerPort = XiaoNetwork::IsPortAvailable(GInstallSettings.CoordiListenPort);
	bAgentCommunicatePort = XiaoNetwork::IsPortAvailable(GInstallSettings.AgentListenPort);
	bCacheServicePort = XiaoNetwork::IsPortAvailable(GInstallSettings.CacheServicePort);
}

bool SCoordinatorSettingsView::PortIsSafe() const
{
	for (int i = 0; i < Ports.Num(); ++i)
	{
		for (int j = i + 1; j < Ports.Num(); j++)
		{
			if (Ports[i] == Ports[j])
			{
				return false;
			}
		}
	}

	return true;
}

#undef ADD_PORT_BOX
#undef LOCTEXT_NAMESPACE
