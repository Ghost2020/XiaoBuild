#pragma once

#include "CoreMinimal.h"
#ifdef IMGUI
#include "SImGuiBase.h"

namespace Xiao
{
	class FImPlot;
}

class SStatsWidget : public SImGuiBase
{
public:
	SLATE_BEGIN_ARGS(SStatsWidget){}
		SLATE_ARGUMENT(FImGuiModuleManager*, ModuleManager)
		SLATE_ARGUMENT(TSharedPtr<SViewport>, Viewport)
	SLATE_END_ARGS()
	
	void Construct(const FArguments& InArgs);

	virtual FVector2D ComputeDesiredSize(float) const override;

	virtual void OnDraw() override;

private:
	TSharedPtr<Xiao::FImPlot> Plot = nullptr;
	TWeakPtr<SViewport> Viewport = nullptr;
};
#endif
