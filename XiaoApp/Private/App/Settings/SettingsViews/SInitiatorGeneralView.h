/**
  * @author cxx2020@outlook.com
  * @date 10:49 AM
 */

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

class SCheckBox;
template <typename OptionType>
class SComboBox;
template <typename OptionType>
class SNumericEntryBox;
struct FInitiatorGeneral;

class SInitiatorGeneralView final : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SInitiatorGeneralView){}
	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs, FInitiatorGeneral* InSettings);

private:
	FInitiatorGeneral* Settings = nullptr;
	TSharedPtr<SCheckBox> RestartRemoteCheckBox = nullptr;
	TSharedPtr<SCheckBox> AvoidTaskCheckBox = nullptr;
	TSharedPtr<SCheckBox> EnableStandaloneCheckBox = nullptr;
	TSharedPtr<SCheckBox> LimitCoreCheckBox = nullptr;
	TSharedPtr<SNumericEntryBox<uint16>> LimitMaxCoreBox = nullptr;
};
