/**
  * @author cxx2020@outlook.com
  * @datetime 2023 -9:08 PM
 */

#pragma once

#include "Service/GenericService.h"

class FLinuxCoordiService final : public FGenericService
{
public:
	explicit FLinuxCoordiService(const FServiceCommandLineOptions& InOptions, const FServiceDesc& InServiceDesc);
	virtual ~FLinuxCoordiService() override {};

	// ~ Begin Override FGenericService
	virtual bool OnInstall() override;
	virtual bool OnStart() override;
	virtual bool OnEnable() override;
	virtual bool OnDisable() override;
	virtual bool OnStop() override;
	virtual bool OnDelete() override;
	// ~ End Override FGenericService
};

typedef FLinuxCoordiService FXiaoService;
