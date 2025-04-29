#include "WindowsFirewall.h"
#include "XiaoLog.h"
#include "XiaoShareField.h"
#include "XiaoInstall.h"
#include "XiaoShare.h"

#include "Windows/AllowWindowsPlatformTypes.h"
#include <netfw.h>
#include <objbase.h>
#include <oleauto.h>
#include <shellapi.h>
#include <wincred.h>
#include <NTSecAPI.h>

#include "Windows/HideWindowsPlatformTypes.h"

#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "oleaut32.lib")
#pragma comment(lib, "credui.lib")
#pragma comment(lib, "Advapi32.lib")
#pragma comment(lib, "Secur32.lib")

FWindowsFirewall::~FWindowsFirewall()
{
    Deinitialize();
}

bool FWindowsFirewall::Initialize()
{
	XIAO_LOG(Log, TEXT("Firewall Initialize Begin!"));

    HRESULT Hr = CoInitialize(nullptr);
    if (FAILED(Hr))
    {
        XIAO_LOG(Error, TEXT("CoInitialize failed: %d"), Hr);
        return false;
    }

    INetFwMgr* FwMgr = nullptr;
    INetFwPolicy* FwPolicy = nullptr;
    Hr = CoCreateInstance(
        __uuidof(NetFwMgr),
        nullptr,
        CLSCTX_INPROC_SERVER,
        __uuidof(INetFwMgr),
        reinterpret_cast<void**>(&FwMgr)
    );
    if (FAILED(Hr))
    {
        XIAO_LOG(Error, TEXT("CoCreateInstance failed:%d"), Hr);
        goto error;
    }
    
    Hr = FwMgr->get_LocalPolicy(&FwPolicy);
    if (FAILED(Hr))
    {
        XIAO_LOG(Error, TEXT("get_LocalPolicy failed:%d"), Hr);
        goto error;
    }
	
    Hr = FwPolicy->get_CurrentProfile(&FwProfile);
    if (FAILED(Hr))
    {
        XIAO_LOG(Error, TEXT("get_CurrentProfile failed"));
        goto error;
    }

    Hr = CoCreateInstance(
        __uuidof(NetFwPolicy2), 
        nullptr, 
        CLSCTX_INPROC_SERVER, 
        __uuidof(INetFwPolicy2), 
        reinterpret_cast<void**>(&FirewallPolicy)
    );
    if (FAILED(Hr))
    {
        XIAO_LOG(Error, TEXT("Failed to get firewall policy instance."));
        return false;
    }

    Hr = FirewallPolicy->get_Rules(&FirewallRules);
    if (FAILED(Hr))
    {
        XIAO_LOG(Error, TEXT("Failed to get Rules."));;
        return false;
    }

error:
    if (FwPolicy != nullptr)
    {
        FwPolicy->Release();
    }
	
    if (FwMgr != nullptr)
    {
        FwMgr->Release();
    }

	XIAO_LOG(Log, TEXT("Firewall Initialize Finish!"));

    return SUCCEEDED(Hr);
}

void FWindowsFirewall::Deinitialize()
{
    if (FirewallRules)
    {
        FirewallRules->Release();
    }

    if (FirewallPolicy)
    {
        FirewallPolicy->Release();
    }

    if (FwProfile)
    {
        FwProfile->Release();
    }

    CoUninitialize();
}

