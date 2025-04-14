/**
  * @author cxx2020@outlook.com
  * @datetime 2023 -10:17 PM
 */

#pragma once

#include "Service/GenericService.h"

class FMacBuildAgentService final : public FGenericService
{
public:
	explicit FMacBuildAgentService(const FServiceCommandLineOptions& InOptions, const FServiceDesc& InServiceDesc);
	virtual ~FMacBuildAgentService() override {};

	// ~ Begin Override FGenericService
	virtual bool OnInstall() override;
	virtual bool OnStart() override;
	virtual bool OnStop() override;
	virtual bool OnDelete() override;
	// ~ End Override FGenericService
};

typedef FMacBuildAgentService FXiaoService;