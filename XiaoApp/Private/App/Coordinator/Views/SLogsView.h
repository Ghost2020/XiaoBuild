/**
  * @author cxx2020@outlook.com
  * @date 11:02 PM
 */
#pragma once

#include "CoreMinimal.h"
#include "SBaseView.h"
#include <string>
#include <unordered_map>

class FException;

class SLogsView final : public SBaseView
{
public:
	SLATE_BEGIN_ARGS(SLogsView){}
		SLATE_EVENT(FOnQueueNotification, OnQueueNotification)
	SLATE_END_ARGS()
	
	void Construct(const FArguments& InArgs);

	virtual void OnUpdate(const bool bInRebuild) const;
	 
protected:
	TSharedRef<ITableRow> OnGenerateRow(const TSharedPtr<FException> InDesc, const TSharedRef<STableViewBase>& InTableView) const;
	TSharedPtr<SWidget> OnContextMenuOpening();
	void OnSortTable(const EColumnSortPriority::Type InPriority, const FName& InName, EColumnSortMode::Type InMode);
	EColumnSortMode::Type GetSortModeForColumn(const FName InColumnId) const;

private:
	mutable TArray<TSharedPtr<FException>> ExceptionArray;
	TSharedPtr<SListView<TSharedPtr<FException>>> ExceptionListView = nullptr;
	mutable std::unordered_map<std::string, TSharedPtr<FException>> Key2Exception;

	FName ColumnIdToSort;
	EColumnSortMode::Type ActiveSortMode = EColumnSortMode::Ascending;
};
