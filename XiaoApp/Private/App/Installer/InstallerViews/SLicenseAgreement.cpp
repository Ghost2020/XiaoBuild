/**
  * @author cxx2020@outlook.com
  * @date 11:02 PM
 */

#include "SLicenseAgreement.h"
#include "SlateOptMacros.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Input/SMultiLineEditableTextBox.h"

#include "ShareWidget.h"
#include "SSetupOptions.h"
#include "Widgets/Layout/SGridPanel.h"
#include "XiaoShare.h"
#include "XiaoAgreement.h"
#include "XiaoStyle.h"

#define LOCTEXT_NAMESPACE "LicenseAgreement"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SLicenseAgreement::Construct(const FArguments& InArgs)
{
	VerticalScrollbar = SNew(SScrollBar)
	.AlwaysShowScrollbar(true)
	.AlwaysShowScrollbarTrack(true)
	.HideWhenNotInUse(false)
	.Orientation(Orient_Vertical);
	
	ChildSlot
	[
		SNew(SVerticalBox)
		TOP_WIDGET
		+SVerticalBox::Slot().AutoHeight()
		[
			SNew(SVerticalBox)
			+SVerticalBox::Slot().FIR_PADDING
			[
				SNew(STextBlock)
				.Text(LOCTEXT("LicenseAgreement_Text", "许可协议"))
			]
			+SVerticalBox::Slot().SEC_PADDING
			[
				SNew(STextBlock)
				.Text(LOCTEXT("ReadFollowing_Text", "阅读下述的协议然后勾选确定"))
			]
		]
		
		+SVerticalBox::Slot().AutoHeight()
		[
			SNew(SBorder)
		]

		+SVerticalBox::Slot().Padding(20.0f)
		[
			SNew(SGridPanel)
			.FillColumn(0, 1.0f)
			.FillRow(0, 1.0f)
			+SGridPanel::Slot(0, 0)
			[
				SNew(SMultiLineEditableText)
				.IsReadOnly(true).AutoWrapText(true)
				.VScrollBar(VerticalScrollbar)
				.Text(LicenseAgreement)
			]
			+SGridPanel::Slot(1, 0)
			[
				VerticalScrollbar.ToSharedRef()
			]
		]

		+SVerticalBox::Slot().AutoHeight().Padding(20.0f)
		[
			SNew(SHorizontalBox)
			+SHorizontalBox::Slot()
			[
				SNullWidget::NullWidget
			]
			+SHorizontalBox::Slot().HAlign(HAlign_Center).AutoWidth()
			[
				SAssignNew(ConfirmCheckBox, SCheckBox)
				.IsChecked(false)
				.Content()
				[
					SNew(STextBlock)
					.Text(LOCTEXT("AccpetConfime_Text", "我接受上述许可协议中出现的条款"))
				]
			]
			+ SHorizontalBox::Slot()
			[
				SNullWidget::NullWidget
			]
		]
	];
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION

TWeakPtr<SWizardView> SLicenseAgreement::GetNext()
{
	if(!SetupView.IsValid())
	{
		SetupView = SNew(SSetupOptions);
	}
	return SetupView;
}

bool SLicenseAgreement::OnCanNext()
{
	return ConfirmCheckBox->IsChecked();
}

bool SLicenseAgreement::OnCanBack()
{
	return true;
}

#undef LOCTEXT_NAMESPACE