/**
  * @author cxx2020@outlook.com
  * @date 10:49 AM
 */

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

DECLARE_DELEGATE_OneParam(FOnValueChanged, uint8_t)

template <typename OptionType>
class SNumericEntryBox;

class SDatetime : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SDatetime){}
		SLATE_EVENT(FOnValueChanged, OnHourChanged)
		SLATE_ATTRIBUTE(TOptional<uint8_t>, HourValue)
		SLATE_EVENT(FOnValueChanged, OnMinuteChanged)
		SLATE_ATTRIBUTE(TOptional<uint8_t>, MinuteValue)
		SLATE_EVENT(FOnValueChanged, OnSecondChanged)
		SLATE_ATTRIBUTE(TOptional<uint8_t>, SecondsValue)
	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs);

private:
	TSharedPtr<SNumericEntryBox<uint8_t>> HourBox = nullptr;
	TSharedPtr<SNumericEntryBox<uint8_t>> MinuteBox = nullptr;
	TSharedPtr<SNumericEntryBox<uint8_t>> SecondBox = nullptr;

	FOnValueChanged OnHourChanged;
	FOnValueChanged OnMinuteChanged;
	FOnValueChanged OnSecondChanged;
};
