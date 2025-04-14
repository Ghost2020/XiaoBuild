/**
  * @author cxx2020@outlook.com
  * @date 11:02 PM
 */

#include "SWelcomeView.h"
#include "SlateOptMacros.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Text/STextBlock.h"
#include "SLicenseAgreement.h"

#include "XiaoStyle.h"

#define LOCTEXT_NAMESPACE "SWelcomeView"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SWelcomeView::Construct(const FArguments& InArgs)
{
	ChildSlot
	[
		SNew(SOverlay)
		+ SOverlay::Slot()
		[
			SNew(SImage)
			.Image(FXiaoStyle::Get().GetBrush(TEXT("Welcome")))
		]
		+ SOverlay::Slot().HAlign(HAlign_Center).VAlign(VAlign_Center)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("Welcome_Text", "欢迎使用Xiaobuild安装"))
		]
	];
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION

TWeakPtr<SWizardView> SWelcomeView::GetNext()
{
	if(!AgreementView.IsValid())
	{
		AgreementView = SNew(SLicenseAgreement);
	}
	return AgreementView;
}

bool SWelcomeView::OnCanNext()
{
	return true;
}

bool SWelcomeView::OnCanBack()
{
	return false;
}

#undef LOCTEXT_NAMESPACE
