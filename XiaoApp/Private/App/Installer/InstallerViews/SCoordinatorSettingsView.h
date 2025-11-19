/**
  * @author cxx2020@outlook.com
  * @date 11:02 PM
 */
#pragma once

#include "SWizardView.h"

template<typename NumericType> class SNumericEntryBox;
class SCheckBox;
class SErrorText;

class SCoordinatorSettingsView final : public SWizardView
{
public:
	SLATE_BEGIN_ARGS(SCoordinatorSettingsView)
	{}
	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs);
	
	virtual TWeakPtr<SWizardView> GetNext() override;
	virtual bool OnCanNext() override;

protected:
	bool CheckCoordiPort() const;
	void OnCheckButtonClicked();
	bool PortIsSafe() const;

private:
	TSharedPtr<SErrorText> ErrorText = nullptr;
	TSharedPtr<SCheckBox> AutomaticallyOpenCheckBox = nullptr;
	bool bPass = true;

	TSharedPtr<class SInstallationFolderView> InstallationFolderView = nullptr;

	bool bCoordiServerPort = false;
	bool bAgentCommunicatePort = false;
	bool bCacheServicePort = false;

	TArray<uint32> Ports;
};
