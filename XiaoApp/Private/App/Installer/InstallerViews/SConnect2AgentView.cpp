/**
  * @author cxx2020@outlook.com
  * @date 11:02 PM
 */
#include "SConnect2AgentView.h"

#include "SAgentSettingsView.h"
#include "SInstallProgressView.h"
#include "SlateOptMacros.h"
#include "Widgets/Input/SButton.h"
#include "Interfaces/IPv4/IPv4Address.h"

#include "ShareWidget.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Input/SNumericEntryBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Dialogs/SMessageWindow.h"
#include "XiaoStyle.h"
#include "XiaoShareRedis.h"
#include "XiaoAppBase.h"

#define LOCTEXT_NAMESPACE "SConnect2AgentView"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SConnect2AgentView::Construct(const FArguments& InArgs)
{
	ChildSlot
	[
		SNew(SVerticalBox)
		TOP_WIDGET
	
		+SVerticalBox::Slot().AutoHeight().L_PADDING(20.0f)
		[
			SNew(STextBlock)
			.Text_Lambda([]()
			{
				const FText AgentText = LOCTEXT("ConnectAgent_Text", "将当前的代理与已安装的协调器建立连接");
				const FText BackText = LOCTEXT("ConnectMaster_Text", "将当前的从服务器与已安装的主协调器建立连接");
				return GInstallSettings.InstallType == EComponentTye::CT_BackCoordi ? BackText : AgentText;
			})
		]
		+SVerticalBox::Slot().AutoHeight().L_PADDING(20.0f)
		[
			SNew(STextBlock).Text(LOCTEXT("IfYouHave_Text", "如果已经有安装好的协调器,确认协调器的IP地址和端口"))
		]
		+SVerticalBox::Slot().AutoHeight()
		[
			SNew(SBorder)
		]

		+SVerticalBox::Slot().SEC_PADDING
		[
			SNew(SHorizontalBox)
			+SHorizontalBox::Slot().AutoWidth()
			[
				SNew(SVerticalBox)
				+SVerticalBox::Slot().AutoHeight()
				[
					SNew(STextBlock).Text(LOCTEXT("NetWorkName_Text", "网络地址"))
				]
				+SVerticalBox::Slot()
				[
					SNew(SHorizontalBox)
					+SHorizontalBox::Slot()
					[
						SAssignNew(NetworkIpText, SEditableTextBox).MinDesiredWidth(450.0f)
						.HintText(FText::FromString(TEXT("localhost")))
						.Text(FText::FromString(TEXT("localhost")))
						.OnTextCommitted_Lambda([this](const FText& InText, ETextCommit::Type InType)
						{
							const FString IPV4Str = InText.ToString();
							NetworkIpText->SetError(FText::GetEmpty());
							FIPv4Address IPV4;
							if (!FIPv4Address::Parse(IPV4Str, IPV4))
							{
								NetworkIpText->SetError(LOCTEXT("NoValidIPV4Address_Text", "不是有效的IPV4地址"));
								GInstallSettings.CoordiIp = TEXT("");
								bCanClickTestButton = false;
								return;
							}
							GInstallSettings.CoordiIp = IPV4Str;
							bCanClickTestButton = true;
						})
					]
				]
			]
			+SHorizontalBox::Slot().L_PADDING(30.0f)
			[
				SNew(SVerticalBox)
				+SVerticalBox::Slot()
				[
					SNew(STextBlock).Text(FText::FromString("TCP/IP port"))
				]
				+SVerticalBox::Slot()
				[
					SAssignNew(PortBox, SNumericEntryBox<uint16>)
					.MinValue(1024).MaxValue(49151)
					.Value_Lambda([]() 
					{
						return GInstallSettings.CoordiPort;
					})
					.OnValueChanged_Lambda([this](const uint16 InPort)
					{
						GInstallSettings.CoordiPort = InPort;
					})
				]
			]
		]

		+ SVerticalBox::Slot().AutoHeight()
		[
			SNew(SErrorText)
				.ErrorText(LOCTEXT("ServerAlreadyExist_Text", "当前机器上已经安装了协调器"))
				.Visibility_Lambda([]()
					{
						if (GInstallSettings.InstallType == EComponentTye::CT_BackCoordi)
						{
							return IsAppRunning(XiaoAppName::SBuildCoordiService) ? EVisibility::Visible : EVisibility::Collapsed;
						}
						return EVisibility::Collapsed;
					})
		]

		+SVerticalBox::Slot().SEC_PADDING
		[
			SNew(SHorizontalBox)
			+SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
			[
				SNew(STextBlock).Text(LOCTEXT("Connect2Exist_Text", "与已安装的协调器进行连接"))
			]
			+SHorizontalBox::Slot().AutoWidth().HAlign(HAlign_Left)
			[
				SNew(SButton).Text(LOCTEXT("ConnectTest_Text", "连接测试"))
				.OnPressed_Lambda([this]()
				{
					AsyncNetworkTest();
				})
				.IsEnabled_Lambda([this]() 
				{
					return bCanClickTestButton;
				})
			]
			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).HAlign(HAlign_Center)
			[
				SAssignNew(NetworkStatus, SImage)
				.Image_Lambda([this]()
				{
					return GetBrush(bConnect);
				})
			]
		]

		+SVerticalBox::Slot().HAlign(HAlign_Center).VAlign(VAlign_Center)
		[
			SNew(SImage)
			.Image_Lambda([]()
			{
				return FXiaoStyle::Get().GetBrush(GInstallSettings.InstallType == EComponentTye::CT_BackCoordi ? TEXT("Installer/Slave") : TEXT("Installer/Agent"));
			})
		]
	];
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION

