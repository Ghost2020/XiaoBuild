/**
  * @author cxx2020@outlook.com
  * @date 11:02 PM
 */
#include "STwowaySlider.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SSlider.h"

void STwowaySlider::Construct(const FArguments& InArgs)
{
	ShowUnit = InArgs._ShowUnit;
	OnValueChanged = InArgs._OnValueChanged;
	ChildSlot
	[
		SNew(SVerticalBox)
		+SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
		[
			SAssignNew(CPuText, STextBlock).Text(FText::FromString(InArgs._InitialString))
		]
		+ SVerticalBox::Slot().AutoHeight()
		[
			SAssignNew(SliderTop, SSlider)
			.MinValue(InArgs._MinValue).MaxValue(InArgs._MaxValue).Value(InArgs._InitialMinValue)
			.IsFocusable(false)
			.OnValueChanged_Lambda([this] (float InValue)
			{
				PrivateOnValueChanged(true);
			})
		]

		+ SVerticalBox::Slot().AutoHeight()
		[
			SAssignNew(SliderBot, SSlider)
			.MinValue(InArgs._MinValue).MaxValue(InArgs._MaxValue).Value(InArgs._InitialMaxValue)
			.IsFocusable(false)
			.OnValueChanged_Lambda([&] (float InValue)
			{
				PrivateOnValueChanged(false);
			})
		]
	];
}

void STwowaySlider::PrivateOnValueChanged(const bool bWhichSlider) const
{
	if(bWhichSlider)
	{
		if (SliderTop->GetValue() >= SliderBot->GetValue())
		{
			SliderBot->SetValue(SliderTop->GetValue() + 1);
		}
	}
	else
	{
		if (SliderBot->GetValue() <= SliderTop->GetValue())
		{
			SliderTop->SetValue(SliderBot->GetValue() - 1);
		}	
	}
	CPuText->SetText(FText::FromString(FString::Printf(TEXT("显示%d%s-%d%s"), static_cast<int>(SliderTop->GetValue()), *ShowUnit, static_cast<int>(SliderBot->GetValue()), *ShowUnit)));
	this->OnValueChanged.Execute(SliderTop->GetValue(), SliderBot->GetValue());
}