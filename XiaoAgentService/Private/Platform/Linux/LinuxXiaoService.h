/**
  * @author cxx2020@outlook.com
  * @datetime 2023 -10:21 PM
 */

#pragma once

#include "Service/GenericService.h"

class FLinuxBuildAgentService final : public FGenericService
{
public:
	explicit FLinuxBuildAgentService(const FServiceCommandLineOptions& InOptions, const FServiceDesc& InServiceDesc);
	virtual ~FLinuxBuildAgentService() {};

	// ~ Begin Override FGenericService
	virtual bool OnInstall() override;
	virtual bool OnStart() override;
	virtual bool OnEnable() override;
	virtual bool OnDisable() override;
	virtual bool OnStop() override;
	virtual bool OnDelete() override;
	// ~ End Override FGenericService
};

typedef FLinuxBuildAgentService FXiaoService;