TWeakPtr<SWizardView> SConnect2AgentView::GetNext()
{
	if (GInstallSettings.InstallType == EComponentTye::CT_BackCoordi)
	{
		if (!InstallProgressView.IsValid())
		{
			InstallProgressView = SNew(SInstallProgressView);
		}
		return InstallProgressView;
	}
	else
	{
		if (!AgentSettingsView.IsValid())
		{
			AgentSettingsView = SNew(SAgentSettingsView);
		}
		return AgentSettingsView;
	}
}

bool SConnect2AgentView::OnCanNext()
{
	if (!GInstallSettings.CoordiIp.IsEmpty())
	{
		if (GInstallSettings.InstallType == EComponentTye::CT_BackCoordi)
		{
			return !IsAppRunning(XiaoAppName::SBuildCoordiService);
		}
		return true;
	}
	return false;
}

void SConnect2AgentView::AsyncNetworkTest()
{
	bCanClickTestButton = false;
	bConnect = false;

	AsyncThread([this]()
	{
		try
		{
			ConnectionOptions Options;
			Options.host = TCHAR_TO_UTF8(*GInstallSettings.CoordiIp);
			Options.port = GInstallSettings.CoordiPort;
			Options.keep_alive = false;
			Options.db = 0;
			Redis Connect(Options);
			const std::string Msg = Connect.ping().c_str();
			bCanClickTestButton = true;
			if (Msg == XiaoRedis::Key::SPong)
			{
				bConnect = true;
				return;
			}
		}
		catch (const sw::redis::Error& Ex)
		{
			XIAO_LOG(Error, TEXT("sw::redis::Error::%s!"), UTF8_TO_TCHAR(Ex.what()));
		}
		bConnect = false;
		bCanClickTestButton = true;

		FXiaoAppBase::GApp->AddNextTickTask(FSimpleDelegate::CreateLambda([this]()
			{
				const auto Window = SNew(SMessageWindow)
					.TiTile(XiaoError::SConnectToCoordinatorTitle)
					.Message(XiaoError::SConnectToCoordinatorMessage);

				FSlateApplication::Get().AddModalWindow(Window, SharedThis(this), false);
			}));
	});
}

#undef LOCTEXT_NAMESPACE
