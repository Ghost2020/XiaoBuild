/**
  * @author cxx2020@outlook.com
  * @date 10:49 AM
 */

#pragma once

#include "CoreMinimal.h"
#include "XiaoAppBase.h"

class FXiaoApp final : public FXiaoAppBase
{
public:
	explicit FXiaoApp(FAppParam &InParam);

	virtual bool InitApp() override;
};
