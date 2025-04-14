/**
  * @author cxx2020@outlook.com
  * @date 11:02 PM
 */

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "XiaoShare.h"

class STextBlock;
class SPie;

class SAgentStatus final : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SAgentStatus){}
	SLATE_END_ARGS()
	
	void Construct(const FArguments& InArgs, const FOnlineAgentActivity& InOnlineActivity, const FLicenseUsage& InLicenseUsage);

protected:
	void OnUpdate(const FOnlineAgentActivity& InOnlineActivity, const FLicenseUsage& InLicenseUsage) const;

private:
	TSharedPtr<STextBlock> OnlineMachineNumText = nullptr;
	TSharedPtr<STextBlock> BusyNumText = nullptr;
	TSharedPtr<STextBlock> IdleNumText = nullptr;
	TSharedPtr<SPie> OnlineMachinePie = nullptr;

	TSharedPtr<STextBlock> BusyCoresNumText = nullptr;
	TSharedPtr<STextBlock> HelperNumText = nullptr;
	TSharedPtr<STextBlock> InitiatorNumText = nullptr;
	TSharedPtr<SPie> BusyCoresPie = nullptr;

	TSharedPtr<STextBlock> BuildsNumText = nullptr;
	TSharedPtr<SPie> BuildsPie = nullptr;

	TSharedPtr<STextBlock> InitiatorsNumText = nullptr;
	TSharedPtr<STextBlock> AssignedNumText = nullptr;
	TSharedPtr<STextBlock> AvailableNumText = nullptr;
	TSharedPtr<SPie> InitiatorsPie = nullptr;

	TSharedPtr<STextBlock> HelperCoresNumText = nullptr;
	TSharedPtr<STextBlock> HelperCoresPoolNumText = nullptr;
};
