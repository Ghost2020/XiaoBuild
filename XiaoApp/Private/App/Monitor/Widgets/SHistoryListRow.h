/**
  * @author cxx2020@outlook.com
  * @date 11:02 PM
 */
#pragma once

#include "CoreMinimal.h"
#include "Widgets/Views/STableRow.h"
#include "Widgets/Views/STableViewBase.h"
#include "XiaoShare.h"

static const FName GHistoryColumnIDStatus = TEXT("Status");
static const FName GHistoryColumnIDStartTime = TEXT("StartTime");
static const FName GHistoryColumnIDDuration = TEXT("Duration");
static const FName GHistoryColumnIDMessage = TEXT("Message");
static const FName GHistoryColumnIDAutoRecovery = TEXT("AutoRecovery");
static const FName GHistoryColumnIDVersion = TEXT("Version");
static const FName GHistoryColumnIDType = TEXT("Type");
static const FName GHistoryColumnIDFilePath = TEXT("FilePath");

struct FBuildHistoryDesc;

DECLARE_DELEGATE_OneParam(FHistoryClicked, const TWeakPtr<FBuildHistoryDesc>);

class SHistoryListRow final : public SMultiColumnTableRow<TSharedPtr<FBuildHistoryDesc>>
{
public:
	SLATE_BEGIN_ARGS(SHistoryListRow){}
		SLATE_ARGUMENT(TSharedPtr<FBuildHistoryDesc>, BuildDesc)
		SLATE_EVENT(FHistoryClicked, OnItemDoubleClick)
	SLATE_END_ARGS()
	
	void Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InOwnerTableView);

	virtual TSharedRef<SWidget> GenerateWidgetForColumn(const FName& InColumnName) override;
	virtual FReply OnMouseButtonDoubleClick(const FGeometry& InMyGeometry, const FPointerEvent& InMouseEvent) override;

private:
	TWeakPtr<FBuildHistoryDesc> BuildDesc = nullptr;
	FHistoryClicked OnItemDoubleClick;
};
