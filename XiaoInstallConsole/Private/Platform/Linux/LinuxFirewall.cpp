#include "LinuxFirewall.h"
#include "XiaoLog.h"
#include "XiaoShareField.h"
#include "XiaoInstall.h"
#include "XiaoShare.h"

FLinuxFirewall::~FLinuxFirewall()
{
    Deinitialize();
}

static bool IsFirewallRunning() 
{
	const int status = std::system("systemctl is-active --quiet firewalld");
	return status == 0;
}

bool FLinuxFirewall::Initialize()
{
	XIAO_LOG(Log, TEXT("Firewall Initialize called!"));
	if (!IsFirewallRunning())
	{
		std::system("systemctl start firewalld");
	}

	return IsFirewallRunning();
}

void FLinuxFirewall::Deinitialize()
{
	XIAO_LOG(Log, TEXT("Firewall Deinitialize called!"));
}

bool FLinuxFirewall::BuildFirewall()
{
    if (!Initialize())
    {
		return false;
    }

	// 入栈规则
	AddFwRule(true, true, 1345, 1350);
	AddFwRule(true, true, 37000, 38000);

	// 出站规则
	AddFwRule(false, true, 1345, 1350);
	AddFwRule(false, true, 37000, 38000);

	return true;
}

void FLinuxFirewall::AddFwRule(const bool InInOrOut, const bool bTcpOrUdp, const uint16 InStartPort, const uint16 InEndPort)
{
	const FString Command = FString::Printf(TEXT("sudo iptables -A %s -p %s --dport %u%s -j ACCEPT"), 
		InInOrOut ? TEXT("INPUT") : TEXT("OUTPUT"), 
		bTcpOrUdp ? TEXT("tcp") : TEXT("udp"),
		InStartPort, (InEndPort == 0) ? TEXT("") : *FString::Printf(TEXT(":%u"), InEndPort)
	);
	const std::string StdCommand = TCHAR_TO_UTF8(*Command);
	if (system(StdCommand.c_str()) == 0)
	{
		XIAO_LOG(Log, TEXT("%s Success!"), *Command);
	}
	else 
	{
		XIAO_LOG(Error, TEXT("%s Success!"), *Command);
	}
}
