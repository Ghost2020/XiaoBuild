/**
  * @author cxx2020@outlook.com
  * @date 11:02 PM
 */
#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

class SConstrainBox final : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SConstrainBox)
	{}
	SLATE_DEFAULT_SLOT(FArguments, Content)
		SLATE_ATTRIBUTE(TOptional<float>, MinWidth)
		SLATE_ATTRIBUTE(TOptional<float>, MaxWidth)
		SLATE_ATTRIBUTE(TOptional<float>, MinHeight)
		SLATE_ATTRIBUTE(TOptional<float>, MaxHeight)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
	virtual FVector2D ComputeDesiredSize(float LayoutScaleMultiplier) const override;

private:
	TAttribute< TOptional<float> > MinWidth;
	TAttribute< TOptional<float> > MaxWidth;
	TAttribute< TOptional<float> > MinHeight;
	TAttribute< TOptional<float> > MaxHeight;
};
