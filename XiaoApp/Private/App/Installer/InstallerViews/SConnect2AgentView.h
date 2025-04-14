/**
  * @author cxx2020@outlook.com
  * @date 11:02 PM
 */
#pragma once

#include "SWizardView.h"

template<typename NumericType>
class SNumericEntryBox;

class SConnect2AgentView final : public SWizardView
{
public:
	SLATE_BEGIN_ARGS(SConnect2AgentView)
	{}
	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs);

	virtual TWeakPtr<SWizardView> GetNext() override;
	virtual bool OnCanNext() override;
	
	void AsyncNetworkTest();

private:
	TSharedPtr<class SCheckBox> ConnectCheckBox = nullptr;
	TSharedPtr<class SEditableTextBox> NetworkIpText = nullptr;
	TSharedPtr<SNumericEntryBox<uint16>> PortBox = nullptr;

	TSharedPtr<class SImage> NetworkStatus = nullptr;

	TSharedPtr<class SAgentSettingsView> AgentSettingsView = nullptr;
	TSharedPtr<class SInstallProgressView> InstallProgressView = nullptr;

	bool bCanClickTestButton = true;
	bool bConnect = false;
};
