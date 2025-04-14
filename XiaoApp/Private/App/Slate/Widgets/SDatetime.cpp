/**
  * @author cxx2020@outlook.com
  * @date 10:49 AM
 */

#include "SDatetime.h"
#include "SlateOptMacros.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Input/SNumericEntryBox.h"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SDatetime::Construct(const FArguments& InArgs)
{
	OnHourChanged = InArgs._OnHourChanged;
	OnMinuteChanged = InArgs._OnMinuteChanged;
	OnSecondChanged = InArgs._OnSecondChanged;
	
	ChildSlot
	[
		SNew(SHorizontalBox)
		+SHorizontalBox::Slot()
		[
			SAssignNew(HourBox, SNumericEntryBox<uint8_t>)
			.MinDesiredValueWidth(20.0f)
			.MinValue(00).MaxValue(24)
			.Value(InArgs._HourValue)
			.OnValueChanged_Lambda([this](const uint8_t InHour)
			{
				(void)OnHourChanged.ExecuteIfBound(InHour);
			})
		]
		+SHorizontalBox::Slot().AutoWidth()
		[
			SNew(STextBlock).Text(FText::FromString(TEXT(" : ")))
		]
		+SHorizontalBox::Slot()
		[
			SAssignNew(MinuteBox, SNumericEntryBox<uint8_t>)
			.MinDesiredValueWidth(20.0f)
			.MinValue(00).MaxValue(60)
			.Value(InArgs._MinuteValue)
			.OnValueChanged_Lambda([this](const uint8_t InMinute)
			{
				(void)OnMinuteChanged.ExecuteIfBound(InMinute);
			})
		]
		+SHorizontalBox::Slot().AutoWidth()
		[
			SNew(STextBlock).Text(FText::FromString(TEXT(" : ")))
		]
		+SHorizontalBox::Slot()
		[
			SAssignNew(SecondBox, SNumericEntryBox<uint8_t>)
			.MinDesiredValueWidth(20.0f)
			.MinValue(00).MaxValue(60)
			.Value(InArgs._SecondsValue)
			.OnValueChanged_Lambda([this](const uint8_t Second)
			{
				(void)OnSecondChanged.ExecuteIfBound(Second);
			})
		]
	];
	
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION
