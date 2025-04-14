/**
  * @author cxx2020@outlook.com
  * @date 10:49 AM
 */

#include "XiaoAppMain.h"
#include "ShareDefine.h"
#include "RequiredProgramMainCPPInclude.h"
#include "App/XiaoApp.h"
#include "App/Coordinator/CoordiManagerApp.h"
#include "App/Monitor/BuildMonitorApp.h"
#include "App/Document/DocumentApp.h"
#include "Version.h"
#include "XiaoShareField.h"

IMPLEMENT_APPLICATION(XiaoApp, XB_PRODUCT_NAME);

int RunRealMain(const TCHAR* CommandLine)
{
	FTaskTagScope Scope(ETaskTag::EGameThread);

	GDateTime = FDateTime::Now();

	GEngineLoop.PreInit(CommandLine);

	FXiaoApp::FAppParam Param{ XiaoAppName::SBuildApp, 1, 24, true };
	TUniquePtr<FXiaoAppBase> App = nullptr;
	Param.AppName = XiaoAppName::SBuildMonitor;
	if (FParse::Value(CommandLine, TEXT("-app"), Param.AppName))
	{
		Param.AppName = Param.AppName.RightChop(1);
	}

	if(Param.AppName == XiaoAppName::SBuildCoordiManager)
	{
		App = MakeUnique<FCoordiManagerApp>(Param);
	}
	else if(Param.AppName == XiaoAppName::SXiaoDocument)
	{
		App = MakeUnique<FDocumentApp>(Param);
	}
	else if (Param.AppName == XiaoAppName::SBuildMonitor)
	{
		Param.AppName = CommandLine;
		Param.bSingleton = false;
		App = MakeUnique<FBuildMonitorApp>(Param);
	}
	else
	{
		if (Param.AppName != XiaoAppName::SBuildAgentSettings || Param.AppName != XiaoAppName::SBuildInstall)
		{
			Param.bConnectRedis = false;
		}
		App = MakeUnique<FXiaoApp>(Param);
	}
	
	if (App->InitApp())
	{
		App->RunApp();
		return 0;
	}

	return -1;
}

#undef LOCTEXT_NAMESPACE