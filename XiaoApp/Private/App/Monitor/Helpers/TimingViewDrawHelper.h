/**
  * @author cxx2020@outlook.com
  * @date 12:25 AM
 */

#pragma once
#include "Insights/ViewModels/ITimingEvent.h"
#include "Insights/ViewModels/ITimingViewDrawHelper.h"
#include "Insights/ViewModels/TimingEventsTrack.h"

struct FDrawContext;

namespace Xiao
{
	struct FTimingEventsTrackDrawState
	{
		struct FBoxPrimitive
		{
			int32 Depth;
			float X;
			float W;
			FLinearColor Color;
		};

		struct FTextPrimitive
		{
			int32 Depth;
			float X;
			FString Text;
			bool bWhite;
			FLinearColor Color;
		};

		FTimingEventsTrackDrawState()
			: Boxes()
			, InsideBoxes()
			, Borders()
			, Texts()
			, NumLanes(0)
			, NumEvents(0)
			, NumMergedBoxes(0)
		{
		}

		void Reset()
		{
			Boxes.Reset();
			InsideBoxes.Reset();
			Borders.Reset();
			Texts.Reset();
			NumLanes = 0;
			NumEvents = 0;
			NumMergedBoxes = 0;
		}

		int32 GetNumLanes() const { return NumLanes; }

		int32 GetNumEvents() const { return NumEvents; }
		int32 GetNumMergedBoxes() const { return NumMergedBoxes; }
		int32 GetTotalNumBoxes() const { return Boxes.Num() + InsideBoxes.Num(); }

		TArray<FBoxPrimitive> Boxes;
		TArray<FBoxPrimitive> InsideBoxes;
		TArray<FBoxPrimitive> Borders;
		TArray<FTextPrimitive> Texts;

		int32 NumLanes;

		// Debug stats.
		int32 NumEvents;
		int32 NumMergedBoxes;
	};

	class FTimingViewDrawHelper final : public ITimingViewDrawHelper
	{
		enum class EDrawLayer : int32
		{
			TrackBackground,
			EventBorder,
			EventFill,
			EventText,
			EventHighlight,
			RelationBackground,
			Relation,
			HeaderBackground,
			HeaderText,

			Count,
		};
		static int32 ToInt32(EDrawLayer Layer) { return static_cast<int32>(Layer); }

	public:
		explicit FTimingViewDrawHelper(const FDrawContext& InDrawContext, const FTimingTrackViewport& InViewport);
		virtual ~FTimingViewDrawHelper();

		/**
		 * Non-copyable
		 */
		FTimingViewDrawHelper(const FTimingViewDrawHelper&) = delete;
		FTimingViewDrawHelper& operator=(const FTimingViewDrawHelper&) = delete;

		// ITimingViewDrawHelper interface
		virtual const FSlateBrush* GetWhiteBrush() const override { return WhiteBrush; }
		virtual const FSlateBrush* GetEventBorderBrush() const override { return EventBorderBrush; }
		virtual const FSlateBrush* GetHoveredEventBorderBrush() const override { return HoveredEventBorderBrush; }
		virtual const FSlateBrush* GetSelectedEventBorderBrush() const override { return SelectedEventBorderBrush; }
		virtual const FSlateFontInfo& GetEventFont() const override { return EventFont; }
		virtual FLinearColor GetEdgeColor() const override { return EdgeColor; }
		virtual FLinearColor GetValidAreaColor() const override { return ValidAreaColor; }
		virtual FLinearColor GetInvalidAreaColor() const override { return InvalidAreaColor; }
		virtual FLinearColor GetTrackNameTextColor(const FBaseTimingTrack& Track) const override;
		virtual int32 GetHeaderBackgroundLayerId() const override { return ReservedLayerId + ToInt32(EDrawLayer::HeaderBackground); }
		virtual int32 GetHeaderTextLayerId() const override { return ReservedLayerId + ToInt32(EDrawLayer::HeaderText); }
		virtual int32 GetRelationLayerId() const override { return ReservedLayerId + ToInt32(EDrawLayer::Relation); };
		virtual int32 GetFirstLayerId() const override { return ReservedLayerId; }
		virtual int32 GetNumLayerIds() const override { return ToInt32(EDrawLayer::Count); }

		const FDrawContext& GetDrawContext() const { return DrawContext; }
		const FTimingTrackViewport& GetViewport() const { return Viewport; }

		void DrawBackground() const;

		void BeginDrawTracks() const;
		void DrawContextSwitchMarkers(const FTimingEventsTrackDrawState& DrawState, float LineY, float LineH, float Opacity, bool bDrawOverlays, bool bDrawVerticalLines) const;

		void DrawMarkers(const FTimingEventsTrackDrawState& DrawState, float LineY, float LineH, float Opacity) const;

		void DrawTrackHeader(const FBaseTimingTrack& Track) const;
		void DrawTrackHeader(const FBaseTimingTrack& Track, const int32 HeaderLayerId, const int32 HeaderTextLayerId) const;

		void DrawGraphDetails(const FBaseTimingTrack& InGraphTrack) const;

		void EndDrawTracks() const;

		void DrawRelations(const TArray<TUniquePtr<ITimingEventRelation>>& Relations, ITimingEventRelation::EDrawFilter Filter) const;

		int32 GetNumEvents() const { return NumEvents; }
		int32 GetNumMergedBoxes() const { return NumMergedBoxes; }
		int32 GetNumDrawBoxes() const { return NumDrawBoxes; }
		int32 GetNumDrawBorders() const { return NumDrawBorders; }
		int32 GetNumDrawTexts() const { return NumDrawTexts; }

	private:
		const FDrawContext& DrawContext;
		const FTimingTrackViewport& Viewport;

		const FSlateBrush* WhiteBrush;
		const FSlateBrush* EventBorderBrush;
		const FSlateBrush* HoveredEventBorderBrush;
		const FSlateBrush* SelectedEventBorderBrush;
		const FSlateBrush* BackgroundAreaBrush;
		const FLinearColor ValidAreaColor;
		const FLinearColor InvalidAreaColor;
		const FLinearColor EdgeColor;
		const FSlateFontInfo EventFont;

		mutable int32 ReservedLayerId;

		mutable float ValidAreaX;
		mutable float ValidAreaW;

		mutable int32 NumEvents;
		mutable int32 NumMergedBoxes;
		mutable int32 NumDrawBoxes;
		mutable int32 NumDrawBorders;
		mutable int32 NumDrawTexts;
	};
}