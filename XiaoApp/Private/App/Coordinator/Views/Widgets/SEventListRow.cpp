/**
  * @author cxx2020@outlook.com
  * @date 11:02 PM
 */
#include "SEventListRow.h"
#include "SlateOptMacros.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Text/SMultiLineEditableText.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Input/SComboBox.h"

#include "XiaoShare.h"
#include "XiaoStyle.h"
#include "XiaoShareRedis.h"


#define LOCTEXT_NAMESPACE "SEventListRow"


BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SEventRow::Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InOwnerTableView)
{
	check(InArgs._EventDesc.IsValid());
	EventDesc = InArgs._EventDesc;
	
	SMultiColumnTableRow::Construct(FSuperRowType::FArguments().Padding(2.0f), InOwnerTableView);
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION

TSharedRef<SWidget> SEventRow::GenerateWidgetForColumn(const FName& InColumnName)
{
	if(EventDesc.IsValid())
	{
		if(InColumnName == GEventTableColumnIDChannel)
		{
			return V_CENTER_WIGET(SNew(STextBlock).Text(FText::FromString(EventDesc.Pin()->Channel)).Justification(ETextJustify::Type::Center));
		}
		if(InColumnName == GEventTableColumnIDMessage)
		{
			return V_CENTER_WIGET(SNew(SMultiLineEditableText)
				.Text(FText::FromString(EventDesc.Pin()->Message))
				.IsReadOnly(true)
			);
		}
		if (InColumnName == GEventTableColumnIDDesc)
		{
			return V_CENTER_WIGET(SNew(STextBlock).Text(EventDesc.Pin()->Desc).Justification(ETextJustify::Type::Center));
		}
	}
	return SNullWidget::NullWidget;
}

#undef LOCTEXT_NAMESPACE