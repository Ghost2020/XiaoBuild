/**
  * @author cxx2020@outlook.com
  * @date 5:57 PM
 */

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SWindow.h"
#include "Widgets/Views/STableRow.h"
#include "XiaoShare.h"

class SGroupItemRow final : public SMultiColumnTableRow<TSharedPtr<FBuildGroup>>
{
	SLATE_BEGIN_ARGS(SGroupItemRow){}
	SLATE_END_ARGS();

	void Construct(const FArguments& InArgs, TSharedPtr<FBuildGroup> InRowData, const TSharedRef<STableViewBase>& InOwnerTable);

	virtual TSharedRef<SWidget> GenerateWidgetForColumn(const FName& ColumnName) override;

	void SetHintText(const FText& InError) const;
	void SetSelected(const bool bSelected);

private:
	TSharedPtr<class SEditableTextBox> NameBox = nullptr;
	TSharedPtr<class STextBlock> NumBlock = nullptr;
	TSharedPtr<SButton> DeleteButton = nullptr;

protected:
	TWeakPtr<FBuildGroup> WeakRow = nullptr;
};

class SManageBuildGroups final : public SWindow
{
public:
	SLATE_BEGIN_ARGS(SManageBuildGroups)
	{}
	SLATE_END_ARGS()

	virtual ~SManageBuildGroups() override;
	
	void Construct(const FArguments& InArgs);

	TSharedRef<ITableRow> OnGenerateRow(TSharedPtr<FBuildGroup> BuildItem, const TSharedRef<STableViewBase>& OwnerTable) const;

private:
	TArray<TSharedPtr<FBuildGroup>> BuildGroupsArray;
	TSharedPtr<SListView<TSharedPtr<FBuildGroup>>> GroupListView = nullptr;
};
