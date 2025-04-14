// Fill out your copyright notice in the Description page of Project Settings.

#include "SLicenseValidationFailed.h"
#include "SlateOptMacros.h"
#include "SSimpleButton.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Input/SHyperlink.h"
#include "Widgets/Text/STextBlock.h"
#include "XiaoStyle.h"

#define LOCTEXT_NAMESPACE "SLicenseValidationFailed"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SLicenseValidationFailed::Construct(const FArguments& InArgs)
{
	// TODO TIXME Alt+F4 可以关掉当前的窗口
	SWindow::Construct(SWindow::FArguments()
	.bDragAnywhere(false)
	.CreateTitleBar(false)
	.HasCloseButton(true)
	.SupportsMaximize(false)
	.SupportsMaximize(false)
	.SizingRule(ESizingRule::FixedSize)
	.AutoCenter(EAutoCenter::PreferredWorkArea)
	.Title(LOCTEXT("LicenseValidationFailed_Title", "许可验证失败"))
	.ClientSize(FVector2D(500, 700))
	[
		SNew(SVerticalBox)

		+SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).VAlign(VAlign_Center).Padding(50.0f)
		[
			SNew(SImage).Image(FXiaoStyle::Get().GetBrush("Icons.LicenseExclamation"))
		]

		+SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).VAlign(VAlign_Center).Padding(50.0f, 10.0f)
		[
			SNew(STextBlock).TextStyle(&XiaoH2TextStyle).Text(LOCTEXT("LicenseExclamation_Text", "许可验证失败"))
		]

		+SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).VAlign(VAlign_Center).Padding(50.0f, 10.0f)
		[
			SNew(SHorizontalBox)
			+SHorizontalBox::Slot().HAlign(HAlign_Center).VAlign(VAlign_Center)
			[
				SNew(STextBlock).Text(LOCTEXT("LicenseValidationFailedDescribe_Text", "检测到问题需要对许可进行验证,请再次尝试或者联系"))
				.AutoWrapText(true)
			]
			+SHorizontalBox::Slot().HAlign(HAlign_Center).VAlign(VAlign_Center)
			[
				SNew(SHyperlink).Text(LOCTEXT("Suport_Text", "support@xiaobuild.com"))
				.OnNavigate_Lambda([]()
				{
					// TODO 
					FPlatformProcess::LaunchURL(TEXT("https://xiaobuild.com"), nullptr, nullptr);
				})
			]
		]

		+SVerticalBox::Slot().Padding(20.0f).HAlign(HAlign_Center)
		[
			SNew(SButton).Text(LOCTEXT("Retry_Text", "重试"))
			.OnClicked_Lambda([]()
			{
				return FReply::Handled();
			})
		]

		+SVerticalBox::Slot().Padding(20.0f).HAlign(HAlign_Center)
		[
			SNew(SButton).Text(LOCTEXT("ErrorDetails_Text", "错误细节"))
			.OnClicked_Lambda([]()
			{
				return FReply::Handled();
			})
		]
	]);
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION

#undef LOCTEXT_NAMESPACE
