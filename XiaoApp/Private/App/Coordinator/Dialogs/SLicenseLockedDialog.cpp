/**
 * @author cxx2020@outlook.com
 * @date 5:57 PM
 */
#include "SLicenseLockedDialog.h"
#include "SlateOptMacros.h"
#include "SSimpleButton.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Text/STextBlock.h"
#include "XiaoStyle.h"

#define LOCTEXT_NAMESPACE "SLicenseLockedDialog"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SLicenseLockedDialog::Construct(const FArguments& InArgs)
{
	SWindow::Construct(SWindow::FArguments()
	.bDragAnywhere(false)
	.CreateTitleBar(false)
	.HasCloseButton(false)
	.SupportsMaximize(false)
	.SupportsMaximize(false)
	.SizingRule(ESizingRule::FixedSize)
	.AutoCenter(EAutoCenter::PreferredWorkArea)
	.Title(LOCTEXT("LicenseState_Title", "许可锁定"))
	.ClientSize(FVector2D(500, 700))
	[
		SNew(SVerticalBox)

		+SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).VAlign(VAlign_Center).Padding(50.0f)
		[
			SNew(SImage).Image(FXiaoStyle::Get().GetBrush("Icons.LicenseLocked"))
		]

		+SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).VAlign(VAlign_Center).Padding(50.0f, 10.0f)
		[
			SNew(STextBlock).TextStyle(&XiaoH2TextStyle).Text(LOCTEXT("LicenseLocked_Text", "许可锁定"))
		]

		+SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).VAlign(VAlign_Center).Padding(50.0f, 10.0f)
		[
			SNew(STextBlock).Text(LOCTEXT("LicenseLockedDescribe_Text", "我们无法连接到许可服务器进行许可验证，请确定所有的组件都能够互相通信"))
			.AutoWrapText(true)
		]

		+SVerticalBox::Slot().Padding(30.0f).HAlign(HAlign_Center).VAlign(VAlign_Bottom)
		[
			SNew(SHorizontalBox)
			+SHorizontalBox::Slot().HAlign(HAlign_Center).VAlign(VAlign_Center)
			[
				SNew(STextBlock).Text(LOCTEXT("IfThisIssure_Text", "如果上述问题解决了,"))
			]
			+SHorizontalBox::Slot().HAlign(HAlign_Center).VAlign(VAlign_Center)
			[
				SNew(SSimpleButton).Text(LOCTEXT("RefreshNow_Text", "请立即刷新")).Icon(FXiaoStyle::Get().GetBrush("Icons.Refresh"))
				.OnClicked_Lambda([]()
				{
					return FReply::Handled();
				})
			]
		]
	]);
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION

#undef LOCTEXT_NAMESPACE
