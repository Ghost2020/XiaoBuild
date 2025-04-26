/**
  * @author cxx2020@outlook.com
  * @date 10:49 AM
 */

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

template <typename NumericType>
class SSpinBox;
class SEditableTextBox;
template <typename NumericType>
class SComboBox;
struct FNetworkCoordinate;

class SNetworkCoordinatorView final : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SNetworkCoordinatorView){}
		SLATE_EVENT(FSimpleDelegate, OnTestPressed)
	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs, FNetworkCoordinate* InSettings);
	void Redraw() const;
	void SetStatus(const int32 bInStatus) const;

private:
	FNetworkCoordinate* Settings = nullptr;
	mutable bool bCanClickTestButton = true;
	FSimpleDelegate OnPressed;
	TSharedPtr<SEditableTextBox> IPBox = nullptr;
	TSharedPtr<SSpinBox<uint16>> PortSpinBox = nullptr;
	TSharedPtr<class SImage> NetworkStatus = nullptr;
};
