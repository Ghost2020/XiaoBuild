/**
  * @author cxx2020@outlook.com
  * @datetime 2023 -10:30 PM
 */
#pragma once

#include "Containers/UnrealString.h"
#include "../Firewall.h"


class FMacFirewall final : public FGenericFirewall
{
public:
    virtual ~FMacFirewall() override;

    virtual bool Initialize() override;
    virtual void Deinitialize() override;
    virtual bool BuildFirewall() override;

private:
	void AddAppToFirewall(const FString& InAppPath);
};

typedef FMacFirewall FFirewall;
