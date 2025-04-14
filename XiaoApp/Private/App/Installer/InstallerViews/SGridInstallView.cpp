/**
  * @author cxx2020@outlook.com
  * @date 11:02 PM
 */

#include "SGridInstallView.h"
#include "SlateOptMacros.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Images/SImage.h"
#include "XiaoStyle.h"

// #include "SlicenseActivation.h"
#include "SCoordinatorSettingsView.h"

#define LOCTEXT_NAMESPACE "SGridInstallView"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SGridInstallView::Construct(const FArguments& InArgs)
{
	ChildSlot
	[
		SNew(SVerticalBox)
		+SVerticalBox::Slot().AutoHeight().VAlign(VAlign_Top)
		[
			SNew(SVerticalBox)
			+SVerticalBox::Slot().FIR_PADDING
			[
				SNew(STextBlock)
				.Text(LOCTEXT("InstallAgent_Text", "安装代理和协调器"))
			]
			+SVerticalBox::Slot().SEC_PADDING
			[
				SNew(STextBlock)
				.Text(LOCTEXT("XiaobuildDesc_Text", "Xiaobuild并行计算网络能够让远程的代理机器分发任务，从而达到分配任务的目的"))
			]
			+ SVerticalBox::Slot().SEC_PADDING.VAlign(VAlign_Center)
			[
				SNew(SImage).Image(FXiaoStyle::Get().GetBrush("Installer/Grid"))
			]
		]
	];
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION

TWeakPtr<SWizardView> SGridInstallView::GetNext()
{
	/*if(!LicenseActivationView.IsValid())
	{
		LicenseActivationView = SNew(SLicenseActivation);
	}
	return LicenseActivationView;*/
	if(!CoordinatorSettingsView.IsValid())
	{
		CoordinatorSettingsView = SNew(SCoordinatorSettingsView);
	}
	return CoordinatorSettingsView;
}

#undef LOCTEXT_NAMESPACE