/**
  * @author cxx2020@outlook.com
  * @date 11:02 PM
 */

#pragma once

#include "SWizardView.h"

class SCheckBox;

class SSetupOptions final : public SWizardView
{
public:
	SLATE_BEGIN_ARGS(SSetupOptions)
	{}
	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs);
	
	virtual TWeakPtr<SWizardView> GetNext() override;
	virtual bool OnCanNext() override;
	virtual bool OnCanBack() override;

private:
	TSharedPtr<class SVerticalBox> SelectionContainer = nullptr;
	
	TSharedPtr<SCheckBox> InstallCheck = nullptr;
	TSharedPtr<SCheckBox> CreateSilentCheck = nullptr;
	TSharedPtr<SCheckBox> UninstallCheck = nullptr;

	TSharedPtr<class SInstallView> InstallView = nullptr;
	TSharedPtr<class SReadyToUpdate> UpdateView = nullptr;
	TSharedPtr<class SUninstallView> UninstallView = nullptr;
	
	bool bCanNext = true;
};
