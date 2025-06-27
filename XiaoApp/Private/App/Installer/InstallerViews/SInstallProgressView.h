/**
  * @author cxx2020@outlook.com
  * @date 11:02 PM
 */
#pragma once

#include "SWizardView.h"

class SInstallProgressView final : public SWizardView
{
public:
	SLATE_BEGIN_ARGS(SInstallProgressView){}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;
	
	virtual bool OnCanBack() override;
	virtual bool OnCanNext() override;
	virtual FText GetNextTitle() override;
	virtual bool IsFinal() override;
	virtual bool OnCanExit() override;
	virtual void OnFinish() override;

protected:
	bool OnInstall() const;
	bool OnUpdate();
	bool OnUninstall();
	bool CheckEnv();

private:
	float UpdateTimer = 0.0f;

	TSharedPtr<class SFinishView> FinishView = nullptr;
	TSharedPtr<class SErrorText> ErrorText = nullptr;
	TSharedPtr<class SProgressBar> ProgressBar = nullptr;
	TSharedPtr<class SEditableTextBox> MessageTextBox = nullptr;
	FString ErrorStr;

	bool bValidEnv = false;
	bool bFinish = false;
};
