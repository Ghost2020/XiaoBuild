/**
  * @author cxx2020@outlook.com
  * @date 11:02 PM
 */
#include "SBaseView.h"
#include "SlateOptMacros.h"

TMap<FString, SBaseView::FUserIp> SBaseView::GID2User;

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SBaseView::Construct(const FArguments& InArgs)
{
	/*
	ChildSlot
	[
		// Populate the widget
	];
	*/
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION
