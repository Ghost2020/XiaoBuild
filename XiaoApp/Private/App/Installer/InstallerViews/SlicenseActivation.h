/**
  * @author cxx2020@outlook.com
  * @date 11:02 PM
 */
#pragma once

#include "SCoordinatorSettingsView.h"
#include "SWizardView.h"

class SCheckBox;

class SLicenseActivation final : public SWizardView
{
public:
	SLATE_BEGIN_ARGS(SLicenseActivation)
	{}
	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs);
	
	virtual TWeakPtr<SWizardView> GetNext() override;
	virtual bool OnCanNext() override;

protected:
	bool GetIsEnable() const;

private:
	TSharedPtr<SCheckBox> HaveLicenseCheckBox = nullptr;
	TSharedPtr<class SEditableTextBox> LicenseKeyTextBox = nullptr;
	bool bActivate = false;
	TSharedPtr<SCheckBox> DontLicenseCheckBox = nullptr;
	TSharedPtr<SCoordinatorSettingsView> CoordinatorSettingsView = nullptr;
};
