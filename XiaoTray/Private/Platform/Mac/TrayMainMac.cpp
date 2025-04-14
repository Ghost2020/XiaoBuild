// Copyright Xiao Studio, Inc. All Rights Reserved.

#include "TrayMain.h"

//<key>NSHighResolutionCapable</key>
//<true/>
//<key>LSUIElement</key>
//<string>1</string >
//<key>LSApplicationCategoryType</key>
//<string>public.app-category.games</string>
//<key>LSMultipleInstancesProhibited</key>
//<true/>
//<key>NSLocalNetworkUsageDescription</key>
//<string>本应用需要访问本地网络以连接其他设备</string>
//
int main(int argc, char* argv[])
{
	return RunTrayApp(nullptr);
}