/**
  * @author cxx2020@outlook.com
  * @date 11:02 PM
 */
#pragma once

#include "CoreMinimal.h"
#include "Widgets/Views/STableRow.h"
#include "Widgets/Views/STableViewBase.h"

namespace XiaoNetwork
{
	struct FNetworkConnectivity;
}
using namespace XiaoNetwork;

static const FName GNetworkColumnIDUpdate = TEXT("Update");
static const FName GNetworkColumnIDIcon = TEXT("Icon");
static const FName GNetworkColumnIDName = TEXT("Name");
static const FName GNetworkColumnIDStatus = TEXT("Status");
static const FName GNetworkColumnIDPerformance = TEXT("Performance");
static const FName GNetworkColumnIDReceivePerfor = TEXT("ReceivePerfor");
static const FName GNetworkColumnIDSendPerfor = TEXT("SendPerfor");
static const FName GNetworkColumnIDRoundTrip = TEXT("RoundTrip");
static const FName GNetworkColumnIDIPAddress = TEXT("IPAddress");


class SNetworkListRow final : public SMultiColumnTableRow<TSharedPtr<FNetworkConnectivity>>
{
public:
	SLATE_BEGIN_ARGS(SNetworkListRow){}
		SLATE_ARGUMENT(TSharedPtr<FNetworkConnectivity>, NetworkDesc)
	SLATE_END_ARGS()
	
	void Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InOwnerTableView);

	virtual TSharedRef<SWidget> GenerateWidgetForColumn(const FName& InColumnName) override;

	FReply OnNetworkTest() const;

protected:
	bool IsNetworkValid() const;
	const FSlateBrush* GetIconImage() const;
	FText GetConnectText() const;
	FText GetPerformanceText() const;
	FSlateColor GetPerformanceColor() const;
	FText GetPerformanceData() const;
	FText GetReceiveText() const;
	FText GetReceiveDataText() const;
	FText GetSendText() const;
	FText GetSendDataText() const;
	FText GetRoundTripTimeText() const;
	FText GetEveryTripText() const;

private:
	TWeakPtr<FNetworkConnectivity> NetworkDesc = nullptr;
};
