/**
  * @author cxx2020@outlook.com
  * @date 11:02 PM
 */
#include "SCertificateView.h"
#include "SlateOptMacros.h"
#include "SPrimaryButton.h"
#include "SSimpleButton.h"
#include "../Widgets/SConstrainBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Text/STextBlock.h"
#include "XiaoStyle.h"
#include "XiaoLog.h"

#define LOCTEXT_NAMESPACE "CertificateView"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SCertificateView::Construct(const FArguments& InArgs)
{
	XIAO_LOG(Log, TEXT("SCertificateView::Construct::Begin"));
	GLog->Flush();

	OnQueueNotification = InArgs._OnQueueNotification;

	ChildSlot
	[
		SNew(SBorder).Padding(50.0f)
		[
			SNew(SScrollBox).Orientation(Orient_Vertical)
			+ SScrollBox::Slot()
			[
			SNew(SHorizontalBox)
#pragma region LeftView
			+SHorizontalBox::Slot().AutoWidth()
			[
				SNew(SConstrainBox).MinWidth(200.0f).MaxWidth(200.0f)
				[
					SNew(SVerticalBox)
					+SVerticalBox::Slot().VAlign(VAlign_Top).AutoHeight()
					[
						SNew(SBorder).Padding(15.0f)
						[
							SNew(SVerticalBox)
							+SVerticalBox::Slot().VAlign(VAlign_Center)
							[
								SNew(SImage).Image(FXiaoStyle::Get().GetBrush(""))
							]
							+SVerticalBox::Slot().VAlign(VAlign_Center)
							[
								SNew(STextBlock).Text_Raw(this, &SCertificateView::GetLicenseType)
							]
						]
					]
					
					+SVerticalBox::Slot().AutoHeight()
					[
						SNew(SBorder).Padding(15.0f)
						[
							SNew(SVerticalBox)
							+SVerticalBox::Slot()
							[
								SNew(STextBlock)
								.Text(LOCTEXT("License_Text", "许可")).TextStyle(&XiaoH3TextStyle)
							]
		
							+SVerticalBox::Slot().Padding(15.0f)
							[
								SNew(SHorizontalBox)
								+SHorizontalBox::Slot().AutoWidth()
								[
									SNew(SOverlay)
									+SOverlay::Slot().HAlign(HAlign_Left)
									[
										SNew(SEditableTextBox)
										.Text_Raw(this, &SCertificateView::GetLicenseText)
										.IsEnabled(false)
									]
									+SOverlay::Slot().HAlign(HAlign_Right)
									[
										SNew(SPrimaryButton).Icon(FXiaoStyle::Get().GetBrush("Icons.copy"))
									]
								]
							]
	
							+SVerticalBox::Slot().Padding(15.0f)
							[
								SNew(SVerticalBox)
								+SVerticalBox::Slot().AutoHeight()
								[
									SNew(SHorizontalBox)
									+SHorizontalBox::Slot()
									[
										SNew(STextBlock)
										.Text(LOCTEXT("LastUpdate_Text", "最近更新"))
									]
								]
								+SVerticalBox::Slot().AutoHeight()
								[
									SNew(STextBlock)
									.Text_Raw(this, &SCertificateView::GetLastUpdate)
								]
	
								+SVerticalBox::Slot().AutoHeight()
								[
									SNew(SConstrainBox).MinWidth(300.0f).MaxWidth(300.0f)
									[
										SNew(SSimpleButton)
										.Text(LOCTEXT("UpdateLicenseNow_Text", "更新许可"))
										.Icon(FXiaoStyle::Get().GetBrush("update"))
										.OnClicked_Lambda([]()
										{
											return FReply::Handled();
										})
									]
								]
							]
	
							+SVerticalBox::Slot().Padding(15.0f)
							[
								SNew(SVerticalBox)
								+SVerticalBox::Slot().AutoHeight()
								[
									SNew(STextBlock)
									.Text(LOCTEXT("ExpirationDate_Text", "过期日期"))
								]
								+SVerticalBox::Slot().AutoHeight()
								[
									SNew(STextBlock)
									.Text_Raw(this, &SCertificateView::GetExpirationDate)
								]
							]
						]
					]
					
					+SVerticalBox::Slot().FillHeight(0.8f).VAlign(VAlign_Bottom)
					[
						SNew(SBorder).Padding(20.0f)
						[
							SNew(SVerticalBox)
							+SVerticalBox::Slot().VAlign(VAlign_Top).Padding(10.0f)
							[
								SNew(STextBlock)
								.Text(LOCTEXT("Deactivate_Text", "停用")).TextStyle(&XiaoH3TextStyle)
							]
		
							+SVerticalBox::Slot().VAlign(VAlign_Top).Padding(10.0f)
							[
								SNew(STextBlock)
								.Text(LOCTEXT("DeactivateDescribe_TExt", "停用这台机器的许可,就能留出位置给其他机器使用"))
								.AutoWrapText(true)
							]
		
							+SVerticalBox::Slot().HAlign(HAlign_Right).AutoHeight().Padding(10.0f)
							[
								SNew(SButton)
								.Text(LOCTEXT("DeactivateButton_TExt", "停用"))
								.OnClicked_Lambda([]()
								{
									return FReply::Handled();
								})
							]
						]
					]
				]
			]
#pragma endregion LeftView
#pragma region RightView
			+SHorizontalBox::Slot().HAlign(HAlign_Fill)
			[
				SNew(SBorder).Padding(15.0f)
				[
					SNew(SVerticalBox)
					+SVerticalBox::Slot().FillHeight(0.35f)
					[
						SNew(SHorizontalBox)
#pragma region Initiator
						+SHorizontalBox::Slot().AutoWidth()
						[
							SNew(SVerticalBox)
							+SVerticalBox::Slot().VAlign(VAlign_Top).HAlign(HAlign_Center).Padding(10.0f).AutoHeight()
							[
								SNew(STextBlock)
								.Text(LOCTEXT("Initiators_Text", "发起者")).TextStyle(&XiaoH3TextStyle)
							]
							+SVerticalBox::Slot().VAlign(VAlign_Center).HAlign(HAlign_Center).Padding(10.0f).AutoHeight()
							[
								SNew(STextBlock)
								.Text_Raw(this, &SCertificateView::GetInitiatorsNum)
							]
							+SVerticalBox::Slot().VAlign(VAlign_Bottom).HAlign(HAlign_Center).Padding(10.0f).AutoHeight()
							[
								SNew(SHorizontalBox)
								+SHorizontalBox::Slot().AutoWidth()
								[
									SNew(SImage)
									.Image(FXiaoStyle::Get().GetBrush("Icons.pin"))
								]
								+SHorizontalBox::Slot()
								[
									SNew(STextBlock).Text(LOCTEXT("Fixed_Text", "固定的"))
								]
							]
						]
#pragma endregion Initiator
						+SHorizontalBox::Slot()[SNew(SBorder)].AutoWidth()
#pragma region HelperCores
						+SHorizontalBox::Slot().AutoWidth()
						[
							SNew(SVerticalBox)
							+SVerticalBox::Slot().VAlign(VAlign_Top).HAlign(HAlign_Center).Padding(10.0f).AutoHeight()
							[
								SNew(STextBlock)
								.Text(LOCTEXT("HelperCores_Text", "协助核心")).TextStyle(&XiaoH3TextStyle)
							]
							+SVerticalBox::Slot().VAlign(VAlign_Center).HAlign(HAlign_Center).Padding(10.0f).AutoHeight()
							[
								SNew(STextBlock)
								.Text_Raw(this, &SCertificateView::GetHelperCoreNum)
							]
							+SVerticalBox::Slot().VAlign(VAlign_Bottom).HAlign(HAlign_Center).Padding(10.0f).AutoHeight()
							[
								SNew(SHorizontalBox)
								+SHorizontalBox::Slot().AutoWidth()
								[
									SNew(SImage)
									.Image(FXiaoStyle::Get().GetBrush("Icons.switch"))
								]
								+SHorizontalBox::Slot()
								[
									SNew(STextBlock).Text(LOCTEXT("Floating_Text", "浮动的"))
								]
							]
						]
#pragma endregion HelperCores
					]
					+SVerticalBox::Slot().VAlign(VAlign_Fill)
					[
						SNew(SBorder)
					]
				]
			]
		]
#pragma endregion RightView
	]
	];

	XIAO_LOG(Log, TEXT("SCertificateView::Construct::Finish"));
	GLog->Flush();
}

void SCertificateView::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{

}

FText SCertificateView::GetLicenseType() const
{
	return LOCTEXT("LicenseFree_Text", "免费使用");
}

FText SCertificateView::GetLicenseText() const
{
	return FText::FromString(TEXT("H3LS-U2UF-YKWJ-YFIN"));
}

FText SCertificateView::GetLastUpdate() const
{
	return FText::FromString(FDateTime::Now().ToString());
}

FText SCertificateView::GetExpirationDate() const
{
	return FText::FromString(TEXT("2028/12/12"));
}

FText SCertificateView::GetInitiatorsNum() const
{
	return FText::FromString(FString::FromInt(InitiatorNum));
}

FText SCertificateView::GetHelperCoreNum() const
{
	return FText::FromString(FString::FromInt(HelperCoreNum));
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION

#undef LOCTEXT_NAMESPACE
