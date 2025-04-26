/**
  * @author cxx2020@outlook.com
  * @date 10:49 AM
 */
#include "BuildMonitorApp.h"
#include "SMonitorWindow.h"
#include "ISourceCodeAccessModule.h"

FBuildMonitorApp::FBuildMonitorApp(FAppParam& InParam)
	: FXiaoAppBase(InParam)
{}

bool FBuildMonitorApp::InitApp()
{
	if (FXiaoAppBase::InitApp())
	{
		if (FModuleManager::Get().ModuleExists(TEXT("MessageLog")))
		{
			FModuleManager::Get().LoadModule("MessageLog");
		}

		if (FModuleManager::Get().ModuleExists(TEXT("TraceInsights")))
		{
			FModuleManager::Get().LoadModule("TraceInsights");
		}

#ifdef USE_IMGUI
		if (FModuleManager::Get().ModuleExists(TEXT("ImGui")))
		{
			FModuleManager::Get().LoadModule("ImGui");
		}
#endif
		
		// Load the source code access module
		ISourceCodeAccessModule& SourceCodeAccessModule = FModuleManager::LoadModuleChecked<ISourceCodeAccessModule>(FName("SourceCodeAccess"));
#if PLATFORM_WINDOWS
		FWindowsPlatformMisc::CoInitialize(ECOMModel::Multithreaded);
		FModuleManager::LoadModuleChecked<IModuleInterface>(FName("VisualStudioSourceCodeAccess"));
		SourceCodeAccessModule.SetAccessor(FName("VisualStudioSourceCodeAccess"));
		FWindowsPlatformMisc::CoUninitialize();
#elif PLATFORM_MAC
		FModuleManager::LoadModuleChecked<IModuleInterface>(FName("XCodeSourceCodeAccess"));
		SourceCodeAccessModule.SetAccessor(FName("XCodeSourceCodeAccess"));
#endif

		const auto AppWindow = SNew(SMonitorWindow);
		Window = AppWindow;
		FSlateApplication::Get().AddWindow(AppWindow, true);
		return true;
	}
	return false;
}
