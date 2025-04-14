/**
  * @author cxx2020@outlook.com
  * @date 10:49 AM
 */

#include "SNetworkCoordinatorView.h"
#include "ShareDefine.h"
#include "SlateOptMacros.h"
#include "Widgets/Layout/SExpandableArea.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SSeparator.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SNumericEntryBox.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SComboBox.h"
#include "Widgets/Input/SEditableText.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Images/SThrobber.h"

#include "XiaoAgent.h"
#include "XiaoStyle.h"
#include "XiaoAppBase.h"
#include "XiaoShareField.h"

#define LOCTEXT_NAMESPACE "NetworkCoordinator"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SNetworkCoordinatorView::Construct(const FArguments& InArgs, FNetworkCoordinate* InSettings)
{
	Settings = InSettings;
	OnPressed = InArgs._OnTestPressed;
	
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
					.Text(LOCTEXT("CoordinatorLocation_Text", "协调器位置"))
				]

				+ SVerticalBox::Slot().SEC_PADDING
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					[
						SNew(STextBlock)
						.Text(LOCTEXT("Computer_Text", "主机"))
					]

					+ SHorizontalBox::Slot()
					[
						SAssignNew(IPBox, SEditableTextBox)
						.Text(FText::FromString(this->Settings->IP))
						.OnTextCommitted_Lambda([this](const FText& InText, ETextCommit::Type)
						{
							const FString IPString = InText.ToString();
							TArray<FString> Sections;
							if(IPString.ParseIntoArray(Sections, TEXT(".")) == 4)
							{
								if(Sections[0].IsNumeric() && Sections[1].IsNumeric() && Sections[2].IsNumeric() && Sections[3].IsNumeric())
								{
									this->Settings->IP = IPString;
								}
							}
							else if (IPString == TEXT("localhost"))
							{
								this->Settings->IP = IPString;
							}
						})
					]
				]

				+ SVerticalBox::Slot().SEC_PADDING
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					[
						SNew(STextBlock)
						.Text(LOCTEXT("Port_Text", "端口"))
					]

					+ SHorizontalBox::Slot()
					[
						SAssignNew(PortSpinBox, SSpinBox<uint16>)
						.OnValueChanged_Lambda([this](const uint16 InValue)
						{
							this->Settings->Port = InValue;
						})
						.Value_Lambda([this]() {
							return Settings->Port;
						})
					]
				]

				+ SVerticalBox::Slot().SEC_PADDING
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot().AutoWidth().HAlign(HAlign_Right)
					[
						SNew(SButton)
						.Text(LOCTEXT("Test_Text", "测试"))
						.IsEnabled_Lambda([this]() 
						{
							return bCanClickTestButton;
						})
						.OnPressed_Lambda([this] ()
						{
							bCanClickTestButton = false;
							this->OnPressed.Execute();
						})
					]
					+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).HAlign(HAlign_Center)
					[
						SAssignNew(NetworkStatus, SImage)
						.Image(FXiaoStyle::Get().GetBrush(TEXT("Port.Query")))
						.Visibility_Lambda([this]() 
						{
							return bCanClickTestButton ? EVisibility::Visible : EVisibility::Collapsed;
						})
					]
					+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).HAlign(HAlign_Center)
					[
						SNew(SThrobber)
						.Visibility_Lambda([this]()
						{
							return bCanClickTestButton ? EVisibility::Collapsed : EVisibility::Visible;
						})
					]

					+ SHorizontalBox::Slot().HAlign(HAlign_Right)
					[
						SNew(SButton)
						.Text(LOCTEXT("TestNetwork_Text", "系统网络连通性"))
						.OnPressed_Lambda([]()
							{
								RunXiaoApp(XiaoAppName::SBuildApp, FString::Printf(TEXT("-app=%s -network"), *XiaoAppName::SBuildMonitor, false, true, true, true));
							})
					]
				]
			]
		]
	];
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SNetworkCoordinatorView::Redraw()
{
	IPBox->SetText(FText::FromString(this->Settings->IP));
	PortSpinBox->SetValue(this->Settings->Port);
}

void SNetworkCoordinatorView::SetStatus(const int32 bInStatus) const
{
	if (FXiaoAppBase::GApp)
	{
		FXiaoAppBase::GApp->AddNextTickTask(FSimpleDelegate::CreateLambda([this, bInStatus]()
		{
			if (NetworkStatus.IsValid())
			{
				const FString BrushKey = (bInStatus == 0) ? TEXT("Port.OK") : (bInStatus == 1 ? TEXT("Port.Query") : TEXT("Port.NG"));
				NetworkStatus->SetImage(FXiaoStyle::Get().GetBrush(*BrushKey));
			}

			bCanClickTestButton = true;
		}));
	}
}

#undef LOCTEXT_NAMESPACE
