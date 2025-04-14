/**
 * @author cxx2020@outlook.com
  * @date 11:02 PM
 */
#pragma once

#include "SWizardView.h"

class SLicenseAgreement final : public SWizardView
{
public:
	SLATE_BEGIN_ARGS(SLicenseAgreement)
	{}
	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs);

	virtual TWeakPtr<SWizardView> GetNext() override;
	virtual bool OnCanNext() override;
	virtual bool OnCanBack() override;

private:
	TSharedPtr<class SScrollBar> VerticalScrollbar = nullptr;
	TSharedPtr<class SCheckBox> ConfirmCheckBox = nullptr;

	TSharedPtr<class SSetupOptions> SetupView = nullptr;
};
