/**
  * @author cxx2020@outlook.com
  * @date 11:02 PM
 */

#include "SUninstallView.h"

#include "SlateOptMacros.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SOverlay.h"
#include "SInstallProgressView.h"
#include "ShareWidget.h"
#include "XiaoStyle.h"

#define LOCTEXT_NAMESPACE "SUninstallView"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SUninstallView::Construct(const FArguments& InArgs)
{
	ChildSlot
	[
		SNew(SVerticalBox)
		TOP_WIDGET

		+SVerticalBox::Slot().AutoHeight().FIR_PADDING
		[
			SNew(STextBlock)
			.Text(LOCTEXT("ReadyToRemove", "已经准备好可以从你的电脑移除XiaoBuild相关的组件"))
		]

		+SVerticalBox::Slot().AutoHeight()
		[
			SNew(SBorder)
		]
	];
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION

TWeakPtr<SWizardView> SUninstallView::GetNext()
{
	if(!ProgressView.IsValid())
	{
		ProgressView = SNew(SInstallProgressView);
	}
	return ProgressView;
}

bool SUninstallView::OnCanNext()
{
	return true;
}

bool SUninstallView::OnCanBack()
{
	return true;
}

bool SUninstallView::IsInstall()
{
	return true;
}

#undef LOCTEXT_NAMESPACE
