// Copyright Xiao Studio, Inc. All Rights Reserved.

#include "XiaoAppMain.h"
#include "Mac/MacProgramDelegate.h"
#include "LaunchEngineLoop.h"

int main(int argc, char *argv[])
{
	return [MacProgramDelegate mainWithArgc:argc argv:argv programMain:RunRealMain programExit:FEngineLoop::AppExit];
}
