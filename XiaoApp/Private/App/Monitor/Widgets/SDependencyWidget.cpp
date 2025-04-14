/**
  * @author cxx2020@outlook.com
  * @datetime 2024 -8:15 PM
 */
#include "SDependencyWidget.h"
#include "SlateOptMacros.h"
#include "HAL/PlatformApplicationMisc.h"
#include "Widgets/SViewport.h"

#ifdef USE_IMGUI
BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SDependencyWidget::Construct(const FArguments& InArgs)
{
	check(InArgs._Viewport);
	
	Viewport = InArgs._Viewport;
	
	SImGuiBase::Construct(
		SImGuiBase::FArguments()
		.Viewport(InArgs._Viewport)
		.ContextIndex(InArgs._ContextIndex)
	);

	DependencyGraph = MakeShared<XiaoGraph::FGraph>();
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION

FVector2D SDependencyWidget::ComputeDesiredSize(float) const
{
	FVector2D Size;
	if(Viewport.IsValid())
	{
		Size = Viewport.Pin()->ComputeDesiredSize(1.0f);
	}
	const auto AbsPos = this->GetCachedGeometry().AbsolutePosition;
	const float DPIScaleFactor = FPlatformApplicationMisc::GetDPIScaleFactorAtPoint(AbsPos.X, AbsPos.Y);
	FVector2D _CanvasSize = Size / DPIScaleFactor;
	_CanvasSize.X -= 95;
	_CanvasSize.Y -= 80;
	DependencyGraph->SetCanvasSize(_CanvasSize);
	if (Size.Length() > 100)
	{
		DependencyGraph->RebuildGraph();
	}
	return Size;
}

void SDependencyWidget::OnDraw()
{
	DependencyGraph->OnDraw();
}

#endif
