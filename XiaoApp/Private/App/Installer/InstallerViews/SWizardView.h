/**
  * @author cxx2020@outlook.com
  * @date 11:02 PM
 */
#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "XiaoInstall.h"

class SWizardView : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SWizardView)
	{}
	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs);

	virtual void OnResetState() {};
	virtual TWeakPtr<SWizardView> GetNext() { return nullptr;  };
	virtual bool OnCanNext() { return false; }
	TWeakPtr<SWizardView> GetBack() const { return BackView; }
	virtual FText GetNextTitle();
	virtual bool OnCanBack() { return true; }
	virtual void OnExit();
	virtual bool OnCanExit() { return true; }
	virtual bool IsFinal() { return false; }
	virtual void OnFinish() { }

	TWeakPtr<SWizardView> BackView = nullptr;
};
