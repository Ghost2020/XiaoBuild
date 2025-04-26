/**
  * @author cxx2020@outlook.com
  * @date 11:02 PM
 */
#include "SAgentStatus.h"
#include "SlateOptMacros.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SSpacer.h"
#include "Widgets/Text/STextBlock.h"
#include "ShareDefine.h"

#define LOCTEXT_NAMESPACE "SAgentStatus"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SAgentStatus::Construct(const FArguments& InArgs, const FOnlineAgentActivity& InOnlineActivity, const FLicenseUsage& InLicenseUsage)
{
	ChildSlot
	[
		SNew(SBorder).HAlign(HAlign_Fill).VAlign(VAlign_Fill)
		[
			SNullWidget::NullWidget
		/*
			SNew(SHorizontalBox)

#pragma region OnlineAgentActivity
			+ SHorizontalBox::Slot().AutoWidth().HAlign(HAlign_Center)
			[
				SNew(SBorder).BorderBackgroundColor(FColor::Black).Clipping(EWidgetClipping::ClipToBounds)
				[
					SNew(SVerticalBox)
#pragma region ActivityHeader
					+SVerticalBox::Slot().AutoHeight()
					[
						SNew(SHorizontalBox)
						+SHorizontalBox::Slot().HAlign(HAlign_Center).VAlign(VAlign_Center)
						[
							SNew(SBorder).BorderImage(FCoreStyle::Get().GetBrush( "ToolPanel.GroupBorder"))
							[
								SNew(STextBlock).Text(LOCTEXT("OnlineAgentActivity_Text", "在线代理活动"))
								.TextStyle(&FXiaoStyle::XiaoH3TextStyle)
								.ToolTipText(LOCTEXT("OnlineAgentActivity_ToolTip", "A summary of your Incredibuild online agents. This includes agents with a valid license that are currently working or ready to participate in builds."))
							]
						]
					]
#pragma endregion 
#pragma region ActivityBody
					+SVerticalBox::Slot().AutoHeight()
					[
						SNew(SHorizontalBox)
#pragma region OnlineMachines
						+SHorizontalBox::Slot().AutoWidth()
						[
							SNew(SHorizontalBox)
							+SHorizontalBox::Slot()
							[
								SNew(SVerticalBox)
								+ SVerticalBox::Slot().Padding(10.0f)
								[
									SNew(SHorizontalBox)
									+SHorizontalBox::Slot().AutoWidth().Padding(2.0, 10.0, 20.0, 2.0)
									[
										SNew(STextBlock).Text(LOCTEXT("OnlineMachines_Text", "在线机器"))
										.TextStyle(&FXiaoStyle::XiaoH3TextStyle)
										.ToolTipText(LOCTEXT("OnlineMachines_ToolTip", "A breakdown of the total number of online agent machines, both on-Prem and cloud (if exists)."
"Busy Machines - This represents the machines that are online and currently participating in a build."
"Idle Machines - On-Prem machines that are available in your grid but aren’t participating in a build."))
									]
									+SHorizontalBox::Slot()
									[
										SAssignNew(OnlineMachineNumText, STextBlock).Text(LOCTEXT("OnlineMachines_Text", "1"))
										.TextStyle(&FXiaoStyle::XiaoH3TextStyle)
									]
								]
	
								+ SVerticalBox::Slot().Padding(10.0f)
								[
									SNew(SHorizontalBox)
									+SHorizontalBox::Slot().AutoWidth().Padding(2.0)
									[
										SNew(SImage).Image(FXiaoStyle::Get().GetBrush(TEXT("yellow_point")))
									]
		
									+SHorizontalBox::Slot().AutoWidth().Padding(2.0, 10.0, 20.0, 2.0)
									[
										SNew(STextBlock).Text(LOCTEXT("Busy_Text", "繁忙"))
									]
		
									+SHorizontalBox::Slot().AutoWidth().Padding(2.0, 10.0, 8.0, 2.0)
									[
										SAssignNew(BusyNumText, STextBlock)
									]
								]
	
								+ SVerticalBox::Slot().Padding(10.0f)
								[
									SNew(SHorizontalBox)
									+SHorizontalBox::Slot().AutoWidth()
									[
										SNew(SImage).Image(FXiaoStyle::Get().GetBrush(TEXT("green_point")))
									]
		
									+SHorizontalBox::Slot().AutoWidth().Padding(2.0, 10.0, 8.0, 10.0)
									[
										SNew(STextBlock).Text(LOCTEXT("Idle_Text", "闲置"))
									]
		
									+SHorizontalBox::Slot().Padding(2.0, 10.0, 8.0, 10.0)
									[
										SAssignNew(IdleNumText, STextBlock)
									]
								]
							]

							+SHorizontalBox::Slot().VAlign(VAlign_Center).HAlign(HAlign_Center)
							[
								SAssignNew(OnlineMachinePie, SPie)
								.BackgroundColor(FXiaoStyle::XiaoGrey).ForegroundColor(FColor::Yellow)
								.Percent(0.5f)
							]
						]
#pragma endregion
						+SHorizontalBox::Slot()[SNew(SBorder)].AutoWidth()
#pragma region BusyCores
						+SHorizontalBox::Slot().AutoWidth()
						[
							SNew(SHorizontalBox)
							+SHorizontalBox::Slot()
							[
								SNew(SVerticalBox)
								+ SVerticalBox::Slot().Padding(10.0f)
								[
									SNew(SHorizontalBox)
									+SHorizontalBox::Slot().AutoWidth().Padding(2.0, 10.0, 20.0, 2.0)
									[
										SNew(STextBlock).Text(LOCTEXT("BusyCores_Text", "繁忙核心"))
										.TextStyle(&FXiaoStyle::XiaoH3TextStyle)
										.ToolTipText(LOCTEXT("BusyCores_ToolTip", "The total number of busy cores currently working."
"Helper Cores - the number of remote cores that are currently assisting to builds, this includes both On-Prem and cloud cores."
"Initiator Cores - the total number of local cores, on the initiator machines, that are currently executing builds."))
									]
									+SHorizontalBox::Slot()
									[
										SAssignNew(BusyCoresNumText, STextBlock).Text(LOCTEXT("BusyCoresNum_Text", "0"))
										.TextStyle(&FXiaoStyle::XiaoH3TextStyle)
									]
								]
						
								+ SVerticalBox::Slot().Padding(10.0f)
								[
									SNew(SHorizontalBox)
									+SHorizontalBox::Slot().AutoWidth().Padding(2.0)
									[
										SNew(SImage).Image(FXiaoStyle::Get().GetBrush(TEXT("green_point")))
									]
							
									+SHorizontalBox::Slot().AutoWidth().Padding(2.0, 10.0, 20.0, 2.0)
									[
										SNew(STextBlock).Text(LOCTEXT("Helper_Text", "协助者"))
									]
							
									+SHorizontalBox::Slot().AutoWidth().Padding(2.0, 10.0, 8.0, 2.0)
									[
										SAssignNew(HelperNumText, STextBlock)
									]
								]
						
								+ SVerticalBox::Slot().Padding(10.0f)
								[
									SNew(SHorizontalBox)
									+SHorizontalBox::Slot().AutoWidth()
									[
										SNew(SImage).Image(FXiaoStyle::Get().GetBrush(TEXT("white_point")))
									]
							
									+SHorizontalBox::Slot().AutoWidth().Padding(2.0, 10.0, 8.0, 10.0)
									[
										SNew(STextBlock).Text(LOCTEXT("Initiator_Text", "发起者"))
									]
							
									+SHorizontalBox::Slot().Padding(2.0, 10.0, 8.0, 10.0)
									[
										SAssignNew(InitiatorNumText, STextBlock)
									]
								]
							]
							+SHorizontalBox::Slot().VAlign(VAlign_Center).HAlign(HAlign_Center)
							[
								SAssignNew(OnlineMachinePie, SPie)
								.BackgroundColor(FXiaoStyle::XiaoGreen).ForegroundColor(FColor::Yellow)
								.Percent(0.5f)
							]
						]
#pragma endregion
						+SHorizontalBox::Slot()[SNew(SBorder)].AutoWidth()
#pragma region Builds
						+SHorizontalBox::Slot().AutoWidth()
						[
							SNew(SHorizontalBox)
							+SHorizontalBox::Slot()
							[
								SNew(SVerticalBox)
								+ SVerticalBox::Slot().Padding(10.0f)
								[
									SNew(SHorizontalBox)
									+SHorizontalBox::Slot().AutoWidth().Padding(2.0, 10.0, 20.0, 2.0)
									[
										SNew(STextBlock).Text(LOCTEXT("Builds_Text", "构建数据"))
										.TextStyle(&FXiaoStyle::XiaoH3TextStyle)
										.ToolTipText(LOCTEXT("Builds_ToolTip", "The number of builds that are being executed at this moment."))
									]
									+SHorizontalBox::Slot()
									[
										SAssignNew(BuildsNumText, STextBlock).Text(LOCTEXT("BuildsNum_Text", "0"))
										.TextStyle(&FXiaoStyle::XiaoH3TextStyle)
									]
								]
							]
							+SHorizontalBox::Slot().VAlign(VAlign_Center).HAlign(HAlign_Center)
							[
								SNew(SConstrainBox).MinWidth(50.0f).MaxWidth(50.0f)
								[
									SAssignNew(BuildsPie, SPie)
									.BackgroundColor(FXiaoStyle::XiaoGreen).ForegroundColor(FColor::Yellow)
									.Percent(0.5f)
								]
							]
						]
#pragma endregion
						]
#pragma endregion 
					]
				]
#pragma endregion
#pragma region LicenseUsage  
				+ SHorizontalBox::Slot().AutoWidth().HAlign(HAlign_Center)
				[
					SNew(SBorder).BorderBackgroundColor(FColor::Black).Clipping(EWidgetClipping::ClipToBounds)
					[
						SNew(SVerticalBox)
#pragma region UsageHeader
						+SVerticalBox::Slot().AutoHeight()
						[
							SNew(SHorizontalBox)
							+SHorizontalBox::Slot().HAlign(HAlign_Center).VAlign(VAlign_Center)
							[
								SNew(SBorder).BorderImage(FCoreStyle::Get().GetBrush( "ToolPanel.GroupBorder" ))
								[
									SNew(STextBlock).Text(LOCTEXT("LicenseUsauge_Text", "许可状况"))
									.TextStyle(&FXiaoStyle::XiaoH3TextStyle)
									.ToolTipText(LOCTEXT("LicenseUsauge_ToolTip", "Displays how many licenses you have of each type, and how they are currently utilized."))
								]
							]
						]
#pragma endregion
#pragma region UsageBody
						+ SVerticalBox::Slot().AutoHeight()
						[
							SNew(SHorizontalBox)
#pragma region Initiators
						+SHorizontalBox::Slot()
						[
							SNew(SHorizontalBox)
							+ SHorizontalBox::Slot()
							[
								SNew(SVerticalBox)
								+ SVerticalBox::Slot().Padding(10.0f)
								[
									SNew(SHorizontalBox)
									+SHorizontalBox::Slot().AutoWidth().Padding(2.0, 10.0, 20.0, 2.0)
									[
										SNew(STextBlock).Text(LOCTEXT("Initiators_Text", "发起者"))
										.TextStyle(&FXiaoStyle::XiaoH3TextStyle)
										.ToolTipText(LOCTEXT("Initiators_ToolTip", "The total number of Initiator licenses."))
									]
									+SHorizontalBox::Slot()
									[
										SAssignNew(InitiatorsNumText, STextBlock).Text(LOCTEXT("InitiatorsNum_Text", "10"))
										.TextStyle(&FXiaoStyle::XiaoH3TextStyle)
									]
								]
		
								+ SVerticalBox::Slot().Padding(10.0f)
								[
									SNew(SHorizontalBox)
									+SHorizontalBox::Slot().AutoWidth().Padding(2.0)
									[
										SNew(SImage).Image(FXiaoStyle::Get().GetBrush(TEXT("green_point")))
									]
		
									+SHorizontalBox::Slot().AutoWidth().Padding(2.0, 10.0, 20.0, 2.0)
									[
										SNew(STextBlock).Text(LOCTEXT("Assigned_Text", "分配"))
										.ToolTipText(LOCTEXT("Assigned_ToolTip", "Licenses that are assigned to an Agent."))
									]
		
									+SHorizontalBox::Slot().AutoWidth().Padding(2.0, 10.0, 8.0, 2.0)
									[
										SAssignNew(AssignedNumText, STextBlock)
									]
								]
			
								+ SVerticalBox::Slot().Padding(10.0f)
								[
									SNew(SHorizontalBox)
									+SHorizontalBox::Slot().AutoWidth()
									[
										SNew(SImage).Image(FXiaoStyle::Get().GetBrush(TEXT("white_point")))
									]
		
									+SHorizontalBox::Slot().AutoWidth().Padding(2.0, 10.0, 8.0, 10.0)
									[
										SNew(STextBlock).Text(LOCTEXT("Avaiable_Text", "可用"))
										.ToolTipText(LOCTEXT("Avaiable_ToolTip", "Licenses that are not assigned to any Agent."))
									]
		
									+SHorizontalBox::Slot().Padding(2.0, 10.0, 8.0, 10.0)
									[
										SAssignNew(AvailableNumText, STextBlock)
									]
								]
							]
	
							+SHorizontalBox::Slot().VAlign(VAlign_Center).HAlign(HAlign_Center)
							[
								SNew(SConstrainBox).MinWidth(50.0f).MaxWidth(50.0f)
								[
									SAssignNew(InitiatorsPie, SPie)
									.BackgroundColor(FXiaoStyle::XiaoGreen).ForegroundColor(FColor::Yellow)
									.Percent(0.5f)
								]
							]
						]
#pragma endregion
						+SHorizontalBox::Slot()[SNew(SBorder)].AutoWidth()
#pragma region HelperCores
						+ SHorizontalBox::Slot().AutoWidth()
						[
							SNew(SHorizontalBox)
							+SHorizontalBox::Slot()
							[
								SNew(SVerticalBox)
								+ SVerticalBox::Slot().Padding(10.0f)
								[
									SNew(SHorizontalBox)
									+SHorizontalBox::Slot().AutoWidth().Padding(2.0, 10.0, 20.0, 2.0)
									[
										SNew(STextBlock).Text(LOCTEXT("HelperCores_Text", "协助核心"))
										.TextStyle(&FXiaoStyle::XiaoH3TextStyle)
										.ToolTipText(LOCTEXT("HelperCores_ToolTip", "The total number of Helper Core licenses."))
									]
									+SHorizontalBox::Slot()
									[
										SAssignNew(HelperCoresNumText, STextBlock).Text(LOCTEXT("HelperCoresNum_Text", "40"))
										.TextStyle(&FXiaoStyle::XiaoH3TextStyle)
									]
								]
	
								+ SVerticalBox::Slot().Padding(10.0f)
								[
									SNew(SHorizontalBox)
		
									+SHorizontalBox::Slot().AutoWidth().Padding(2.0, 10.0, 20.0, 2.0)
									[
										SNew(STextBlock).Text(LOCTEXT("HelperCoresPoolNumText_Text", "协助线程心池"))
										.ToolTipText(LOCTEXT("HelperCoresPoolNumText_ToolTip", "The current number of online Helper Cores that have been assigned a floating license. You can add as many Helper Cores to this pool as you want, and in general, the Helper Core Pool should be greater than the total number of Helper Core licenses."))
									]
		
									+SHorizontalBox::Slot().AutoWidth().Padding(2.0, 10.0, 8.0, 2.0)
									[
										SAssignNew(HelperCoresPoolNumText, STextBlock)
									]
								]
							]
						]
#pragma endregion 
					]
				]		
			]
		*/
#pragma endregion
#pragma endregion LIcenseUsage
		]
	];
	
	OnUpdate(InOnlineActivity, InLicenseUsage);
}

