/**
  * @author cxx2020@outlook.com
  * @date 11:02 PM
 */

#include "SReadyToUpdate.h"
#include "SlateOptMacros.h"
#include "Widgets/SBoxPanel.h"
#include "ShareWidget.h"
#include "XiaoStyle.h"

#define LOCTEXT_NAMESPACE "SReadyToUpdate"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SReadyToUpdate::Construct(const FArguments& InArgs)
{
	ChildSlot
	[
		SNew(SVerticalBox)
		TOP_WIDGET

		+SVerticalBox::Slot().AutoHeight().FIR_PADDING
		[
			SNew(STextBlock)
			.Text(LOCTEXT("ReadyToUpdate", "已经准备好可以从你的电脑更新Xiaobuild相关的组件"))
		]

		+SVerticalBox::Slot().AutoHeight()
		[
			SNew(SBorder)
		]
	];
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION

TWeakPtr<SWizardView> SReadyToUpdate::GetNext()
{
	return nullptr;
}

bool SReadyToUpdate::OnCanNext()
{
	return false;
}

bool SReadyToUpdate::OnCanBack()
{
	return true;
}

#undef LOCTEXT_NAMESPACE