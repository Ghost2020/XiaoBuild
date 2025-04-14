/**
  * @author cxx2020@outlook.com
  * @datetime 2024 -8:15 PM
 */
#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/Views/SListView.h"
#include "XiaoShareRedis.h"
#include <unordered_map>

namespace XiaoNetwork
{
	struct FNetworkConnectivity;
}
using namespace XiaoNetwork;

class SNetworkView final : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SNetworkView){}
	SLATE_END_ARGS()

	SNetworkView();
	virtual ~SNetworkView() override;
	
	void Construct(const FArguments& InArgs);

	virtual 

	void UpdateNetwork(const bool bTest = false);
protected:
	TSharedRef<ITableRow> OnGenerateRow(const TSharedPtr<FNetworkConnectivity> InDesc, const TSharedRef<STableViewBase>& InTableView) const;
	TSharedPtr<SWidget> OnContextMenu() const;
	void OnSortTable(const EColumnSortPriority::Type InPriority, const FName& InName, EColumnSortMode::Type InMode);
	EColumnSortMode::Type GetSortModeForColumn(const FName InColumnId) const;

private:
	TArray<TSharedPtr<FText>> ViewOptionArray;
	TArray<TSharedPtr<FText>> StatusOptionArray;
	TArray<TSharedPtr<FText>> VersionOptionArray;
	
	TArray<TSharedPtr<FNetworkConnectivity>> NetworkArray;
	TMap<FString, TSharedPtr<FNetworkConnectivity>> Ip2Desc;

	TSharedPtr<SListView<TSharedPtr<FNetworkConnectivity>>> NetworkListView = nullptr;
	FName ColumnIdToSort;
	EColumnSortMode::Type ActiveSortMode = EColumnSortMode::Ascending;
};
