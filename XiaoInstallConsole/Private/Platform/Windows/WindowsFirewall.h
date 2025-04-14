/**
  * @author cxx2020@outlook.com
  * @datetime 2023 -10:30 PM
 */
#pragma once

#include "Containers/UnrealString.h"
#include "../Firewall.h"


class FWindowsFirewall final : public FGenericFirewall
{
public:
    virtual ~FWindowsFirewall() override;

    virtual bool Initialize() override;
    virtual void Deinitialize() override;
    virtual bool BuildFirewall() override;

	bool GetCredentials(FString& OutUsername, FString& OutPassword, const FString& InTarget = L"");
	bool LoginUser(const FString& InUsername, const FString& InPassword, const uint32& InLoginType = 2);

private:
    HRESULT FirewallIsOn(bool& bFwOn) const;
    HRESULT FirewallTurnOn() const;
    HRESULT AddFwRule(const FString& InRuleName, const FString& InGroupName, const FString& InApplicationPath, const bool bDirection = true, const FString& InLocalPort=TEXT("Any"), const FString& InRemotePort=TEXT("Any")) const;

private:
	void* HToken = nullptr;
    struct INetFwProfile* FwProfile = nullptr;
    struct INetFwPolicy2* FirewallPolicy = nullptr;
    struct INetFwRules* FirewallRules = nullptr;
};

typedef FWindowsFirewall FFirewall;
