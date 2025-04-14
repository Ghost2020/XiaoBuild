/**
  * @author cxx2020@outlook.com
  * @date 11:02 PM
 */
#pragma once

#include "CoreMinimal.h"
#include "Widgets/SWindow.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Framework/Docking/TabManager.h"

static constexpr int32 GWindow_Width = 400;
static constexpr int32 GWindow_Height = 200;


class SExpandableArea;
class SCategoryWidget;


class SAgentSettingsWindow final : public SWindow
{
	SLATE_BEGIN_ARGS(SAgentSettingsWindow){}
	SLATE_END_ARGS()

public:
	SAgentSettingsWindow();
	virtual	~SAgentSettingsWindow() override;

	void Construct(const FArguments& Args);

	void OnExit(const TSharedRef<SWindow>& InWindow) const;

protected:
	void ConstructWidgets();
	void OnExit() const;

private:
	bool LoadSettings();
	bool OnCanCommit() const;
	FReply OnCommit();
	void ChangeStatus(const bool Status) const;
	void OnQueueNotification(int8 InStatus, const FText& InText) const;
	void AsyncNetworkTest();

private:
	TSharedPtr<FTabManager> TabManager = nullptr;

	TSharedPtr<class SNotificationList> NotificationList = nullptr;

	TSharedPtr<class SErrorText> ErrorText = nullptr;
#pragma region Expandable
	TSharedPtr<SExpandableArea> AgentArea = nullptr;
	TSharedPtr<SExpandableArea> NetworkArea = nullptr;
	TSharedPtr<SExpandableArea> InitiatorArea = nullptr;
#pragma endregion Expandable

#pragma region Category
	TSharedPtr<SCategoryWidget> NetworkCoordinatorCategory = nullptr;

	TSharedPtr<SCategoryWidget> AgentGeneralCategory = nullptr;
	TSharedPtr<SCategoryWidget> AgentAdvancedCategory = nullptr; 

	TSharedPtr<SCategoryWidget> InitiatorAdvancedCategory = nullptr;

#pragma endregion Category

#pragma region WidgetView
	TSharedPtr<class SAgentGeneralView> AgentGeneral = nullptr;
	TSharedPtr<class SUbaAgentSettingsView> UbaAgentAdvanced = nullptr;

	TSharedPtr<class SNetworkGeneralView> NetworkGeneral = nullptr;
	TSharedPtr<class SNetworkCoordinatorView> NetworkCoordinator = nullptr;

	TSharedPtr<class SInitiatorGeneralView> InitiatorGeneral = nullptr;
	TSharedPtr<class SInitiatorAdvancedView> InitiatorAdvanced = nullptr;

#pragma endregion WidgetView

	TSharedPtr<SWidgetSwitcher> WidgetSwitcher = nullptr;
};