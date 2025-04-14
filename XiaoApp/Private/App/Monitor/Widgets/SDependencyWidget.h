/**
  * @author cxx2020@outlook.com
  * @datetime 2024 -8:15 PM
 */
#pragma once

#include "CoreMinimal.h"
#ifdef USE_IMGUI
#include "SImGuiBase.h"
#include "DependencyGraph.h"

class SDependencyWidget final : public SImGuiBase
{
public:
	SLATE_BEGIN_ARGS(SDependencyWidget){}
		SLATE_ARGUMENT(TSharedPtr<SViewport>, Viewport)
		SLATE_ARGUMENT(int32, ContextIndex)
	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs);

	virtual FVector2D ComputeDesiredSize(float) const override;

	virtual void OnDraw() override;

public:
	TSharedPtr<XiaoGraph::FGraph> DependencyGraph = nullptr;
	TWeakPtr<SViewport> Viewport = nullptr;
};
#endif