bool FWindowsFirewall::BuildFirewall()
{
    if (!Initialize())
    {
		XIAO_LOG(Log, TEXT("Initialize Failed!"));
		return false;
    }

	static FString TCPIn = TEXT("(Tcp-In)");
	static FString TCpOut = TEXT("(Tcp-Out)");

	FString MiddlePath;
#if PLATFORM_CPU_ARM_FAMILY
	MiddlePath = TEXT("UBAC/arm64/");
#else
	MiddlePath = TEXT("UBAC/x64/");
#endif

	// 代理类型
	if (GInstallSettings.InstallType & CT_Agent)
	{
		// 入栈规则
		AddFwRule(XiaoAppName::SUbaAgent + TCPIn, SXiaoBuild, GetXiaoAppPath(XiaoAppName::SUbaAgent, MiddlePath));
		AddFwRule(XiaoAppName::SXiaoScheduler + TCPIn, SXiaoBuild, GetXiaoAppPath(XiaoAppName::SXiaoScheduler, MiddlePath));
		AddFwRule(XiaoAppName::SBuildPerfService + TCPIn, SXiaoBuild, GetXiaoAppPath(XiaoAppName::SIperfServer));

		// 出站规则
		AddFwRule(XiaoAppName::SBuildApp + TCpOut, SXiaoBuild, GetXiaoAppPath(XiaoAppName::SBuildApp), false);
		AddFwRule(XiaoAppName::SBuildTray + TCpOut, SXiaoBuild, GetXiaoAppPath(XiaoAppName::SBuildTray), false);
		AddFwRule(XiaoAppName::SXiaoScheduler + TCpOut, SXiaoBuild, GetXiaoAppPath(XiaoAppName::SXiaoScheduler, MiddlePath), false);
		AddFwRule(XiaoAppName::SBuildPerfService + TCpOut, SXiaoBuild, GetXiaoAppPath(XiaoAppName::SIperfServer), false);
		// AddFwRule(XiaoAppName::SBuildLicenseService + TCpOut, SXiaoBuild, GetXiaoAppPath(XiaoAppName::SBuildLicenseService, TEXT(""), false));
	}

	// 调度器类型
	if (GInstallSettings.InstallType & CT_Coordinator || GInstallSettings.InstallType & CT_BackCoordi)
	{
		// 入站规则
		AddFwRule(XiaoAppName::SCacheServer + TCPIn, SXiaoBuild, GetXiaoAppPath(XiaoAppName::SCacheServer));

		// 出站规则
		AddFwRule(XiaoAppName::SCacheServer + TCpOut, SXiaoBuild, GetXiaoAppPath(XiaoAppName::SCacheServer), false);
		AddFwRule(XiaoAppName::SBuildPerfService + TCpOut, SXiaoBuild, GetXiaoAppPath(XiaoAppName::SIperfServer), false);
	}

	// 验证服务器
	if (GInstallSettings.InstallType == CT_AgentCoordiVisulizer)
	{
		// 入站规则
		// AddFwRule(XiaoAppName::SBuildLicenseService + TCPIn, SXiaoBuild, GetXiaoAppPath(XiaoAppName::SBuildLicenseService));

		// 出站规则
		AddFwRule(XiaoAppName::SXiaoScheduler + TCpOut, SXiaoBuild, GetXiaoAppPath(XiaoAppName::SXiaoScheduler, MiddlePath));
	}

	return true;
}

/*bool FWindowsFirewall::IsRunAsAdmin()
{
	BOOL BIsRunAsAdmin = false;
	PSID PAdminSid = nullptr;

	SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
	if(AllocateAndInitializeSid(&NtAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &PAdminSid))
	{
		if(!CheckTokenMembership(nullptr, PAdminSid, &BIsRunAsAdmin))
		{
			BIsRunAsAdmin = -1;
		}

		FreeSid(PAdminSid);
	}

	return BIsRunAsAdmin == 1;
}*/

bool FWindowsFirewall::GetCredentials(FString& OutUsername, FString& OutPassword, const FString& InTarget)
{
	wchar_t Username[256] = {};
	DWORD Size;
	GetUserName(Username, &Size);
	
	wchar_t Password[256] = {};
	BOOL Save = -1;

	CREDUI_INFO Info;
	ZeroMemory(&Info, sizeof(Info));
	Info.cbSize = sizeof(Info);
	Info.pszMessageText = L"请进行身份验证";

	const DWORD Result = CredUIPromptForCredentials(
		&Info,
		*InTarget,
		nullptr,
		0,
		Username,
		256,
		Password,
		256,
		&Save,
		CREDUI_FLAGS_DO_NOT_PERSIST | CREDUI_FLAGS_GENERIC_CREDENTIALS
	);

	if(Result == ERROR_SUCCESS)
	{
		OutUsername = Username;
		OutPassword = Password;
		return LoginUser(OutUsername, OutPassword);
	}

	return Result == ERROR_SUCCESS;
}

bool FWindowsFirewall::LoginUser(const FString& InUsername, const FString& InPassword, const uint32& InLoginType)
{
	HANDLE HLas = nullptr;
	NTSTATUS Nts = LsaConnectUntrusted(&HLas);
	if(Nts != 0)
	{
		XIAO_LOG(Error, TEXT("LSA Connection Failed:%d"), Nts);
		return false;
	}

	Nts = LogonUser(
		*InUsername,
		L"",
		*InPassword,
		InLoginType,
		LOGON32_PROVIDER_DEFAULT,
		&HToken
	);
	if(HToken == nullptr || Nts != 1)
	{
		XIAO_LOG(Error, TEXT("LogonUser Failed:%d"), Nts);
	}
	else
	{
		XIAO_LOG(Error, TEXT("LogonUser Successful!"));
	}
	
	LsaClose(HLas);
	return HToken != nullptr;
}

