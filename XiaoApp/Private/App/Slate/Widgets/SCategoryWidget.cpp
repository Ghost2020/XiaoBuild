/**
  * @author cxx2020@outlook.com
  * @date 10:49 AM
 */


#include "SCategoryWidget.h"

#include "XiaoStyle.h"
#include "SlateOptMacros.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Text/STextBlock.h"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SCategoryWidget::Construct(const FArguments& InArgs)
{
	ChildSlot
	[
		SNew(SButton).Content()
		[
			//SNew(SBorder)
			// .Padding(50.0f, 5.0f, 10.0f, 5.0f)
			/*.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
			.BorderBackgroundColor_Lambda([this] ()
			{
				return bSelect ? FLinearColor::Green : FLinearColor::Transparent;
			})
			[*/
				SNew(STextBlock).Text(InArgs._Text)
			/*]*/
		]
		.OnPressed(InArgs._OnPressed)
	];
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SCategoryWidget::SetSelect(const bool bInSelect)
{
	bSelect = bInSelect;
}