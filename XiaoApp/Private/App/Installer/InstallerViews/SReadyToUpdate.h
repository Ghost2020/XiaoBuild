/**
  * @author cxx2020@outlook.com
  * @date 11:02 PM
 */

#pragma once

#include "SWizardView.h"

class SReadyToUpdate final : public SWizardView
{
public:
	SLATE_BEGIN_ARGS(SReadyToUpdate)
	{}
	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs);

	virtual TWeakPtr<SWizardView> GetNext() override;
	virtual bool OnCanNext() override;
	virtual bool OnCanBack() override;
};
