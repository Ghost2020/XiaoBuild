/**
  * @author cxx2020@outlook.com
  * @date 11:01 PM
 */

#pragma once

#include "CoreMinimal.h"
#include "Fonts/SlateFontInfo.h"

struct FDrawContext;
struct FSlateBrush;

class FTimingTrackViewport;

namespace Xiao
{
	class FDrawHelpers
	{
	public:
		static void DrawBackground(const FDrawContext& DrawContext,
								   const FSlateBrush* BackgroundAreaBrush,
								   const FLinearColor& ValidAreaColor,
								   const FLinearColor& InvalidAreaColor,
								   const FLinearColor& EdgeColor,
								   const float X0,
								   const float X1,
								   const float X2,
								   const float X3,
								   const float Y,
								   const float H,
								   float& OutValidAreaX,
								   float& OutValidAreaW);

		static void DrawBackground(const FDrawContext& DrawContext,
								   const FSlateBrush* BackgroundAreaBrush,
								   const float X0,
								   const float X1,
								   const float X2,
								   const float X3,
								   const float Y,
								   const float H);

		static void DrawBackground(const FDrawContext& DrawContext,
								   const FSlateBrush* BackgroundAreaBrush,
								   const FTimingTrackViewport& Viewport,
								   const float Y,
								   const float H);

		static void DrawBackground(const FDrawContext& DrawContext,
								   const FSlateBrush* BackgroundAreaBrush,
								   const FTimingTrackViewport& Viewport,
								   const float Y,
								   const float H,
								   float& OutValidAreaX,
								   float& OutValidAreaW);
	
		static void DrawTimeRangeSelection(const FDrawContext& DrawContext,
										   const FTimingTrackViewport& Viewport,
										   const double StartTime,
										   const double EndTime,
										   const FSlateBrush* Brush,
										   const FSlateFontInfo& Font);

		static void DrawSelection(const FDrawContext& DrawContext,
								  const float MinX,
								  const float MaxX,
								  float SelectionX1,
								  float SelectionX2,
								  const float Y,
								  const float H,
								  const float TextY,
								  const FString& Text,
								  const FSlateBrush* Brush,
								  const FSlateFontInfo& Font);
	};
}