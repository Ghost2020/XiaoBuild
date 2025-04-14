/**
  * @author cxx2020@outlook.com
  * @date 11:02 PM
 */
#pragma once

#include "CoreMinimal.h"
#include "SBaseView.h"
#include "Widgets/Views/SListView.h"


namespace XiaoDB
{
	struct FUserDesc;
	struct FUserDetail;
} using namespace XiaoDB;


class SUsersView final : public SBaseView
{
public:
	SLATE_BEGIN_ARGS(SUsersView){}
		SLATE_EVENT(FOnQueueNotification, OnQueueNotification)
	SLATE_END_ARGS()
	
	void Construct(const FArguments& InArgs);

	virtual void OnUpdate(bool bInRebuild) const override;

protected:
	TSharedRef<ITableRow> OnGenerateRow(const TSharedPtr<FUserDesc> InDesc, const TSharedRef<STableViewBase>& InTableView) const;
	void OnSortTable(const EColumnSortPriority::Type InPriority, const FName& InName, EColumnSortMode::Type InMode);
	EColumnSortMode::Type GetSortModeForColumn(const FName InColumnId) const;
	FText GetUsername() const;

private:
	void OnAddUser(FUserDetail& InUserDesc);
	void OnRoleChange(const TWeakPtr<FUserDesc>& InUser) const;
	void OnDeleteUser(const TWeakPtr<FUserDesc>& InUser) const;

private:
	mutable TArray<TSharedPtr<FUserDesc>> UserArray;
	TSharedPtr<SListView<TSharedPtr<FUserDesc>>> UserListView = nullptr;
	mutable TMap<FString, TWeakPtr<FUserDesc>> User2Desc;

	TWeakPtr<SWindow> AddUserWindow = nullptr;
	FName ColumnIdToSort;
	EColumnSortMode::Type ActiveSortMode = EColumnSortMode::Ascending;
	TSharedPtr<STextBlock> UserNumText = nullptr;	
};
