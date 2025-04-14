// /**
//   * @author cxx2020@outlook.com
//   * @date 9:02 PM
//  */

#pragma once

#include "CoreMinimal.h"
#include "Fonts/SlateFontInfo.h"

class ITimingTrackUpdateContext;
class ITimingTrackDrawContext;
class FBaseTimingTrack;
struct FSlateBrush;

namespace Xiao
{
	class FTrackHeader
	{
	public:
		explicit FTrackHeader(FBaseTimingTrack& InParentTrack);
		~FTrackHeader() {}

		void Reset();

		bool IsInBackground() const { return bIsInBackground; }
		void SetIsInBackground(const bool bOnOff) { bIsInBackground = bOnOff; }

		bool CanBeCollapsed() const { return bCanBeCollapsed; }
		void SetCanBeCollapsed(const bool bOnOff) { bCanBeCollapsed = bOnOff; }

		bool IsCollapsed() const { return bIsCollapsed; }
		void SetIsCollapsed(const bool bOnOff) { bIsCollapsed = bOnOff; }
		void ToggleCollapsed() { bIsCollapsed = !bIsCollapsed; }

		void UpdateSize();

		void Update(const ITimingTrackUpdateContext& InContext);
		void PostUpdate(const ITimingTrackUpdateContext& InContext);

		void Draw(const ITimingTrackDrawContext& InContext) const;
		void PostDraw(const ITimingTrackDrawContext& InContext) const;

		float GetFontScale() const { return FontScale; }
		void SetFontScale(const float InFontScale) { FontScale = InFontScale; }

	private:
		void DrawInternal(const ITimingTrackDrawContext& Context, bool bDrawBackgroundLayer) const;

	private:
		FBaseTimingTrack& ParentTrack;

		float Width;
		float Height;

		bool bIsInBackground;

		bool bCanBeCollapsed;
		bool bIsCollapsed;

		float TargetHoveredAnimPercent; // [0.0 .. 1.0], 0.0 = hidden, 1.0 = visible
		float CurrentHoveredAnimPercent;

		// Slate resources
		const FSlateBrush* WhiteBrush;
		const FSlateFontInfo Font;
		float FontScale;
	};
}
