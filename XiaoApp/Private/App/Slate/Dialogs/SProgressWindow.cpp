/**
  * @author cxx2020@outlook.com
  * @date 5:57 PM
 */

#include "SProgressWindow.h"
#include "SlateOptMacros.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Notifications/SProgressBar.h"
#include "XiaoStyle.h"
#include "XiaoShare.h"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SProgressWindow::Construct(const FArguments& InArgs)
{
	AllAmount = InArgs._AllAmount;

	SWindow::Construct(SWindow::FArguments()
	.bDragAnywhere(false)
	.CreateTitleBar(false)
	.HasCloseButton(false)
	.SupportsMaximize(false)
	.SupportsMaximize(false)
	.SizingRule(ESizingRule::FixedSize)
	.AutoCenter(EAutoCenter::PreferredWorkArea)
	.ClientSize(FVector2D(400, 50))
	[
		SNew(SVerticalBox)

		+SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(10.0f, 10.0f)
		[
			SAssignNew(Text, STextBlock).TextStyle(&XiaoH2TextStyle).Text(InArgs._TiTile)
		]

		+SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Fill).VAlign(VAlign_Center).Padding(10.0f, 10.0f)
		[
			SAssignNew(ProgressBar, SProgressBar)
		]
	]);
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SProgressWindow::EnterProgressFrame(const float InExpectedWorkThisFrame, const FText& InText)
{
	Amount += InExpectedWorkThisFrame;
	Text->SetText(FText::FromString(InText.ToString() + FString::Printf(TEXT(" [%.f/%.f]"), Amount, AllAmount)));
	ProgressBar->SetPercent(Amount / AllAmount);
	FSlateApplication::Get().Tick();
}
