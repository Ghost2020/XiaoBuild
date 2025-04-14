/**
  * @author cxx2020@outlook.com
  * @date 11:02 PM
 */
#include "SNetworkListRow.h"
#include "XiaoStyle.h"
#include "XiaoShareNetwork.h"
#include "SlateOptMacros.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Images/SThrobber.h"

#define LOCTEXT_NAMESPACE "SNetworkListRow"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SNetworkListRow::Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InOwnerTableView)
{
	check(InArgs._NetworkDesc.IsValid());
	NetworkDesc = InArgs._NetworkDesc;
	
	SMultiColumnTableRow::Construct(FSuperRowType::FArguments().Padding(2.0f), InOwnerTableView);
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION

TSharedRef<SWidget> SNetworkListRow::GenerateWidgetForColumn(const FName& InColumnName)
{
	if(NetworkDesc.IsValid())
	{
		if(const auto Network = NetworkDesc.Pin())
		{
			if(InColumnName == GNetworkColumnIDUpdate)
			{
				return SNew(SButton)
				.ButtonColorAndOpacity(FColor::Transparent)
				.Content()
				[
					SNew(SHorizontalBox)
					+SHorizontalBox::Slot().HAlign(HAlign_Center).VAlign(VAlign_Center)
					[
						SNew(SImage)
						.Image(FXiaoStyle::Get().GetBrush(TEXT("Icons.update")))
						.Visibility_Lambda([this]() 
						{
							return NetworkDesc.Pin()->Connectivety != ETestConnectivity::Test_Progress ? EVisibility::Visible : EVisibility::Collapsed;
						})
					]
					+ SHorizontalBox::Slot().HAlign(HAlign_Center).VAlign(VAlign_Center)
					[
						SNew(SThrobber)
						.Visibility_Lambda([this]()
						{
							return NetworkDesc.Pin()->Connectivety != ETestConnectivity::Test_Progress ? EVisibility::Collapsed : EVisibility::Visible;
						})
					]
				]
				.OnClicked_Raw(this, &SNetworkListRow::OnNetworkTest)
				.IsEnabled_Lambda([this]() 
				{
					if (NetworkDesc.IsValid())
					{
						return NetworkDesc.Pin()->Connectivety == ETestConnectivity::Test_Progress ? false : true;
					}
					return false;
				});
			}
			if(InColumnName == GNetworkColumnIDIcon)
			{
				return V_CENTER_WIGET(SNew(SImage).Image_Raw(this, &SNetworkListRow::GetIconImage));
			}
			if(InColumnName == GNetworkColumnIDName)
			{
				return V_CENTER_WIGET(SNew(STextBlock)
					.Text_Lambda([this]() {
						return FText::FromString(NetworkDesc.IsValid() ? NetworkDesc.Pin()->Name : TEXT("---"));
						}));
			}
			if(InColumnName == GNetworkColumnIDStatus)
			{
				return SNew(SVerticalBox)
				+SVerticalBox::Slot()
				[
					SNew(STextBlock).Text_Raw(this, &SNetworkListRow::GetConnectText)
				]
				+SVerticalBox::Slot()
				[
					V_CENTER_WIGET(SNew(STextBlock)
					.Text_Lambda([this]()
					{
						return FText::FromString(FString::Printf(TEXT("%f Mbps"), IsNetworkValid() ? NetworkDesc.Pin()->Performance : 0.0f));
					}))
				];
			}
			if(InColumnName == GNetworkColumnIDPerformance)
			{
				return SNew(SVerticalBox)
				+SVerticalBox::Slot()
				[
					V_CENTER_WIGET(SNew(STextBlock)
					.Text_Raw(this, &SNetworkListRow::GetPerformanceText)
					.ColorAndOpacity_Raw(this, &SNetworkListRow::GetPerformanceColor))
				]
				+SVerticalBox::Slot()
				[
					V_CENTER_WIGET(SNew(STextBlock).Text_Raw(this, &SNetworkListRow::GetPerformanceData))
				];
			}
			if(InColumnName == GNetworkColumnIDReceivePerfor)
			{
				return SNew(SVerticalBox)
				+SVerticalBox::Slot()
				[
					V_CENTER_WIGET(SNew(STextBlock).Text_Raw(this, &SNetworkListRow::GetReceiveText))
				]
				+SVerticalBox::Slot()
				[
					V_CENTER_WIGET(SNew(STextBlock).Text_Raw(this, &SNetworkListRow::GetReceiveDataText))
				];
			}
			if(InColumnName == GNetworkColumnIDSendPerfor)
			{
				return SNew(SVerticalBox)
				+SVerticalBox::Slot()
				[
					V_CENTER_WIGET(SNew(STextBlock).Text_Raw(this, &SNetworkListRow::GetSendText))
				]
				+SVerticalBox::Slot()
				[
					V_CENTER_WIGET(SNew(STextBlock).Text_Raw(this, &SNetworkListRow::GetSendDataText))
				];
			}
			if(InColumnName == GNetworkColumnIDRoundTrip)
			{
				return SNew(SVerticalBox)
				+SVerticalBox::Slot()
				[
					V_CENTER_WIGET(SNew(STextBlock).Text_Raw(this, &SNetworkListRow::GetRoundTripTimeText))
				]
				+SVerticalBox::Slot()
				[
					V_CENTER_WIGET(SNew(STextBlock).Text_Raw(this, &SNetworkListRow::GetEveryTripText))
				];
			}
			if(InColumnName == GNetworkColumnIDIPAddress)
			{
				return V_CENTER_WIGET(
						SNew(STextBlock)
						.Text_Lambda([this]() {
							return FText::FromString(NetworkDesc.IsValid() ? NetworkDesc.Pin()->RemoteConnection : TEXT(""));
						}
					)
				);
			}
		}
	}
	return SNullWidget::NullWidget;
}

FReply SNetworkListRow::OnNetworkTest() const
{
	if (NetworkDesc.IsValid())
	{
		NetworkDesc.Pin()->OnNetworkTest();
	}
	return FReply::Handled();
}

bool SNetworkListRow::IsNetworkValid() const
{
	return NetworkDesc.IsValid() ? (NetworkDesc.Pin()->Connectivety == Test_OK) : false;
}

const FSlateBrush* SNetworkListRow::GetIconImage() const
{
	return FXiaoStyle::Get().GetBrush(NetworkDesc.Pin()->bConnect ? "Icons.link" : "Icons.disconnect");
}

FText SNetworkListRow::GetConnectText() const
{
	return NetworkDesc.Pin()->bConnect ? LOCTEXT("", "") : LOCTEXT("", ""); 
}

static FText NetworkConnectivety2Text(const XiaoNetwork::ENetworkPerformance InType)
{
	switch (InType)
	{
	case XiaoNetwork::Performance_Excellent:	return LOCTEXT("PerformanceExcellent_Text", "极好");
	case XiaoNetwork::Performance_Good:			return LOCTEXT("PerformanceGood_Text", "不错");
	case XiaoNetwork::Performance_Normal:		return LOCTEXT("PerformanceNormal_Text", "正常");
	case XiaoNetwork::Performance_Bad:			return LOCTEXT("PerformanceBad_Text", "较差");
	}
	return FText::GetEmpty();
}

FText SNetworkListRow::GetPerformanceText() const
{
	// TODO 需要验证
	const ENetworkPerformance Performance = GetPerformance(NetworkDesc.Pin()->Performance);
	return NetworkConnectivety2Text(Performance);
}

FSlateColor SNetworkListRow::GetPerformanceColor() const
{
	switch(GetPerformance(NetworkDesc.Pin()->Performance))
	{
	case Performance_Excellent:
	case Performance_Good:
		return FColor::Green;
	case Performance_Normal:
		return FColor::Blue;
	case Performance_Bad:
		return FColor::Red;
	};
	return FColor::Silver;
}

FText SNetworkListRow::GetPerformanceData() const
{
	if(NetworkDesc.IsValid())
	{
		if(const auto Network = NetworkDesc.Pin())
		{
			return FText::FromString(!IsNetworkValid() ? TEXT("-") : FString::Printf(TEXT("%f Mbps (%d%%)"), Network->Performance, static_cast<int>(Network->Performance)));
		}
	}
	return FText::GetEmpty();
}

FText SNetworkListRow::GetReceiveText() const
{
	return FText::FromString(!IsNetworkValid() ? TEXT("-") : FString::Printf(TEXT("%f MB/sec"), NetworkDesc.Pin()->ReceivePerfor));
}

FText SNetworkListRow::GetReceiveDataText() const
{
	if(NetworkDesc.IsValid())
	{
		if(const auto Network = NetworkDesc.Pin())
		{
			return FText::FromString(!IsNetworkValid() ? TEXT("-") : FString::Printf(TEXT("%d Mbps(10%%)"), static_cast<int>(Network->ReceivePerfor)));
		}
	}
	return FText::GetEmpty();
}

FText SNetworkListRow::GetSendText() const
{
	return FText::FromString(!IsNetworkValid() ? TEXT("-") : FString::Printf(TEXT("%f MB/sec"), NetworkDesc.Pin()->SendPerfor));
}

FText SNetworkListRow::GetSendDataText() const
{
	if(NetworkDesc.IsValid())
	{
		if(const auto Network = NetworkDesc.Pin())
		{
			return FText::FromString(!IsNetworkValid() ? TEXT("-") : FString::Printf(TEXT("%d Mbps(10%%)"),  static_cast<int>(Network->SendPerfor)));
		}
	}
	return FText::GetEmpty();
}

FText SNetworkListRow::GetRoundTripTimeText() const
{
	return FText::FromString(!IsNetworkValid() ? TEXT("-") : FString::Printf(TEXT("%d ms"), NetworkDesc.Pin()->RoundTripTime));
}

FText SNetworkListRow::GetEveryTripText() const
{
	if(NetworkDesc.IsValid())
	{
		if(const auto Network = NetworkDesc.Pin())
		{
			return FText::FromString(FString::Printf(TEXT("(%dms, %dms, %dms, %dms)"), Network->Trip1, Network->Trip2, Network->Trip3, Network->Trip4));
		}
	}
	return FText::GetEmpty();
}

#undef LOCTEXT_NAMESPACE
