/**
  * @author cxx2020@outlook.com
  * @date 5:57 PM
 */

#include "SMessageWindow.h"
#include "SlateOptMacros.h"
#include "Widgets/Text/STextBlock.h"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SMessageWindow::Construct(const FArguments& InArgs)
{
	SWindow::Construct(SWindow::FArguments()
	.bDragAnywhere(false)
	.CreateTitleBar(true)
	.HasCloseButton(true)
	.SupportsMaximize(false)
	.SupportsMaximize(false)
	.SizingRule(ESizingRule::FixedSize)
	.AutoCenter(EAutoCenter::PreferredWorkArea)
	.Title(InArgs._TiTile)
	.ClientSize(FVector2D(500, 200))
	[
		SNew(STextBlock).Text(InArgs._Message).AutoWrapText(true)
	]);
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION
