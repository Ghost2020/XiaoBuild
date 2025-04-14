/**
  * @author cxx2020@outlook.com
  * @date 11:02 PM
 */
#pragma once

#include "CoreMinimal.h"
#include "Widgets/Views/STableRow.h"
#include "Widgets/Views/STableViewBase.h"

static const FName GBackupTableColumnIDIp = TEXT("Ip");
static const FName GBackupTableColumnIDRole = TEXT("Role");
static const FName GBackupTableColumnIDPriority = TEXT("Priority");
static const FName GBackupTableColumnIDStatus = TEXT("Status");
static const FName GBackupTableColumnIDDelete = TEXT("Delete");

template< typename OptionType >
class SComboBox;

template< typename OptionType >
class SSpinBox;

struct FRedisServerDesc;

class SBackupRow final : public SMultiColumnTableRow<TSharedPtr<FRedisServerDesc>>
{
public:
	DECLARE_DELEGATE_OneParam(FOnRoleChange, const TWeakPtr<FRedisServerDesc>&);
	DECLARE_DELEGATE_OneParam(FOnBackupChange, const TWeakPtr<FRedisServerDesc>&);

	SLATE_BEGIN_ARGS(SBackupRow){}
		SLATE_ARGUMENT(TSharedPtr<FRedisServerDesc>, BackupDesc)
		SLATE_EVENT(FOnRoleChange, OnRoleChange)
		SLATE_EVENT(FOnBackupChange, OnBackupChange)
		SLATE_EVENT(FOnBackupChange, OnBackupDelete)
	SLATE_END_ARGS()
	
	void Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InOwnerTableView);

	virtual TSharedRef<SWidget> GenerateWidgetForColumn(const FName& InColumnName) override;

private:
	TWeakPtr<FRedisServerDesc> BackupDesc;
	FOnRoleChange OnRoleChange;
	FOnBackupChange OnBackupChange;
	FOnBackupChange OnBackupDelete;
	TSharedPtr<class SImage> StatusImage = nullptr;
	TSharedPtr<SComboBox<TSharedPtr<FText>>> RoleComboBox = nullptr;
	TSharedPtr<SSpinBox<uint8>> PriorityBox = nullptr;
	bool bHoverd = false;
};
