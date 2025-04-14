/**
  * @author cxx2020@outlook.com
  * @date 10:49 AM
 */

#include "SWarningBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Images/SImage.h"
#include "XiaoStyle.h"

void SWarningBox::Construct(const FArguments& InArgs)
{
	SBorder::Construct(SBorder::FArguments()
		.Padding(InArgs._Padding)
		.ForegroundColor(FAppStyle::Get().GetSlateColor("Colors.White"))
		.BorderImage(FXiaoStyle::Get().GetBrush("RoundedWarning"))
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.HAlign(HAlign_Left)
			.VAlign(VAlign_Center)
			.AutoWidth()
			.Padding(FMargin(0.0f, 0.0f, 16.0f, 0.0f))
			[
				SNew(SImage)
				.DesiredSizeOverride(InArgs._IconSize)
				.Image(FXiaoStyle::Get().GetBrush("Icons.WarningWithColor"))
				.ColorAndOpacity(FSlateColor::UseForeground())
			]
			+SHorizontalBox::Slot()
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(InArgs._Message)
				.ColorAndOpacity(FAppStyle::Get().GetSlateColor("Colors.White"))
				.AutoWrapText(InArgs._AutoWrapText)
			]

			+SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.HAlign(HAlign_Right)
			[
				InArgs._Content.Widget
			]
		]);
}

