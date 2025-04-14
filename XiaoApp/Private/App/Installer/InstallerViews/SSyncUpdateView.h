/**
  * @author cxx2020@outlook.com
  * @date 11:02 PM
 */
#pragma once

#include "SWizardView.h"

class SSyncUpdateView final : public SWizardView
{
public:
	SLATE_BEGIN_ARGS(SSyncUpdateView){}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
	
	virtual bool OnCanBack() override;
	virtual bool OnCanNext() override;
	virtual FText GetNextTitle() override;
	virtual bool IsFinal() override;

protected:
	bool CanSyncUpdate();

private:
	TSharedPtr<class SFinishView> FinishView = nullptr;
	TSharedPtr<class SErrorText> ErrorText = nullptr;
	FString ErrorStr;
	bool bCanUpdate = false;
};
