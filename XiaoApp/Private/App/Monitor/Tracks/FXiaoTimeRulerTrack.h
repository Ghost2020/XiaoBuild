/**
  * @author cxx2020@outlook.com
  * @date 10:47 PM
 */

#pragma once

#include "CoreMinimal.h"
#include "Fonts/SlateFontInfo.h"
#include "Insights/ITimingViewSession.h"
#include "InsightsCore/Common/SimpleRtti.h"
#include "Insights/ViewModels/BaseTimingTrack.h"


namespace Xiao
{
	class FTimeMarker final : public UE::Insights::Timing::ITimeMarker
	{
	public:
		FTimeMarker()
			: Time(0.0)
			, Name(TEXT("T"))
			, Color(FLinearColor(1.0f, 1.0f, 1.0f, 0.5f))
			, bIsVisible(true)
			, bIsHighlighted(false)
			, CrtTextWidth(0.0f)
		{}

		virtual ~FTimeMarker() override {}

		virtual double GetTime() const override { return Time; }
		virtual void SetTime(const double  InTime) override { Time = InTime; }

		const FString& GetName() const { return Name; }
		void SetName(const FString& InName) { Name = InName; }

		const FLinearColor& GetColor() const { return Color; }
		void SetColor(const FLinearColor& InColor) { Color = InColor; }

		bool IsVisible() const { return bIsVisible; }
		void SetVisibility(const bool bOnOff) { bIsVisible = bOnOff; }

		bool IsHighlighted() const { return bIsHighlighted; }
		void SetHighlighted(const bool bOnOff) { bIsHighlighted = bOnOff; }

		float GetCrtTextWidth() const { return CrtTextWidth; }
		void SetCrtTextWidthAnimated(const float InTextWidth) const { CrtTextWidth = CrtTextWidth * 0.6f + InTextWidth * 0.4f; }

	private:
		double Time;
		FString Name;
		FLinearColor Color;
		bool bIsVisible;
		bool bIsHighlighted;

		// Smoothed time marker text width to avoid flickering
		mutable float CrtTextWidth;
	};
}

class FXiaoTimeRulerTrack : public FBaseTimingTrack
{
	INSIGHTS_DECLARE_RTTI(FXiaoTimeRulerTrack, FBaseTimingTrack)

public:
	FXiaoTimeRulerTrack();
	virtual ~FXiaoTimeRulerTrack() override;

	virtual void Reset() override;

	void SetSelection(const bool bInIsSelecting, const double InSelectionStartTime, const double InSelectionEndTime);

	TArray<TSharedRef<Xiao::FTimeMarker>>& GetTimeMarkers() { return TimeMarkers; }
	const TArray<TSharedRef<Xiao::FTimeMarker>>& GetTimeMarkers() const { return TimeMarkers; }
	void AddTimeMarker(TSharedRef<Xiao::FTimeMarker> InTimeMarker);
	void RemoveTimeMarker(TSharedRef<Xiao::FTimeMarker> InTimeMarker);
	void RemoveAllTimeMarkers();

	TSharedPtr<Xiao::FTimeMarker> GetTimeMarkerByName(const FString& InTimeMarkerName);
	TSharedPtr<Xiao::FTimeMarker> GetTimeMarkerAtPos(const FVector2D& InPosition,
	                                                 const FTimingTrackViewport& InViewport);

	bool IsScrubbing() const { return bIsScrubbing; }
	TSharedRef<Xiao::FTimeMarker> GetScrubbingTimeMarker();
	void StartScrubbing(TSharedRef<Xiao::FTimeMarker> InTimeMarker);
	void StopScrubbing();

	virtual void PostUpdate(const ITimingTrackUpdateContext& Context) override;
	virtual void Draw(const ITimingTrackDrawContext& Context) const override;
	virtual void PostDraw(const ITimingTrackDrawContext& Context) const override;

	virtual void BuildContextMenu(FMenuBuilder& MenuBuilder) override;

private:
	void DrawTimeMarker(const ITimingTrackDrawContext& Context, const Xiao::FTimeMarker& TimeMarker) const;
	void ContextMenu_MoveTimeMarker_Execute(TSharedRef<Xiao::FTimeMarker> InTimeMarker) const;

private:
	// Slate resources
	const struct FSlateBrush* WhiteBrush;
	const FSlateFontInfo Font;

	bool bIsSelecting;
	double SelectionStartTime;
	double SelectionEndTime;

	// The last time value at mouse postion. Updated in PostDraw.
	mutable double CrtMousePosTime;

	// The smoothed width of "the text at mouse position" to avoid flickering. Updated in PostDraw.
	mutable float CrtMousePosTextWidth;

	/**
	 * The sorted list of all registered time markers. It defines the draw order of time markers.
	 * The time marker currently scrubbing will be moved at the end of the list in order to be displayed on top of other markers.
	 */
	TArray<TSharedRef<Xiao::FTimeMarker>> TimeMarkers;

	/** True if the user is currently dragging a time marker. */
	bool bIsScrubbing;
};

