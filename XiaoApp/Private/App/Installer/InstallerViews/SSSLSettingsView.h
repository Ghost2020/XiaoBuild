/**
  * @author cxx2020@outlook.com
  * @date 11:02 PM
 */
#pragma once

#include "SWizardView.h"

class SCheckBox;
class SEditableTextBox;

class SSSLSettingsView final : public SWizardView
{
public:
	SLATE_BEGIN_ARGS(SSSLSettingsView)
	{}
	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs);
	
	virtual TWeakPtr<SWizardView> GetNext() override;
	virtual bool OnCanNext() override;

private:
	TSharedPtr<SCheckBox> HaveTrustedBox = nullptr;
	TSharedPtr<SEditableTextBox> YourCertificateTextBox = nullptr;
	TSharedPtr<SEditableTextBox> PrivateKeyTextBox = nullptr;

	TSharedPtr<SCheckBox> NotUseTrustedBox = nullptr;

	TSharedPtr<class SLoginView> LoginView = nullptr;
	TSharedPtr<class SInstallProgressView> ProgressView = nullptr;
};
