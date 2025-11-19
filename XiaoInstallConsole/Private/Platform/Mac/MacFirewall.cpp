#include "MacFirewall.h"
#include "XiaoLog.h"
#include "XiaoShareField.h"
#include "XiaoInstall.h"
#include "XiaoShare.h"

FMacFirewall::~FMacFirewall()
{
    Deinitialize();
}

static bool IsFirewallEnabled() 
{
	const int status = std::system("/usr/bin/defaults read /Library/Preferences/com.apple.alf globalstate | grep -q '[12]'");
	return status == 0;
}

bool FMacFirewall::Initialize()
{
	XIAO_LOG(Log, TEXT("Firewall Initialize called!"));
	if (!IsFirewallEnabled())
	{
		const int Ret = std::system("sudo /usr/libexec/ApplicationFirewall/socketfilterfw --setglobalstate on");
		return Ret == 0;
	}

	return true;
}

void FMacFirewall::Deinitialize()
{
   XIAO_LOG(Log, TEXT("Firewall Deinitialize called!"));
}

bool FMacFirewall::BuildFirewall()
{
    if (!Initialize())
    {
		XIAO_LOG(Log, TEXT("Initialize Failed!"));
		return false;
    }

	static FString TCPIn = TEXT("(Tcp-In)");
	static FString TCpOut = TEXT("(Tcp-Out)");

	// 代理类型
	if (GInstallSettings.InstallType & CT_Agent)
	{
		AddAppToFirewall(GetXiaoAppPath(XiaoAppName::SBuildApp));
		AddAppToFirewall(GetXiaoAppPath(XiaoAppName::SBuildTray));
		AddAppToFirewall(GetXiaoAppPath(XiaoAppName::SIperfServer));
		AddAppToFirewall(GetXiaoAppPath(XiaoAppName::SUbaAgent, SMiddlePath));
		AddAppToFirewall(GetXiaoAppPath(XiaoAppName::SXiaoScheduler, SMiddlePath));
	}

	// 调度器类型
	if (GInstallSettings.InstallType & CT_Coordinator || GInstallSettings.InstallType & CT_BackCoordi)
	{
		AddAppToFirewall(GetXiaoAppPath(XiaoAppName::SCacheServer));
		AddAppToFirewall(GetXiaoAppPath(XiaoAppName::SIperfServer));
	}

	return true;
}

void FMacFirewall::AddAppToFirewall(const FString& InAppPath)
{
	const FString Command = TEXT("sudo /usr/libexec/ApplicationFirewall/socketfilterfw --add ") + InAppPath;
	const std::string StdCmd = TCHAR_TO_UTF8(*Command);
	if (system(StdCmd.c_str()) == 0)
	{
		XIAO_LOG(Log, TEXT("%s Success!"), *Command);
	}
	else
	{
		XIAO_LOG(Error, TEXT("%s Success!"), *Command);
	}
}
