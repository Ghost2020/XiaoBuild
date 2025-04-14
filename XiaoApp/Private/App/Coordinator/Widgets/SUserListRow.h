/**
  * @author cxx2020@outlook.com
  * @date 11:02 PM
 */
#pragma once

#include "CoreMinimal.h"
#include "Widgets/Views/STableRow.h"
#include "Widgets/Views/STableViewBase.h"
#include "Database/Users.h"

static const FName GUserTableColumnIDID = TEXT("ID");
static const FName GUserTableColumnIDUsername = TEXT("Username");
static const FName GUserTableColumnIDRole = TEXT("Role");
static const FName GUserTableColumnIDLastLogin = TEXT("LastLogin");
static const FName GUserTableColumnIDStatus = TEXT("Status");
static const FName GUserTableColumnIDAttempts = TEXT("Attempts");
static const FName GUserTableColumnIDCreateBy = TEXT("CreateBy");
static const FName GUserTableColumnIDDelete = TEXT("Delete");

template< typename OptionType >
class SComboBox;

using namespace XiaoDB;


class SUserListRow final : public SMultiColumnTableRow<TSharedPtr<FUserDesc>>
{
public:
	DECLARE_DELEGATE_OneParam(FOnUserChange, const TWeakPtr<FUserDesc>&);
	
	SLATE_BEGIN_ARGS(SUserListRow){}
		SLATE_ARGUMENT(TSharedPtr<FUserDesc>, UserDesc)
		SLATE_EVENT(FOnUserChange, OnUserChange)
		SLATE_EVENT(FOnUserChange, OnUserDelete)
	SLATE_END_ARGS()
	
	void Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InOwnerTableView);

	virtual TSharedRef<SWidget> GenerateWidgetForColumn(const FName& InColumnName) override;

protected:
	FReply OnDeleteUser() const;

private:
	TWeakPtr<FUserDesc> UserDesc;
	FOnUserChange OnUserChange;
	FOnUserChange OnUserDelete;
	TSharedPtr<class SImage> StatusImage = nullptr;
	TSharedPtr<SComboBox<TSharedPtr<FText>>> RoleComboBox = nullptr;
	TSharedPtr<SComboBox<TSharedPtr<FText>>> StatusComboBox = nullptr;
	TSharedPtr<class SButton> DeleteButton = nullptr;
	bool bHoverd = false;
};
