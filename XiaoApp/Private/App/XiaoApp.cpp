/**
  * @author cxx2020@outlook.com
  * @date 10:49 AM
 */
#include "XiaoApp.h"
#include "Settings/SAgentSettingsWindow.h"
#include "About/SAboutWindow.h"
#include "Installer/SInstallerWindow.h"
#include "XiaoShareField.h"
#include "ShareDefine.h"

#define LOCTEXT_NAMESPACE "XiaoApp"

FXiaoApp::FXiaoApp(FAppParam& InParam)
	: FXiaoAppBase(InParam)
{}

bool FXiaoApp::InitApp()
{
	if (!FXiaoAppBase::InitApp())
	{
		return false;
	}

	TSharedPtr<SWindow> TempWindow = nullptr;
	if (Param.AppName == XiaoAppName::SBuildAbout)
	{
		TempWindow = SNew(SAboutWindow);
	}
	else if (Param.AppName == XiaoAppName::SBuildInstall)
	{
		if (!IsRunAsAdmin())
		{
			const FText Title = LOCTEXT("Warning_Text", "Warning");
			FMessageDialog::Open(EAppMsgType::Type::Ok, LOCTEXT("NotRunAsAdmin_Text", "Must Run As Administrator"), Title);
			return false;
		}
		const auto SharedRefWindow = SNew(SInstallerWindow);
		TempWindow = SharedRefWindow;
	}
	else
	{
		TempWindow = SNew(SAgentSettingsWindow);
	}
	Window = TempWindow;
	if (!TempWindow.IsValid())
	{
		return false;
	}
	FSlateApplication::Get().AddWindow(TempWindow.ToSharedRef(), true);
	return true;
}

#undef LOCTEXT_NAMESPACE