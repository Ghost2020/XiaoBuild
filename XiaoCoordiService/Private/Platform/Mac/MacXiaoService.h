/**
  * @author cxx2020@outlook.com
  * @datetime 2023 -9:08 PM
 */

#pragma once

#include "Service/GenericService.h"

class FMacCoordiService final : public FGenericService
{
public:
	explicit FMacCoordiService(const FServiceCommandLineOptions& InOptions, const FServiceDesc& InServiceDesc);
	virtual ~FMacCoordiService() override;

	// ~ Begin Override FGenericService
	virtual bool OnInstall() override;
	virtual bool OnStart() override;
	virtual bool OnStop() override;
	virtual bool OnDelete() override;
	// ~ End Override FGenericService
};

typedef FMacCoordiService FXiaoService;
