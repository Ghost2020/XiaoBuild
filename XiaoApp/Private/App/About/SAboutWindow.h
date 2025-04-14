/**
  * @author cxx2020@outlook.com
  * @date 11:02 PM
 */
#pragma once

#include "CoreMinimal.h"
#include "Widgets/SWindow.h"
#include "Widgets/DeclarativeSyntaxSupport.h"

class SAboutWindow final : public SWindow
{
	SLATE_BEGIN_ARGS(SAboutWindow){}
	SLATE_END_ARGS()

	void Construct(const FArguments& Args);

protected:
	FText GetVersionText() const;

	void OnExit(const TSharedRef<SWindow>& InWindow) const;

private:
	TSharedPtr<FSlateBrush> QRBrush = nullptr;
};