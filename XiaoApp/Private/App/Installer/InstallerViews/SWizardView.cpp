/**
  * @author cxx2020@outlook.com
  * @date 11:02 PM
 */
#include "SWizardView.h"
#include "SlateOptMacros.h"
#include "Dialog/SMessageDialog.h"
#include "XiaoAppBase.h"
#include "../SInstallerWindow.h"

#define LOCTEXT_NAMESPACE "SWizardView"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SWizardView::Construct(const FArguments& InArgs)
{
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION

FText SWizardView::GetNextTitle()
{
	static const FText NextTitle = LOCTEXT("NextTitle_Text", "继续");
	return NextTitle;
}

void SWizardView::OnExit()
{
	if (auto Window = StaticCastSharedPtr<SInstallerWindow>(FXiaoAppBase::GApp->GetMainWindow().Pin()))
	{
		Window->SetLockedState(true);
		FXiaoAppBase::GApp->AddNextTickTask(FSimpleDelegate::CreateLambda([]()
		{
			const TSharedRef<SMessageDialog> Dialog = SNew(SMessageDialog)
			.Title(LOCTEXT("ConfirmTitle", "警告"))
			.Icon(FAppStyle::Get().GetBrush("Icons.WarningWithColor.Large"))
			.Message(LOCTEXT("AreYouSure_Text", "确定需要中途退出吗?"))
			.UseScrollBox(false)
			.AutoCloseOnButtonPress(true)
			.Buttons(
			{
				SMessageDialog::FButton(LOCTEXT("ExitButton", "退出"))
				.SetOnClicked(FSimpleDelegate::CreateLambda([]()
				{
					RequestEngineExit(TEXT("RequestExit"));
				})),
				SMessageDialog::FButton(LOCTEXT("CancelButton", "取消")).SetPrimary(true).SetFocus()
			});
			Dialog->GetOnWindowClosedEvent().AddLambda([](const TSharedRef<SWindow>&)
			{
				if (auto Window = StaticCastSharedPtr<SInstallerWindow>(FXiaoAppBase::GApp->GetMainWindow().Pin()))
				{
					Window->SetLockedState(false);
				}
			});
			Dialog->ShowModal();
		}));
	}
}

#undef LOCTEXT_NAMESPACE
