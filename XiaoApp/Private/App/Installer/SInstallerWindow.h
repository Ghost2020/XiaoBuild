/**
  * @author cxx2020@outlook.com
  * @date 11:02 PM
 */
#pragma once

#include "CoreMinimal.h"
#include "Widgets/SWindow.h"
#include "Widgets/DeclarativeSyntaxSupport.h"

class SPrimaryButton;

class SInstallerWindow final : public SWindow
{
public:
	SLATE_BEGIN_ARGS(SInstallerWindow){}
	SLATE_END_ARGS()

	void Construct(const FArguments& Args);

	void SetLockedState(const bool InbLock) const;
protected:
	void ConstructView();

	static FReply OnHelp();

	void OnExit(const TSharedRef<SWindow>& InWindow) const;
	void OnExit() const;

private:
	TArray<TSharedPtr<FString>> LauguageOptionList;
	TSharedPtr<STextBlock> LauguageText = nullptr;

	TSharedPtr<class SBorder> MaskBorder = nullptr;
	TSharedPtr<class SWidgetSwitcher> Switcher = nullptr;
	TSharedPtr<class SWelcomeView> WelcomeView = nullptr;
	TSharedPtr<class SSyncUpdateView> UpdateView = nullptr;
	TWeakPtr<class SWizardView> CurrentView = nullptr;
	
	TSharedPtr<SPrimaryButton> HelpButton = nullptr;
	TSharedPtr<SPrimaryButton> BackButton = nullptr;
	TSharedPtr<SPrimaryButton> NextButton = nullptr;
	TSharedPtr<SPrimaryButton> ExitButton = nullptr;
	TSharedPtr<SPrimaryButton> FinishButton = nullptr;

	bool bSyncUpdate = false;
};