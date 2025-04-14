/**
  * @author cxx2020@outlook.com
  * @date 11:02 PM
 */

#pragma once

#include "XiaoShare.h"
#include "XiaoStyle.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Images/SImage.h"


#define TOP_WIDGET \
	+SVerticalBox::Slot().VAlign(VAlign_Top).AutoHeight() \
	[ \
		SNew(SHorizontalBox) \
		+SHorizontalBox::Slot() \
		[ \
			SNew(SOverlay) \
			+SOverlay::Slot().HAlign(HAlign_Left).VAlign(VAlign_Center) \
			[ \
				SNew(SImage) \
				.Image(FXiaoStyle::Get().GetBrush("LongLogo")) \
			] \
			+SOverlay::Slot().HAlign(HAlign_Right).VAlign(VAlign_Center) \
			[ \
				SNew(STextBlock) \
				.Text(FText::FromString(GetBuildVersion())) \
			] \
		] \
	]


inline const FSlateBrush* GetBrush(const bool InStatus)
{
	return InStatus ? FXiaoStyle::Get().GetBrush(TEXT("Port.OK")) : FXiaoStyle::Get().GetBrush(TEXT("Port.Query"));
}

#define LOCTEXT_NAMESPACE "XiaoShare"

namespace Xiao
{
	const FText SSamePortError = LOCTEXT("SamePortError_Text", "当前端口与其他端口冲突");
}

#undef LOCTEXT_NAMESPACE