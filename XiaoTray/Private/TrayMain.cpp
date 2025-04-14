/**
  * @author cxx2020@outlook.com
  * @datetime 2023 -9:32 PM
 */
#include "TrayMain.h"
#include "Tray.h"
#include "Version.h"

IMPLEMENT_APPLICATION(XiaoTray, XB_PRODUCT_NAME);

int RunTrayApp(const TCHAR* Commandline)
{
	FXiaoTray Tray;
	if (Tray.InitApp())
	{
		Tray.RunApp();
		return 0;
	}

	return -1;
}
