/**
  * @author cxx2020@outlook.com
  * @date 11:02 PM
 */
#pragma once

#include "SWizardView.h"

class SGridInstallView final : public SWizardView
{
public:
	SLATE_BEGIN_ARGS(SGridInstallView)
	{}
	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs);
	
	virtual TWeakPtr<SWizardView> GetNext() override;

private:
	// TSharedPtr<class SLicenseActivation> LicenseActivationView = nullptr;
	TSharedPtr<class SCoordinatorSettingsView> CoordinatorSettingsView = nullptr;
};
