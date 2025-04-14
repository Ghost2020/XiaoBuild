/**
  * @author cxx2020@outlook.com
  * @date 11:02 PM
 */

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

class SAgentFilter final : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SAgentFilter)
	{}
	SLATE_END_ARGS()
	
	void Construct(const FArguments& InArgs);

private:
	TArray<TSharedPtr<FString>> LicenseAdditionalSource;
	TSharedPtr<class STextBlock> LicenseAdditionalText = nullptr;
};
