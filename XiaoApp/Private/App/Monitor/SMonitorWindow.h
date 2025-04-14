/**
  * @author cxx2020@outlook.com
  * @date 11:02 PM
 */
#pragma once

#include "CoreMinimal.h"
#include "Widgets/SWindow.h"
#include "Widgets/DeclarativeSyntaxSupport.h"

class FMenuBuilder;
class SSimpleButton;

class SMonitorWindow final : public SWindow
{
	SLATE_BEGIN_ARGS(SMonitorWindow){}
	SLATE_END_ARGS()
	
	SMonitorWindow();
	virtual	~SMonitorWindow() override;

	void Construct(const FArguments& Args);
	void ConstructWidgets();

	// ~SCompoundWidget Begin
	virtual FReply OnDragOver(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent) override;
	virtual FReply OnDrop(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent) override;
	virtual FReply OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) override;
	// ~SCompoundWidget Finish

	void Update(const int InIndex) const;

	void OnExit(const TSharedRef<SWindow>& InWindow) const;

protected:
	TSharedRef<SWidget> MakeMainMenu();
	
	void FillFileMenu(FMenuBuilder& MenuBuilder);
	void FillViewMenu(FMenuBuilder& MenuBuilder);
	void OnLoadBuildFile();
	void OnToggleFullScreen();
	void OnExit() const;

private:
	void SetLockedState(const bool InbLock) const;
	EVisibility GetButtonVisibility() const;
	bool GetCanSeeViewButton() const;
private:
	TSharedPtr<class SBorder> MaskBorder = nullptr;
	
	TSharedPtr<class SWidgetSwitcher> ViewSwitcher = nullptr;
	TSharedPtr<class SBuildView> BuildView = nullptr;
	TSharedPtr<SSimpleButton> ButtonBuild = nullptr;
	TSharedPtr<class SLocalHistoryView> HistoryView = nullptr;
	TSharedPtr<SSimpleButton> ButtonHistory = nullptr;
	TSharedPtr<class SNetworkView> NetworkView = nullptr;
	TSharedPtr<SSimpleButton> ButtonNetwork = nullptr;
	TSharedPtr<SSimpleButton> HelpButton = nullptr;

	bool bFullScreen = false;
	bool bImportFile = false;
};