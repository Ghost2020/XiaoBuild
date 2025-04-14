/**
  * @author cxx2020@outlook.com
  * @date 11:02 PM
 */
#pragma once

#include "SWizardView.h"

template<typename NumericType> class SNumericEntryBox;
class SCheckBox;
class SErrorText;

class SAgentSettingsView final : public SWizardView
{
public:
	SLATE_BEGIN_ARGS(SAgentSettingsView)
	{}
	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs);
	virtual TWeakPtr<SWizardView> GetNext() override;
	virtual bool OnCanNext() override;

protected:
	void OnCheckButtonClicked();

private:
	TSharedPtr<SErrorText> ErrorText = nullptr;
	TSharedPtr<SNumericEntryBox<uint16>> AgentServicePort = nullptr;
	TSharedPtr<SNumericEntryBox<uint16>> HelperPort = nullptr;
	TSharedPtr<SCheckBox> AutomaticallyOpenCheckBox = nullptr;
	TSharedPtr<SNumericEntryBox<uint32>> MaxFileSize = nullptr;
	TSharedPtr<SCheckBox> InstallVSAddInCheckBox = nullptr;

	TSharedPtr<class SInstallationFolderView> InstallationFolderView = nullptr;

	bool bAgentServerPort = false;
	bool bSchedulerServerPort = false;
	bool bHelperPort = false;
};
