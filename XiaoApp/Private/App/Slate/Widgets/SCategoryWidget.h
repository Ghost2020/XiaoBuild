/**
  * @author cxx2020@outlook.com
  * @date 10:49 AM
 */

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

class SCategoryWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SCategoryWidget){}
		SLATE_ATTRIBUTE(FText, Text)
		SLATE_EVENT(FSimpleDelegate, OnPressed)
	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs);

	void SetSelect(const bool bInSelect);

private:
	bool bSelect = true;
};
