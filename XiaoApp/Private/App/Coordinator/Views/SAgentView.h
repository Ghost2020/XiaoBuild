/**
  * @author cxx2020@outlook.com
  * @date 11:02 PM
 */
#pragma once

#include "SBaseView.h"
#include "Widgets/Views/SHeaderRow.h"
#include "Widgets/Views/SListView.h"
#include "XiaoShare.h"

class FAgentProto;
namespace Xiao
{
	class FNetworkTrace;
}
using namespace Xiao;

class SAgentView final : public SBaseView
{
public:
	SLATE_BEGIN_ARGS(SAgentView){}
		SLATE_EVENT(FOnQueueNotification, OnQueueNotification)
	SLATE_END_ARGS()

	SAgentView();
	
	void Construct(const FArguments& InArgs);

	virtual ~SAgentView() override;

	virtual void OnUpdate(const bool bRebuild) const override;

	bool IsEnableEdit() const;

protected:
	void InitData();
	void RefreshIndex() const;
	TSharedRef<ITableRow> OnGenerateRow(const TSharedPtr<FAgentProto> InAgent, const TSharedRef<STableViewBase>& InTableView) const;
	TSharedPtr<SWidget> OnGenerateContextMenu() const;
	void OnMouseDoubleClick(const TSharedPtr<FAgentProto> InProto) const;
	void OnGenerateTable() const;
	EColumnSortMode::Type GetSortModeForColumn(FName InColumnId) const;
	void OnTableSort(EColumnSortPriority::Type InSortPriority, const FName& InName, EColumnSortMode::Type InSortMode) const;
	FReply OnTableSave();

	int32 GetSelectedAgentsVal(const int32 InDeffIndex, TFunction<int32(const TSharedPtr<FAgentProto>&)> InGetter) const;
	ECheckBoxState GetSelectedAgentsCheck(TFunction<bool(const TSharedPtr<FAgentProto>&)> InGetter) const;
	void SetSeletecedAgentsParams(TFunction<void(const TSharedPtr<FAgentProto>&)> InSetter) const;

private:
	void OnAgentChange(const TWeakPtr<FAgentProto>& InAgent) const;
	void OnAgentsChange(const TArray<TSharedPtr<FAgentProto>>& InAgents) const;
	void OnSearchChange(const FText& InFiler, ETextCommit::Type InCommitType);
	void DeleteAgents(const TSet<FString>& InAgentIds, const bool bRebuild) const;
	void PostStopBuild(const TSharedPtr<FAgentProto> InProto) const;

private:
	TSharedPtr<FJsonObject> AgentConfigJson = nullptr;
	FString FilterString;
	mutable FName ColumnIdToSort;
	mutable EColumnSortMode::Type ActiveSortMode = EColumnSortMode::Ascending;

	TUniquePtr<FNetworkTrace> NetworkTrace = nullptr;

	TSharedPtr<class SSearchBox> SearchBox = nullptr;
	TSharedPtr<SScrollBar> ExternalScrollbar = nullptr;

	TSharedPtr<SListView<TSharedPtr<FAgentProto>>> AgentsListView = nullptr;
	mutable TArray<TSharedPtr<FAgentProto>> AgentsArray;
	mutable TArray<TSharedPtr<FAgentProto>> FiltedArray;
	mutable TMap<FString, TWeakPtr<FAgentProto>> Id2Agent;
	TSharedPtr<SHeaderRow> TableHeader = nullptr;

	TArray<TSharedPtr<FString>> ShowOptionList;
	TSharedPtr<STextBlock> ShowNumText = nullptr;

	TArray<TSharedPtr<FText>> TableShowArray;
	TSharedPtr<STextBlock> TableShowText = nullptr;

	mutable uint32 AvailableAgentNum = 0;
	mutable uint32 MaxTheoryHelpCore = 0;
	mutable uint32 MaxTrueHelpCore = 0;
};
