/**
  * @author cxx2020@outlook.com
  * @date 10:49 AM
 */

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

class STextComboBox;
class SCheckBox;

template <typename NumericType>
class SSpinBox;

class SUbaAgentSettingsView final : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SUbaAgentSettingsView)
	{}
	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs);

protected:
	FString QueryComputerConfiguration();
	bool GetCanChangeDir() const;
	void ChangeDir(const FString& InNewDir);

private:
	FString HardwareDesc;
	uint32 LogicThreadNum = 0;

	TSharedPtr<SEditableTextBox> SaveDirFolderText = nullptr;
	TSharedPtr<SSpinBox<uint32>> MaxCpuSpinBox = nullptr;

	TSharedPtr<SEditableTextBox> ConifgPathText = nullptr;
	TSharedPtr<SEditableTextBox> AgentNameText = nullptr;

	TSharedPtr<SEditableTextBox> EventFileText = nullptr;

	TSharedPtr<SEditableTextBox> CryptoText = nullptr;
};
