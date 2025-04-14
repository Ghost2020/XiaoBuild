/**
  * @author cxx2020@outlook.com
  * @date 11:02 PM
 */
#pragma once

#include "CoreMinimal.h"
#include "Widgets/Views/STableRow.h"
#include "Widgets/Views/STableViewBase.h"

static const FName GEventTableColumnIDChannel = TEXT("Channel");
static const FName GEventTableColumnIDMessage = TEXT("Message");
static const FName GEventTableColumnIDDesc = TEXT("Desc");

struct FRedisEventDesc;

class SEventRow final : public SMultiColumnTableRow<TSharedPtr<FRedisEventDesc>>
{
public:
	SLATE_BEGIN_ARGS(SEventRow){}
		SLATE_ARGUMENT(TSharedPtr<FRedisEventDesc>, EventDesc)
	SLATE_END_ARGS()
	
	void Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InOwnerTableView);

	virtual TSharedRef<SWidget> GenerateWidgetForColumn(const FName& InColumnName) override;

private:
	TWeakPtr<FRedisEventDesc> EventDesc = nullptr;
};