HRESULT FWindowsFirewall::FirewallIsOn(bool& bFwOn) const
{
    VARIANT_BOOL FwEnabled;

    _ASSERT(fwProfile != NULL);

    bFwOn = false;

    const HRESULT Hr = FwProfile->get_FirewallEnabled(&FwEnabled);
    if (FAILED(Hr))
    {
        XIAO_LOG(Error, TEXT("get_FirewallEnabled failed:%d"), Hr);
        goto error;
    }
	
    if (FwEnabled != VARIANT_FALSE)
    {
        bFwOn = true;
        XIAO_LOG(Error, TEXT("The firewall is on.\n"));
    }
    else
    {
        XIAO_LOG(Error, TEXT("The firewall is off.\n"));
    }

error:
    return Hr;
}

HRESULT FWindowsFirewall::FirewallTurnOn() const
{
    bool bFwOn = false;

    _ASSERT(fwProfile != NULL);
	
    HRESULT Hr = FirewallIsOn(bFwOn);
    if (FAILED(Hr))
    {
        XIAO_LOG(Error, TEXT("WindowsFirewallIsOn failed:%d"), Hr);
        goto error;
    }
	
    if (!bFwOn)
    {
        Hr = FwProfile->put_FirewallEnabled(VARIANT_TRUE);
        if (FAILED(Hr))
        {
            XIAO_LOG(Error, TEXT("put_FirewallEnabled failed:%d"), Hr);
            goto error;
        }

        XIAO_LOG(Log, TEXT("The firewall is now on"));
    }

error:
    return Hr;
}

HRESULT FWindowsFirewall::AddFwRule(const FString& InRuleName, const FString& InGroupName, const FString&InApplicationPath, const bool bDirection, const FString& InLocalPort, const FString& InRemotePort) const
{
	check(FirewallRules);

	XIAO_LOG(Log, TEXT("AddFwRule: RuleName::\"%s\" GroupName:\"%s\" LocalPort::%s RemotePort::%s begin!"), *InRuleName, *InGroupName, *InLocalPort, *InRemotePort);

	const auto Description = SysAllocString(*InRuleName);
	INetFwRule* FwRule = nullptr;
	HRESULT Hr = FirewallRules->Item(Description, &FwRule);
	if (FwRule)
	{
		SysFreeString(Description);
		XIAO_LOG(Warning, TEXT("Firewall Rule: \"%s\" Already Exist!."), *InRuleName);
		return Hr;
	}

    INetFwRule* FirewallRule = nullptr;
    Hr = CoCreateInstance(__uuidof(NetFwRule), nullptr, CLSCTX_INPROC_SERVER, __uuidof(INetFwRule), reinterpret_cast<void**>(&FirewallRule));
    if (FAILED(Hr)) 
    {
		SysFreeString(Description);
        XIAO_LOG(Error, TEXT("Failed to create firewall rule instance."));;
        return Hr;
    }

	const auto ApplicationName = SysAllocString(*InApplicationPath);
    const auto GroupName = SysAllocString(*InGroupName);
    const auto LocalPort = SysAllocString(*InLocalPort);
	const auto RemotePortStr = SysAllocString(*InRemotePort);
    const auto RemoteAddress = SysAllocString(L"*");
    FirewallRule->put_Name(Description);
    FirewallRule->put_Description(Description);
	FirewallRule->put_ApplicationName(ApplicationName);
    FirewallRule->put_Grouping(GroupName);
    FirewallRule->put_Action(NET_FW_ACTION_ALLOW);
    FirewallRule->put_Direction(bDirection ? NET_FW_RULE_DIR_IN : NET_FW_RULE_DIR_OUT);
    FirewallRule->put_Enabled(VARIANT_TRUE);
    FirewallRule->put_Protocol(NET_FW_IP_PROTOCOL_TCP);
    FirewallRule->put_LocalPorts(LocalPort);
	FirewallRule->put_RemotePorts(RemotePortStr);
    FirewallRule->put_RemoteAddresses(RemoteAddress);
    Hr = FirewallRules->Add(FirewallRule);
    if (FAILED(Hr))
    {
        XIAO_LOG(Error, TEXT("Failed to add firewall rule."));;
    }
    else
    {
        XIAO_LOG(Log, TEXT("Firewall rule added successfully."));
    }

	XIAO_LOG(Log, TEXT("AddFwRule: RuleName::\"%s\"!"), *InRuleName);

    FirewallRule->Release();
	SysFreeString(Description);
	SysFreeString(ApplicationName);
	SysFreeString(GroupName);
	SysFreeString(LocalPort);
	SysFreeString(RemotePortStr);
	SysFreeString(RemoteAddress);
    return Hr;
}