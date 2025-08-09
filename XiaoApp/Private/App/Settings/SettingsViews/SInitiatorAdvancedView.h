/**
  * @author cxx2020@outlook.com
  * @date 10:49 AM
 */

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

class SCheckBox;

template <typename NumericType>
class SSpinBox;

class SInitiatorAdvancedView final : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SInitiatorAdvancedView)	
	{}
	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs);

protected:
	bool GetCanChangeDir() const;
	void ChangeDir(const FString& InNewDir);

private:

	TSharedPtr<SEditableTextBox> SaveDirFolderText = nullptr;

	TSharedPtr<SCheckBox> LogCheckBox = nullptr;
	TSharedPtr<SCheckBox> StandaloneCheckBox = nullptr;
	TSharedPtr<SCheckBox> QuietCheckBox = nullptr;
	TSharedPtr<SSpinBox<uint32>> CapacitySpinBox = nullptr;
	TSharedPtr<SCheckBox> CheckCasCheckBox = nullptr;
	TSharedPtr<SCheckBox> DeletecasCheckBox = nullptr;
	TSharedPtr<SCheckBox> GetCasCheckBox = nullptr;
	TSharedPtr<SCheckBox> SummaryCheckBox = nullptr;
	TSharedPtr<SCheckBox> NoCustoMallocCheckBox = nullptr;
	TSharedPtr<SCheckBox> AllowMemoryMapsCheckBox = nullptr;
	TSharedPtr<SCheckBox> EnableStdOutCheckBox = nullptr;
	TSharedPtr<SCheckBox> StoreRawCheckBox = nullptr;

	TSharedPtr<SSpinBox<uint32>> MaxLocalCoreSpinBox = nullptr;
	TSharedPtr<SSpinBox<uint32>> MaxCpuSpinBox = nullptr;
	TSharedPtr<SSpinBox<uint32>> MaxConSpinBox = nullptr;

	TSharedPtr<SCheckBox> VisualizerCheckBox = nullptr;
	TSharedPtr<SEditableTextBox> CryptoText = nullptr;

	TArray<TSharedPtr<FString>> LevelArray;
	TSharedPtr<SSpinBox<int32>> KeepBox = nullptr;
};
