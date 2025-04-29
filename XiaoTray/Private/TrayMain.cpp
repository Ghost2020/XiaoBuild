/**
  * @author cxx2020@outlook.com
  * @datetime 2023 -9:32 PM
 */
#include "TrayMain.h"
#include "Tray.h"

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
