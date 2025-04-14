/**
  * @author cxx2020@outlook.com
  * @date 5:57 PM
 */
#pragma once

#include "CoreMinimal.h"
#include "Widgets/SWindow.h"

class XIAOAPP_API SProgressWindow final : public SWindow
{
public:
	SLATE_BEGIN_ARGS(SProgressWindow) {}
		SLATE_ARGUMENT(float, AllAmount)
		SLATE_ARGUMENT(FText, TiTile)
	SLATE_END_ARGS()
	
	void Construct(const FArguments& InArgs);

	void EnterProgressFrame(const float InExpectedWorkThisFrame, const FText& InText);

private:
	float AllAmount = 0.0f;
	float Amount = 0.0f;
	TSharedPtr<class STextBlock> Text = nullptr;
	TSharedPtr<class SProgressBar> ProgressBar = nullptr;
};
