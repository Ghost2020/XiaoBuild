/**
  * @author cxx2020@outlook.com
  * @date 10:49 AM
 */

#pragma once

#include "CoreMinimal.h"
#include "../XiaoApp.h"

class FBuildMonitorApp final : public FXiaoAppBase
{
public:
	explicit FBuildMonitorApp(FAppParam& InParam);

	virtual bool InitApp() override;
};
