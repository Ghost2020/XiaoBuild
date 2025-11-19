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

	TSharedPtr<SEditableTextBox> CryptoText = nullptr;

	TArray<TSharedPtr<FString>> LevelArray;
};
