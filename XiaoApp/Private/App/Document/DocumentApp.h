/**
  * @author cxx2020@outlook.com
  * @datetime 2024 -7:19 PM
 */

#pragma once

#include "CoreMinimal.h"
#include "XiaoAppBase.h"

class FDocumentApp final : public FXiaoAppBase
{
public:
	explicit FDocumentApp(FAppParam& InParam);

	virtual bool InitApp() override;
};