void SAgentStatus::OnUpdate(const FOnlineAgentActivity& InOnlineActivity, const FLicenseUsage& InLicenseUsage) const
{
	/*OnlineMachineNumText->SetText(FText::FromString(FString::FromInt(InOnlineActivity.OnlineMachineNum)));
	BusyNumText->SetText(FText::FromString(FString::FromInt(InOnlineActivity.BusyNum)));
	IdleNumText->SetText(FText::FromString(FString::FromInt(InOnlineActivity.IdleNum)));

	BusyCoresNumText->SetText(FText::FromString(FString::FromInt(InOnlineActivity.BusyCoresNum)));
	HelperNumText->SetText(FText::FromString(FString::FromInt(InOnlineActivity.HelperNum)));
	InitiatorNumText->SetText(FText::FromString(FString::FromInt(InOnlineActivity.InitiatorNum)));

	BuildsNumText->SetText(FText::FromString(FString::FromInt(InOnlineActivity.BuildBum)));

	InitiatorsNumText->SetText(FText::FromString(FString::FromInt(InOnlineActivity.InitiatorsNum)));
	AssignedNumText->SetText(FText::FromString(FString::FromInt(InOnlineActivity.AssignedNum)));
	AvailableNumText->SetText(FText::FromString(FString::FromInt(InOnlineActivity.AvailableNum)));

	HelperCoresNumText->SetText(FText::FromString(FString::FromInt(InOnlineActivity.InitiatorsNum)));
	HelperCoresPoolNumText->SetText(FText::FromString(FString::FromInt(InOnlineActivity.HelperCorePoolNum)));*/
}


END_SLATE_FUNCTION_BUILD_OPTIMIZATION

#undef LOCTEXT_NAMESPACE
