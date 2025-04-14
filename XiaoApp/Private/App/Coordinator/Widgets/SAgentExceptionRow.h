/**
  * @author cxx2020@outlook.com
  * @date 11:02 PM
 */
#pragma once

#include "CoreMinimal.h"
#include "Widgets/Views/STableRow.h"
#include "Widgets/Views/STableViewBase.h"

static const FName GExceptTableColumnIDTimestamp = TEXT("Timestamp");
static const FName GExceptTableColumnIDIP = TEXT("IP");
static const FName GExceptTableColumnIDContent = TEXT("Content");


class FException;


class SAgentExceptionRow final : public SMultiColumnTableRow<TSharedPtr<FException>>
{
public:
	SLATE_BEGIN_ARGS(SAgentExceptionRow){}
		SLATE_ARGUMENT(TSharedPtr<FException>, ExceptionDesc)
	SLATE_END_ARGS()
	
	void Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InOwnerTableView);

	virtual TSharedRef<SWidget> GenerateWidgetForColumn(const FName& InColumnName) override;

private:
	TWeakPtr<FException> ExceptionDesc = nullptr;
	bool bHoverd = false;
};
