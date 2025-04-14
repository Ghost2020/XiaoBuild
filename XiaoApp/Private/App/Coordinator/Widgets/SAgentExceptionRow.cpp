#include "SAgentExceptionRow.h"
#include "Widgets/Input/SMultiLineEditableTextBox.h"
#include "exception.pb.h"
#include "XiaoStyle.h"

#define LOCTEXT_NAMESPACE "SAgentExceptionRow"


BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SAgentExceptionRow::Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InOwnerTableView)
{
	check(InArgs._ExceptionDesc.IsValid());
	ExceptionDesc = InArgs._ExceptionDesc;

	SMultiColumnTableRow::Construct(FSuperRowType::FArguments().Padding(2.0f), InOwnerTableView);
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION

TSharedRef<SWidget> SAgentExceptionRow::GenerateWidgetForColumn(const FName& InColumnName)
{
	if (ExceptionDesc.IsValid())
	{
		if (const auto Desc = ExceptionDesc.Pin())
		{
			if (InColumnName == GExceptTableColumnIDTimestamp)
			{
				return V_CENTER_WIGET(SNew(STextBlock)
					.Text(FText::FromString(UTF8_TO_TCHAR(Desc->timestamp().c_str())))
					.Justification(ETextJustify::Type::Center));
			}
			if (InColumnName == GExceptTableColumnIDIP)
			{
				return V_CENTER_WIGET(SNew(STextBlock)
					.Text(FText::FromString(UTF8_TO_TCHAR(Desc->exechost().c_str())))
					.Justification(ETextJustify::Type::Center));
			}
			if (InColumnName == GExceptTableColumnIDContent)
			{
				const FString Content = FString::Printf(TEXT("BuildId::%s\nExecHost::%s\nApplication::%s\nArguments::%s\nWorkingDir::%s\nDescription::%s\nExitCode::%u\nLogFile::%s"),
					UTF8_TO_TCHAR(Desc->buildid().c_str()),
					UTF8_TO_TCHAR(Desc->exechost().c_str()),
					UTF8_TO_TCHAR(Desc->application().c_str()),
					UTF8_TO_TCHAR(Desc->arguments().c_str()),
					UTF8_TO_TCHAR(Desc->workingdir().c_str()),
					UTF8_TO_TCHAR(Desc->description().c_str()),
					Desc->exitcode(),
					UTF8_TO_TCHAR(Desc->logfile().c_str())
				);
				return V_FILL_WIGET(SNew(SMultiLineEditableTextBox)
					.IsReadOnly(true)
					.AutoWrapText(true)
					.Text(FText::FromString(Content))
					.Justification(ETextJustify::Type::Left));
			}
		}
	}
	return SNullWidget::NullWidget;
}


#undef LOCTEXT_NAMESPACE