// Copyright Xiao Studio, Inc. All Rights Reserved.

#include "TrayMain.h"
#include "UnixCommonStartup.h"

int main(int argc, char* argv[])
{
	return CommonUnixMain(argc, argv, &RunTrayApp);
}