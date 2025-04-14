/**
  * @author cxx2020@outlook.com
  * @date 10:49 AM
 */

#pragma once

#include "CoreMinimal.h"
#include "Widgets/Layout/SBorder.h"

class SWarningBox : public SBorder
{
public:
	SLATE_BEGIN_ARGS(SWarningBox)
		: _Padding(16.0f)
		, _IconSize(24,24)
		, _AutoWrapText(true)
		, _Content()
	{}
		SLATE_ATTRIBUTE(FText, Message)
		SLATE_ARGUMENT(FMargin, Padding)
		SLATE_ARGUMENT(FVector2D, IconSize)
		SLATE_ARGUMENT(bool, AutoWrapText)

		SLATE_DEFAULT_SLOT( FArguments, Content )

	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
};