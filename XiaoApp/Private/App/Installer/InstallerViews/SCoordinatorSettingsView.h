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

private:
	TSharedPtr<SErrorText> ErrorText = nullptr;
	TSharedPtr<SNumericEntryBox<uint16>> CoordiServerPort = nullptr;
	TSharedPtr<SNumericEntryBox<uint16>> XiaobuildManagerPort = nullptr;
	TSharedPtr<SNumericEntryBox<uint16>> MessageTransportPort = nullptr;
	TSharedPtr<SNumericEntryBox<uint16>> AgentCommunicationPort = nullptr;
	TSharedPtr<SNumericEntryBox<uint16>> LicenseServicePort = nullptr;
	TSharedPtr<SCheckBox> AutomaticallyOpenCheckBox = nullptr;
	bool bPass = true;

	TSharedPtr<class SInstallationFolderView> InstallationFolderView = nullptr;

	bool bCoordiServerPort = false;
	bool bUIManagerPort = false;
	bool bMessageTransPort = false;
	bool bAgentCommunicatePort = false;
	bool bLicenseServicePort = false;
};
