/**
  * @author cxx2020@outlook.com
  * @date 11:02 PM
 */
#include "SlicenseActivation.h"
#include "SlateOptMacros.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SCheckBox.h"

#include "ShareWidget.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Layout/SBorder.h"
#include "XiaoStyle.h"


#define LOCTEXT_NAMESPACE "SLicenseActivation"


BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SLicenseActivation::Construct(const FArguments& InArgs)
{
	ChildSlot
	[
		SNew(SVerticalBox)
		TOP_WIDGET

		+SVerticalBox::Slot().AutoHeight()
		[
			SNew(STextBlock).Text(LOCTEXT("Activate_Text", "激活你的许可"))
		]

		+SVerticalBox::Slot().AutoHeight()
		[
			SNew(SBorder)
		]

		+SVerticalBox::Slot().SEC_PADDING
		[
			SNew(SVerticalBox)
			+SVerticalBox::Slot()
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().AutoWidth().HAlign(HAlign_Left)
				[
					SAssignNew(HaveLicenseCheckBox, SCheckBox)
					.IsChecked(true)
					.OnCheckStateChanged_Lambda([this](const ECheckBoxState InState)
					{
						if(InState == ECheckBoxState::Checked)
						{
							DontLicenseCheckBox->SetIsChecked(false);
						}
						else
						{
							HaveLicenseCheckBox->SetIsChecked(true);
						}
						GInstallSettings.bHasLicense = DontLicenseCheckBox->GetCheckedState() == ECheckBoxState::Checked ? true : false;
					})
				]
				+ SHorizontalBox::Slot().AutoWidth().HAlign(HAlign_Left)
				[
					SNew(STextBlock).Text(LOCTEXT("IHaveLicense_Text", "我有许可密钥"))
				]
			]
			+SVerticalBox::Slot().L_PADDING(20.0f)
			[
				SNew(STextBlock).Text(LOCTEXT("PleaseEnter_Text", "请输入许可密钥,然后点击激活"))
			]
			+SVerticalBox::Slot().L_PADDING(20.0f)
			[
				SNew(SHorizontalBox)
				+SHorizontalBox::Slot()
				[
					SAssignNew(LicenseKeyTextBox, SEditableTextBox)
					.IsEnabled_Raw(this, &SLicenseActivation::GetIsEnable)
				]
				+SHorizontalBox::Slot().AutoWidth()
				[
					SNew(SButton)
					.IsEnabled_Lambda([this]()
					{
						return GetIsEnable() && !LicenseKeyTextBox->GetText().IsEmpty();
					})
					.Text(LOCTEXT("ActivateButton_Text", "激活"))
					.OnPressed_Lambda([this]()
					{
						// TODO
						this->bActivate = true;
						if(this->bActivate)
						{
							GInstallSettings.LicenseKey = LicenseKeyTextBox->GetText().ToString();
						}
					})
				]
			]
		]

		+SVerticalBox::Slot().SEC_PADDING
		[
			SNew(SVerticalBox)
			+SVerticalBox::Slot()
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().AutoWidth().HAlign(HAlign_Left)
				[
					SAssignNew(DontLicenseCheckBox, SCheckBox)
					.IsChecked(false)
					.OnCheckStateChanged_Lambda([this](const ECheckBoxState InState)
					{
						if(InState == ECheckBoxState::Checked)
						{
							HaveLicenseCheckBox->SetIsChecked(false);
						}
						else
						{
							DontLicenseCheckBox->SetIsChecked(true);
						}
						GInstallSettings.bHasLicense = DontLicenseCheckBox->GetCheckedState() == ECheckBoxState::Checked ? true : false;
					})
				]
				+ SHorizontalBox::Slot().AutoWidth().HAlign(HAlign_Left)
				[
					SNew(STextBlock).Text(LOCTEXT("IDontHaveLicense_Text", "我没有许可密钥"))
				]
			]
			+SVerticalBox::Slot().L_PADDING(20.0f)
			[
				SNew(STextBlock).Text(LOCTEXT("ContinueWithout_Text", "在没有许可密钥的情况下继续"))
			]
			+SVerticalBox::Slot().L_PADDING(20.0f)
			[
				SNew(STextBlock).Text(LOCTEXT("YouWillNot_Text", "在没有许可密钥的情况下你将无法使用Xiaobuild"))
			]
		]
	];
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION

TWeakPtr<SWizardView> SLicenseActivation::GetNext()
{
	if(!CoordinatorSettingsView.IsValid())
	{
		CoordinatorSettingsView = SNew(SCoordinatorSettingsView);
	}
	return CoordinatorSettingsView;
}

bool SLicenseActivation::OnCanNext()
{
	if(HaveLicenseCheckBox->IsChecked())
	{
		if(!LicenseKeyTextBox->GetText().IsEmpty())
		{
			return bActivate;
		}
	}
	else
	{
		// TODO 需要实际运行查看
	}
	return false;
}

bool SLicenseActivation::GetIsEnable() const
{
	return HaveLicenseCheckBox->IsChecked();
}

#undef LOCTEXT_NAMESPACE