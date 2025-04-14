/**
  * @author cxx2020@outlook.com
  * @date 10:49 AM
 */

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Input/SNumericDropDown.h"

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

private:
	FString HardwareDesc;
	uint32 LogicThreadNum = 0;

	TSharedPtr<SEditableTextBox> SaveDirFolderText = nullptr;
	TSharedPtr<SSpinBox<uint16>> ListenPortSpinBox = nullptr;
	TSharedPtr<SSpinBox<uint32>> MaxCpuSpinBox = nullptr;
	TSharedPtr<SSpinBox<uint32>> MulcpuSpinBox = nullptr;
	TSharedPtr<SSpinBox<uint32>> MaxConSpinBox = nullptr;
	TSharedPtr<SSpinBox<uint32>> MaxWorkersSpinBox = nullptr;
	TSharedPtr<SSpinBox<uint32>> CapacitySpinBox = nullptr;

	TSharedPtr<SEditableTextBox> ConifgPathText = nullptr;
	TSharedPtr<SEditableTextBox> AgentNameText = nullptr;
	TSharedPtr<SSpinBox<uint32>> StatsSpinBox = nullptr;

	TSharedPtr<SCheckBox> VerboseCheckBox = nullptr;
	TSharedPtr<SCheckBox> LogCheckBox = nullptr;
	TSharedPtr<SCheckBox> NoCustoMallocCheckBox = nullptr;
	TSharedPtr<SCheckBox> StoreRawCheckBox = nullptr;
	TSharedPtr<SCheckBox> SendRawCheckBox = nullptr;

	TSharedPtr<SSpinBox<uint32>> SendSizeSpinBox = nullptr;
	TSharedPtr<SCheckBox> NoPollCheckBox = nullptr;
	TSharedPtr<SCheckBox> NoStoreCheckBox = nullptr;
	TSharedPtr<SCheckBox> ResetStoreCheckBox = nullptr;
	TSharedPtr<SCheckBox> QuietCheckBox = nullptr;

	TSharedPtr<SCheckBox> BinaryVersionCheckBox = nullptr;
	TSharedPtr<SCheckBox> SummaryCheckBox = nullptr;

	TSharedPtr<SEditableTextBox> EventFileText = nullptr;

	TSharedPtr<SCheckBox> SentryCheckBox = nullptr;
	TSharedPtr<SCheckBox> NoProxyCheckBox = nullptr;
	TSharedPtr<SCheckBox> KillRandomCheckBox = nullptr;

	TSharedPtr<SSpinBox<uint32>> MemWaitSpinBox = nullptr;
	TSharedPtr<SSpinBox<uint32>> MemKillSpinBox = nullptr;
	TSharedPtr<SEditableTextBox> CryptoText = nullptr;
};
