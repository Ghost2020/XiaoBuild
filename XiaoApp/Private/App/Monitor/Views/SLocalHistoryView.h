/**
  * @author cxx2020@outlook.com
  * @datetime 2024 -8:15 PM
 */

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/Views/SListView.h"
#include "../Widgets/SHistoryListRow.h"
#include "XiaoShare.h"

class SLocalHistoryView final : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SLocalHistoryView){}
		SLATE_EVENT(FHistoryClicked, OnItemDoubleClick)
	SLATE_END_ARGS()
	
	void Construct(const FArguments& InArgs);

	bool GetSelectedFiles(TArray<FString>& OutFiles) const;
	void UpdateHistory(const bool bInForce = false);

protected:
	void InitData();
	TSharedRef<ITableRow> OnGenerateRow(const TSharedPtr<FBuildHistoryDesc> InDesc, const TSharedRef<STableViewBase>& InTableView) const;
	TSharedPtr<SWidget> OnContextMenu();
	void OnSortTable(const EColumnSortPriority::Type InPriority, const FName& InName, EColumnSortMode::Type InMode);
	EColumnSortMode::Type GetSortModeForColumn(const FName InColumnId) const;

	void OnShowBuild() const;
	bool OnCanShowBuild() const;
	void OnReplayBuild() const;
	void OnClearHistory();

private:
	TSharedPtr<class FTabManager> TabManager = nullptr;

	FHistoryClicked OnItemDoubleClick;
	bool bHasData = false;
	bool bHasHistory = false;
	
	TArray<TSharedPtr<FBuildHistoryDesc>> BuildArray;
	TSharedPtr<SListView<TSharedPtr<FBuildHistoryDesc>>> BuildListView = nullptr;
	TMap<FString, TSharedPtr<FBuildHistoryDesc>> Path2Desc;
	FName ColumnIdToSort;
	EColumnSortMode::Type ActiveSortMode = EColumnSortMode::Ascending;
	
	bool bAutoUpdate = false;
	uint32 FailureNum = 0;
	uint32 CancelNum = 0;
};
