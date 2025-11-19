/**
  * @author cxx2020@outlook.com
  * @date 11:02 PM
 */
#pragma once

#include "SWizardView.h"
#include "Widgets/Views/SListView.h"

struct FInstallFolder;

class SInstallationFolderView final : public SWizardView
{
public:
	SLATE_BEGIN_ARGS(SInstallationFolderView)
	{}
	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs, const bool InType);
	
	virtual void OnResetState() override;
	virtual TWeakPtr<SWizardView> GetNext() override;
	virtual bool OnCanNext() override;

	TSharedRef<ITableRow> OnGenerateRow(const TSharedPtr<FInstallFolder> InDesc, const TSharedRef<STableViewBase>& InTableView) const;
	EColumnSortMode::Type GetSortModeForColumn(const FName InColumnId) const;

protected:
	void ValidationFolder(const FString& InFolder);
	EVisibility CanInstallUnrealEngine() const;

private:
	bool bType = true;

	FString DefualtCASDir;
	
	TSharedPtr<class SEditableTextBox> InstallFolderBox = nullptr;
	TSharedPtr<class SSSLSettingsView> SSLSettingsView = nullptr;
	TSharedPtr<class SInstallProgressView> InStallView = nullptr;

	TArray<TSharedPtr<FInstallFolder>> FolderArray;
	TSharedPtr<SListView<TSharedPtr<FInstallFolder>>> FolderListView = nullptr;

	FName ColumnIdToSort;
	EColumnSortMode::Type ActiveSortMode = EColumnSortMode::Ascending;
	
	bool bPass = false;
};
