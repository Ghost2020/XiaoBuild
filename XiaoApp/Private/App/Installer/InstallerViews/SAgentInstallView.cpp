/**
  * @author cxx2020@outlook.com
  * @date 11:02 PM
 */
#include "SAgentInstallView.h"
#include "SlateOptMacros.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Images/SImage.h"

#include "XiaoStyle.h"
#include "ShareWidget.h"

#include "SConnect2AgentView.h"

#define LOCTEXT_NAMESPACE "SAgentInstallView"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SAgentInstallView::Construct(const FArguments& InArgs)
{
	ChildSlot
	[
		SNew(SVerticalBox)
		+SVerticalBox::Slot().AutoHeight()
		[
			SNew(SVerticalBox)
			+SVerticalBox::Slot().FIR_PADDING
			[
				SNew(STextBlock)
				.Text(LOCTEXT("InstallAgent_Text", "安装代理"))
			]
			+SVerticalBox::Slot().SEC_PADDING
			[
				SNew(STextBlock)
				.Text(LOCTEXT("AgentDesc_Text", "仅仅安装代理所需的相关程序，前提是已经有相应的调度器安装"))
			]
			+ SVerticalBox::Slot().SEC_PADDING.HAlign(HAlign_Center).VAlign(VAlign_Center)
			[
				SNew(SImage).Image(FXiaoStyle::Get().GetBrush("Installer/Agent"))
			]
		]
	];
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION

TWeakPtr<SWizardView> SAgentInstallView::GetNext()
{
	if(!Connect2AgentView.IsValid())
	{
		Connect2AgentView = SNew(SConnect2AgentView);
	}
	return Connect2AgentView;
}


#undef LOCTEXT_NAMESPACE