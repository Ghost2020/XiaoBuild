/**
  * @author cxx2020@outlook.com
  * @date 11:02 PM
 */
#include "SHistoryListRow.h"

#include "SlateOptMacros.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Images/SImage.h"
#include "ShareDefine.h"

#include "XiaoStyle.h"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SHistoryListRow::Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InOwnerTableView)
{
	check(InArgs._BuildDesc.IsValid());
	BuildDesc = InArgs._BuildDesc;
	OnItemDoubleClick = InArgs._OnItemDoubleClick;
	
	SMultiColumnTableRow::Construct(FSuperRowType::FArguments().Padding(2.0f)/*.Style(FXiaoStyle::Get(), "TableView.DarkRow")*/, InOwnerTableView);
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION

TSharedRef<SWidget> SHistoryListRow::GenerateWidgetForColumn(const FName& InColumnName)
{
	if(BuildDesc.IsValid())
	{
		if(const auto Build = BuildDesc.Pin())
		{
			if(InColumnName == GHistoryColumnIDStatus)
			{
				return SNew(SHorizontalBox)
					+SHorizontalBox::Slot().HAlign(HAlign_Center).VAlign(VAlign_Center)
					[
						SNew(SImage)
						.Image(FXiaoStyle::Get().GetBrush(Build->BuildStatus == 0 ? "build.ok" : (Build->BuildStatus == -1 ? "build.error" : "build.warning")))
					];
			}
			if(InColumnName == GHistoryColumnIDStartTime)
			{
				const auto Start = Build->StartTime;
				return SNew(SVerticalBox)
				+SVerticalBox::Slot()
				[
					V_CENTER_WIGET(SNew(STextBlock)
						.Text(FText::FromString(FString::Printf(TEXT("%04i-%02i-%02i"), Start.GetYear(), Start.GetMonth(), Start.GetDay()))))
				]
				+SVerticalBox::Slot()
				[
					V_CENTER_WIGET(SNew(STextBlock)
						.Text(FText::FromString(FString::Printf(TEXT("%02i:%02i:%02i"), Start.GetHour(), Start.GetMinute(), Start.GetSecond()))))
				];
			}
			if(InColumnName == GHistoryColumnIDDuration)
			{
				return V_CENTER_WIGET(SNew(STextBlock)
					.Text(FText::FromString(Build->Duration.ToString())));
			}
			if(InColumnName == GHistoryColumnIDMessage)
			{
				return SNew(SVerticalBox)
				+SVerticalBox::Slot()
				[
					V_CENTER_WIGET(SNew(STextBlock)
						.Text(FText::FromString(FString::FromInt(Build->ErrorNum) + TEXT(" error(s)"))))
				]
				+SVerticalBox::Slot()
				[
					V_CENTER_WIGET(SNew(STextBlock)
						.Text(FText::FromString(FString::FromInt(Build->WarningNum) + TEXT(" warning(s)"))))
				];
			}
			if(InColumnName == GHistoryColumnIDAutoRecovery)
			{
				return V_CENTER_WIGET(SNew(STextBlock)
					.Text(FText::FromString(FString::FromInt(Build->AutoRecoveryNum) + TEXT(" message(s)"))));
			}
			if (InColumnName == GHistoryColumnIDVersion)
			{
				return V_CENTER_WIGET(SNew(STextBlock)
					.Text(FText::FromString(Build->Version)));
			}
			if(InColumnName == GHistoryColumnIDType)
			{
				return V_CENTER_WIGET(SNew(STextBlock)
					.Text(BuildType2Text(Build->Type)));
			}
			if(InColumnName == GHistoryColumnIDFilePath)
			{
				return SNew(STextBlock)
					.Text(FText::FromString(Build->FilePath)).AutoWrapText(true);
			}
		}
	}
	return SNullWidget::NullWidget;
}

FReply SHistoryListRow::OnMouseButtonDoubleClick(const FGeometry& InMyGeometry, const FPointerEvent& InMouseEvent)
{
	if (BuildDesc.IsValid())
	{
		return OnItemDoubleClick.ExecuteIfBound(BuildDesc) ? FReply::Handled() : FReply::Unhandled();
	}
	return FReply::Unhandled();
}
