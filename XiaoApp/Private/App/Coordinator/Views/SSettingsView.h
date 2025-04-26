/**
  * @author cxx2020@outlook.com
  * @date 11:02 PM
 */

#pragma once

#include "CoreMinimal.h"
#include "SBaseView.h"

class SWidgetSwitcher;
class SButton;
class SSlider;
template <typename NumericType>
class SSpinBox;
struct FRedisServerDesc;
struct FRedisEventDesc;

class SSettingsView final : public SBaseView
{
public:
	SLATE_BEGIN_ARGS(SSettingsView){}
		SLATE_EVENT(FOnQueueNotification, OnQueueNotification)
	SLATE_END_ARGS()
	
	void Construct(const FArguments& InArgs);

	virtual void OnUpdate(const bool bInRebuild) const override;

protected:
	void ConstructWidgets();

	void OnQuerySystem() const;
	void OnQueryBackup() const;

	TSharedRef<ITableRow> OnGenerateEventRow(const TSharedPtr<FRedisEventDesc> InDesc, const TSharedRef<STableViewBase>& InTableView) const;

	TSharedRef<ITableRow> OnGenerateBackRow(const TSharedPtr<FRedisServerDesc> InDesc, const TSharedRef<STableViewBase>& InTableView) const;
	TSharedPtr<SWidget> OnContextMenuOpening();
	void OnTableSort(const EColumnSortPriority::Type InSortPriority, const FName& InName, EColumnSortMode::Type InSortMode) const;
	EColumnSortMode::Type GetSortModeForColumn(const FName InColumnId) const;

	void OnRoleChange(const TWeakPtr<FRedisServerDesc>& InDesc) const;
	void OnUpdateBackup(const TWeakPtr<FRedisServerDesc>& InDesc) const;
	void OnDeleteBackup(const TWeakPtr<FRedisServerDesc>& InDesc) const;

	static float ToShowVal(const TSharedPtr<FText>& InText, const float& InRealData);
	static float ToRealVal (const TSharedPtr<FText>& InText, const float& InShowData);

	EVisibility GetButtonVisibility() const;
	bool GetDiscardButtonEnable() const;
	bool GetSaveButtonEnable() const;
	void OnDiscardModify();
	void OnCommitModify();

private:
	TArray<TSharedPtr<FRedisEventDesc>> EventArray;
	TSharedPtr<SListView<TSharedPtr<FRedisEventDesc>>> EventListView = nullptr;

	mutable TArray<TSharedPtr<FRedisServerDesc>> BackupArray;
	mutable TSharedPtr<SListView<TSharedPtr<FRedisServerDesc>>> BackupListView = nullptr;
	mutable TMap<FString, TWeakPtr<FRedisServerDesc>> Ip2Redis;

	mutable FName ColumnIdToSort;
	mutable EColumnSortMode::Type ActiveSortMode = EColumnSortMode::Ascending;

	TSharedPtr<TSharedPtr<FText>> DiskSpaceComboBox = nullptr;
	TArray<TSharedPtr<FText>> DiskUnitArray;
	TSharedPtr<SComboBox<TSharedPtr<FText>>> DiskSpaceUnitBox = nullptr;
	TSharedPtr<SComboBox<TSharedPtr<FText>>> VirtualMemoryUnitBox = nullptr;
	TSharedPtr<SComboBox<TSharedPtr<FText>>> PhysicalMemoryUnitBox = nullptr;

	TSharedPtr<SSlider> AvailableCPUSlider = nullptr;
	TSharedPtr<SSlider> DefaultAllocationSlider = nullptr;

	TSharedPtr<SCheckBox> AllowAgentCheckBox = nullptr;
	TSharedPtr<SCheckBox> ScheduleCleanupCheckBox = nullptr;

	TSharedPtr<SSpinBox<uint32>> ScheduleTimeSpinbox = nullptr;

	TSharedPtr<FSlateBrush> QRWeChatBrush = nullptr;
	
	TSharedPtr<SButton> DiscardAllButton = nullptr;
	TSharedPtr<SButton> SaveAllButton = nullptr;
};
