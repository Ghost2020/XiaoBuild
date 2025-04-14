/**
  * @author cxx2020@outlook.com
  * @datetime 2023 -10:30 PM
 */
#pragma once

class FGenericFirewall
{
public:
	virtual ~FGenericFirewall() {};

	virtual bool Initialize() { return false; }
	virtual void Deinitialize() {}
	virtual bool BuildFirewall() { return false; }
	virtual bool IsRunAsAdmin() { return false; }
};

#if PLATFORM_WINDOWS or PLATFORM_APPLE or PLATFORM_UNIX
#include COMPILED_PLATFORM_HEADER(Firewall.h)
#else
typedef FGenericFirewall FFirewall;
#endif
