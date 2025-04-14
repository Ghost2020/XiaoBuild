/**
  * @author cxx2020@outlook.com
  * @date 5:57 PM
 */
#pragma once

#include "CoreMinimal.h"
#include "Widgets/SWindow.h"

class XIAOAPP_API SMessageWindow final : public SWindow
{
public:
	SLATE_BEGIN_ARGS(SMessageWindow) {}
		SLATE_ARGUMENT(FText, TiTile)
		SLATE_ARGUMENT(FText, Message)
	SLATE_END_ARGS()
	
	void Construct(const FArguments& InArgs);
};
