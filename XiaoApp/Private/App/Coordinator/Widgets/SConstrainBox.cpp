/**
  * @author cxx2020@outlook.com
  * @date 11:02 PM
 */
#include "SConstrainBox.h"
#include "SlateOptMacros.h"

void SConstrainBox::Construct(const FArguments& InArgs)
{
	MinWidth = InArgs._MinWidth;
	MaxWidth = InArgs._MaxWidth;
	MinHeight = InArgs._MinHeight;
	MaxHeight = InArgs._MaxHeight;

	ChildSlot
	[
		InArgs._Content.Widget
	];
}

FVector2D SConstrainBox::ComputeDesiredSize(const float LayoutScaleMultiplier) const
{
	const float MinWidthVal = MinWidth.Get().Get(0.0f);
	const float MaxWidthVal = MaxWidth.Get().Get(0.0f);
	const float MinHeightVal = MinHeight.Get().Get(0.0f);
	const float MaxHeightVal = MaxHeight.Get().Get(0.0f);

	if (MinWidthVal == 0.0f && MaxWidthVal == 0.0f)
	{
		return SCompoundWidget::ComputeDesiredSize(LayoutScaleMultiplier);
	}

	const FVector2D ChildSize = ChildSlot.GetWidget()->GetDesiredSize();
	float XVal = FMath::Max(MinWidthVal, ChildSize.X);
	if (MaxWidthVal > MinWidthVal)
	{
		XVal = FMath::Min(MaxWidthVal, XVal);
	}

	float YVal = FMath::Max(MinHeightVal, ChildSize.Y);
	if (MaxHeightVal > MinHeightVal)
	{
		YVal = FMath::Min(MaxHeightVal, YVal);
	}
	return FVector2D(XVal, YVal);
}
