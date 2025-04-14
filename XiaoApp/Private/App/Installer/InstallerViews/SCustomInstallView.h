/**
  * @author cxx2020@outlook.com
  * @date 11:02 PM
 */
#pragma once

#include "SWizardView.h"

class SCheckBox;

class SCustomInstallView final : public SWizardView
{
public:
	SLATE_BEGIN_ARGS(SCustomInstallView)
	{}
	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs);
	
	virtual TWeakPtr<SWizardView> GetNext() override;

private:
	TSharedPtr<SCheckBox> AgentCheckBox = nullptr;
	TSharedPtr<SCheckBox> CoordinatorCheckBox = nullptr;
	TSharedPtr<SCheckBox> BuildMonitorCheckBox = nullptr;
	TSharedPtr<SCheckBox> BackUpCoordinatorCheckBox = nullptr;

	TSharedPtr<class SConnect2AgentView> Connect2AgentView = nullptr;
	TSharedPtr<class SCoordinatorSettingsView> CoordinatorSettingsView = nullptr;
	TSharedPtr<class SInstallProgressView> ProgressView = nullptr;
};
