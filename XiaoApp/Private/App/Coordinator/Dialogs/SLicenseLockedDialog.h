/**
  * @author cxx2020@outlook.com
  * @date 5:57 PM
 */
#pragma once

#include "CoreMinimal.h"
#include "Widgets/SWindow.h"

class SLicenseLockedDialog final : public SWindow
{
public:
	SLATE_BEGIN_ARGS(SLicenseLockedDialog)
	{}
	SLATE_END_ARGS()
	
	void Construct(const FArguments& InArgs);
};
