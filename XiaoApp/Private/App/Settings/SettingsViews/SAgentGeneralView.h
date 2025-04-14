/**
  * @author cxx2020@outlook.com
  * @date 10:49 AM
 */

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

class STextBlock;
template<typename NumericType>
class SSpinBox;
class STextComboBox;
class SCheckBox;
struct FAgentGeneral;
struct FInstallFolder;

class SAgentGeneralView final : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SAgentGeneralView){}
	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs);

	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;

	bool OnCanCommit() const;
	FReply OnCommit();
	FReply OnRevert();

protected:
	void Update(const bool bInRebuildTable = false);
	FText OnServiceWorkState() const;
	void OnStartService();
	bool OnGetStartEnable() const;
	void OnStopService();
	bool OnGetStopEnable() const;

	TSharedRef<ITableRow> OnGenerateRow(const TSharedPtr<FInstallFolder> InDesc, const TSharedRef<STableViewBase>& InTableView) const;
	static void OnSortTable(const EColumnSortPriority::Type InPriority, const FName& InName, EColumnSortMode::Type InMode);
	EColumnSortMode::Type GetSortModeForColumn(const FName InColumnId) const;

private:
	float TotalTime = 5.f;
	bool bAgentServiceState = false;

	TSharedPtr<SCheckBox> XiaoBuildCheckBox = nullptr;
	TSharedPtr<SCheckBox> AgentHelperCheckBox = nullptr;

	TArray<TSharedPtr<FString>> LauguageOptionList;
	TSharedPtr<STextBlock> LauguageText = nullptr;

	TSharedPtr<STextBlock> AgentStateText = nullptr;

	TArray<TSharedPtr<FInstallFolder>> ModiefyFolderArray;
	TArray<TSharedPtr<FInstallFolder>> OriginlFolderArray;
	TSharedPtr<SListView<TSharedPtr<FInstallFolder>>> FolderListView = nullptr;

	FName ColumnIdToSort;
	EColumnSortMode::Type ActiveSortMode = EColumnSortMode::Ascending;
};
