/**
  * @author cxx2020@outlook.com
  * @date 11:02 PM
 */
#pragma once

#include "CoreMinimal.h"
#include "Widgets/SWindow.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Framework/Docking/TabManager.h"

class FMenuBuilder;
class SExpandableArea;
class SNavButton;

class SCoordinatorWindow final : public SWindow
{
	SLATE_BEGIN_ARGS(SCoordinatorWindow){}
	SLATE_END_ARGS()
	
	SCoordinatorWindow();
	virtual	~SCoordinatorWindow() override;

	void Construct(const FArguments& Args);

	// ~SCompoundWidget Begin
	virtual void Tick(const FGeometry& InAllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;
	virtual FReply OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) override;
	// ~SCompoundWidget Finish

	void OnExit(const TSharedRef<SWindow>& InWindow);
	void SetLockedState(const bool InbLock) const;
	void SetErrorText(const FText& InText);
	
protected:
	void OnUpdate();
	void ResetNavState() const;
	void ConstructWidgets();
	void OnQueueNotification(int8 InStatus, const FText& InText);
	
private:
	bool bFullScreen = false;

	TSharedPtr<FTabManager> TabManager = nullptr;

	TSharedPtr<class SNotificationList> NotificationList = nullptr;

	TSharedPtr<class SBorder> MaskBorder = nullptr;

	TSharedPtr<class STextBlock> TitleText = nullptr;
	TSharedPtr<class SErrorText> NotificationText = nullptr;
	FText ErrorTExt;

	TSharedPtr<SNavButton> AgentButton = nullptr;
	TSharedPtr<SNavButton> UsersButton = nullptr;
	TSharedPtr<SNavButton> LogsButton = nullptr;
	TSharedPtr<SNavButton> SettingsButton = nullptr;
	TSharedPtr<SNavButton> HelpButton = nullptr;

	TSharedPtr<class SButton> NotificationButton = nullptr;
	
	TSharedPtr<class SAgentView> AgentView = nullptr;
	TSharedPtr<class SUsersView> UsersView = nullptr;
	TSharedPtr<class SLogsView> LogsView = nullptr;
	TSharedPtr<class SSettingsView> SettingsView = nullptr;

	TSharedPtr<SWidgetSwitcher> ViewSwitcher = nullptr;
};