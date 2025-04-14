/**
  * @author cxx2020@outlook.com
  * @datetime 2023 -10:30 PM
 */
#pragma once

#include "Containers/UnrealString.h"
#include "../Firewall.h"


class FLinuxFirewall final : public FGenericFirewall
{
public:
    virtual ~FLinuxFirewall() override;

    virtual bool Initialize() override;
    virtual void Deinitialize() override;
    virtual bool BuildFirewall() override;

private:
	void AddFwRule(const bool InInOrOut, const bool bTcpOrUdp, const uint16 InStartPort, const uint16 InEndPort = 0);
};

typedef FLinuxFirewall FFirewall;
