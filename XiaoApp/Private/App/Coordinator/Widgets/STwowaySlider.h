/**
  * @author cxx2020@outlook.com
  * @date 11:02 PM
 */
#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

class SSlider;

DECLARE_DELEGATE_TwoParams( FOnTwowayValueChanged, float, float);

class STwowaySlider final : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(STwowaySlider){}
		SLATE_ARGUMENT(FString, InitialString)
		SLATE_ARGUMENT(FString, ShowUnit)
		SLATE_ARGUMENT(float, MinValue)
		SLATE_ARGUMENT(float, MaxValue)
		SLATE_ARGUMENT(float, InitialMinValue)
		SLATE_ARGUMENT(float, InitialMaxValue)
		SLATE_EVENT(FOnTwowayValueChanged, OnValueChanged )
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

protected:
	void PrivateOnValueChanged(const bool bWhichSlider) const;

private:
	FString ShowUnit;
	TSharedPtr<class STextBlock> CPuText = nullptr;
	TSharedPtr<SSlider> SliderTop = nullptr;
	TSharedPtr<SSlider> SliderBot = nullptr;
	FOnTwowayValueChanged OnValueChanged;
};
