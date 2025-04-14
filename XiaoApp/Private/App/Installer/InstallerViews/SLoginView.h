/**
  * @author cxx2020@outlook.com
  * @date 11:02 PM
 */
#pragma once

#include "SWizardView.h"

class SEditableTextBox;

class SLoginView final : public SWizardView
{
public:
	SLATE_BEGIN_ARGS(SLoginView){}
	SLATE_END_ARGS()
	
	void Construct(const FArguments& InArgs);
	
	virtual TWeakPtr<SWizardView> GetNext() override;
	virtual bool OnCanNext() override;

protected:
	void OnUsernameChanged(const FText& InText);
	void OnPasswordChanged(const FText& InText);
	void OnRePasswordChanged(const FText& InText);

private:
	TSharedPtr<class SErrorText> ErrorText = nullptr;
	TSharedPtr<SEditableTextBox> UsernameTextBox = nullptr;
	TSharedPtr<SEditableTextBox> PasswordTextBox = nullptr;
	TSharedPtr<SEditableTextBox> RePasswordTextBox = nullptr;

	TSharedPtr<class SInstallProgressView> ProgressView = nullptr;
	
	bool bIsValidUsername = true;
	bool bIsValidPassword = false;
	bool bIsSamePassword = false;
};
