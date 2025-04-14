/**
  * @author cxx2020@outlook.com
  * @date 11:02 PM
 */

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

class SNavButton final : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SNavButton){}
		SLATE_ARGUMENT(FName, NormalImage)
		SLATE_ARGUMENT(FName, SeletedImage)
		SLATE_ATTRIBUTE(FText, Text)	
		SLATE_EVENT(FSimpleDelegate, OnPressed)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	void SetSelect(const bool bInSelect);

private:
	const FSlateBrush* NormalImage = nullptr;
	const FSlateBrush* SelectedImage = nullptr;
	TSharedPtr<class SBorder> Border = nullptr;
	TSharedPtr<class SImage> Image = nullptr;
	TSharedPtr<class STextBlock> Text = nullptr;
};
