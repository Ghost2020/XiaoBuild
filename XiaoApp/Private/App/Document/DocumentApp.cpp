/**
  * @author cxx2020@outlook.com
  * @date 10:49 AM
 */
#include "DocumentApp.h"
#include "SDocumentWindow.h"

FDocumentApp::FDocumentApp(FAppParam& InParam)
	: FXiaoAppBase(InParam)
{
	
}

bool FDocumentApp::InitApp()
{
	if (FXiaoAppBase::InitApp())
	{
		const auto AppWindow = SNew(SDocumentWindow);
		Window = AppWindow;
		FSlateApplication::Get().AddWindow(AppWindow, true);
		return true;
	}
	return false;
}
