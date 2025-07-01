/**
  * @author cxx2020@outlook.com
  * @date 10:47 PM
 */

#pragma once

#include "CoreMinimal.h"
#include "Runtime/Launch/Resources/Version.h"
#include "../Tracks/FMarkersTimingTrack.h"
#include "Widgets/SCompoundWidget.h"
#include "Insights/ITimingViewSession.h"
#include "Insights/ViewModels/TimingTrackViewport.h"
#include "Insights/ViewModels/TooltipDrawState.h"
#include "Common/FixedCircularBuffer.h"
#include "../Tracks/UbaTraceReader.h"

namespace Xiao
{
	class FTimeMarker;
	class FTimingGraphTrack;
}

class FJsonObject;
class FXiaoTimeRulerTrack;
class FXiaoMarkersTimingTrack;
class FXiaoTimingGraphTrack;
class FJsonValue;

DECLARE_DELEGATE_OneParam(FActionEventClicked, const FName&)

class SBuildProgressView final : public SCompoundWidget, public UE::Insights::Timing::ITimingViewSession
{
	friend class SMonitorWindow;
public:
	SLATE_BEGIN_ARGS(SBuildProgressView){}
		SLATE_EVENT(FActionEventClicked, OnActionEventClicked);
		SLATE_ARGUMENT(bool*, bRealtime);
	SLATE_END_ARGS()
	
	SBuildProgressView();

	/** Virtual destructor. */
	virtual ~SBuildProgressView() override;

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs);

	double LoadFromTraceView(Xiao::FTraceView& OutView);
	double AddGraphFromSession(const Xiao::FTraceView::FSession& InSession, const float InBaseLineY, const int32 InOrder, const FString& InGraphName, const FString& UniqueName);
	void AddDetailsFromSession(Xiao::FTraceView& InView, const Xiao::FTraceView::FSession& InSession, const float InBaseLineY, const int32 InOrder, const FString& UniqueName, const bool bIsRemote);
	void AddTrackFromProcessor(Xiao::FTraceView& InView, const Xiao::FTraceView::FProcessor& InProcessor, const float InBaseLineY, const int32 InOrder, const FString& InTrackName);
	void UpdateView(Xiao::FTraceView& InView);

	// ~SCompoundWidget Begin
	FORCEINLINE virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;
	FORCEINLINE virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;
	virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseButtonDoubleClick(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual void OnMouseEnter(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual void OnMouseLeave(const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseWheel(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FCursorReply OnCursorQuery(const FGeometry& MyGeometry, const FPointerEvent& CursorEvent) const override;
	virtual bool SupportsKeyboardFocus() const override { return true; }
	virtual FReply OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) override;
	virtual FReply OnKeyUp(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) override;
	// ~SCompoundWidget End

	// ITimingViewSession interface Begin
	virtual const FName& GetName() const override 
	{
		static const FName SName(TEXT("BuildProgress"));
		return SName;
	}
	virtual void AddTopDockedTrack(const TSharedPtr<FBaseTimingTrack> Track) override { AddTrack(Track, ETimingTrackLocation::TopDocked); }
	virtual bool RemoveTopDockedTrack(const TSharedPtr<FBaseTimingTrack> Track) override { return RemoveTrack(Track); }
	
	virtual void AddBottomDockedTrack(const TSharedPtr<FBaseTimingTrack> Track) override { AddTrack(Track, ETimingTrackLocation::BottomDocked); }
	virtual bool RemoveBottomDockedTrack(const TSharedPtr<FBaseTimingTrack> Track) override { return RemoveTrack(Track); }

	virtual void AddScrollableTrack(const TSharedPtr<FBaseTimingTrack> Track) override { AddTrack(Track, ETimingTrackLocation::Scrollable); }
	virtual bool RemoveScrollableTrack(const TSharedPtr<FBaseTimingTrack> Track) override { return RemoveTrack(Track); }
	virtual void InvalidateScrollableTracksOrder() override;

	virtual void AddForegroundTrack(const TSharedPtr<FBaseTimingTrack> Track) override { AddTrack(Track, ETimingTrackLocation::Foreground); }
	virtual bool RemoveForegroundTrack(const TSharedPtr<FBaseTimingTrack> Track) override { return RemoveTrack(Track); }

	virtual void AddTrack(TSharedPtr<FBaseTimingTrack> Track, ETimingTrackLocation Location) override;
	virtual bool RemoveTrack(TSharedPtr<FBaseTimingTrack> Track) override;

	virtual TSharedPtr<FBaseTimingTrack> FindTrack(uint64 InTrackId) override;

#if (ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >=6)
	virtual void EnumerateTracks(TFunctionRef<void(TSharedPtr<FBaseTimingTrack> Track)> Callback) override;
#endif

	virtual double GetTimeMarker() const override;
	virtual void SetTimeMarker(double InTimeMarker) override;
	virtual void SetAndCenterOnTimeMarker(double InTimeMarker) override;

	virtual UE::Insights::Timing::FSelectionChangedDelegate& OnSelectionChanged() override { return OnSelectionChangedDelegate; }
	virtual UE::Insights::Timing::FTimeMarkerChangedDelegate& OnTimeMarkerChanged() override { return OnTimeMarkerChangedDelegate; }
	virtual UE::Insights::Timing::FCustomTimeMarkerChangedDelegate& OnCustomTimeMarkerChanged() override { return OnCustomTimeMarkerChangedDelegate; }
	virtual UE::Insights::Timing::FHoveredTrackChangedDelegate& OnHoveredTrackChanged() override { return OnHoveredTrackChangedDelegate; }
	virtual UE::Insights::Timing::FHoveredEventChangedDelegate& OnHoveredEventChanged() override { return OnHoveredEventChangedDelegate; }
	virtual UE::Insights::Timing::FSelectedTrackChangedDelegate& OnSelectedTrackChanged() override { return OnSelectedTrackChangedDelegate; }
	virtual UE::Insights::Timing::FSelectedEventChangedDelegate& OnSelectedEventChanged() override { return OnSelectedEventChangedDelegate; }
	virtual UE::Insights::Timing::FTrackVisibilityChangedDelegate& OnTrackVisibilityChanged() override;
	virtual UE::Insights::Timing::FTrackAddedDelegate& OnTrackAdded() override;
	virtual UE::Insights::Timing::FTrackRemovedDelegate& OnTrackRemoved() override;

	virtual void ResetSelectedEvent() override;
	virtual void ResetEventFilter() override {  }
	virtual void PreventThrottling() override;
	virtual void AddOverlayWidget(const TSharedRef<SWidget>& InWidget) override;
	// ITimingViewSession interface End

	const TArray<TSharedPtr<FBaseTimingTrack>>& GetTrackList(const ETimingTrackLocation TrackLocation) const
	{
		static const TArray<TSharedPtr<FBaseTimingTrack>> EmptyTrackList;
		switch (TrackLocation)
		{
			case ETimingTrackLocation::TopDocked:    return TopDockedTracks;
			case ETimingTrackLocation::Scrollable:	 return ScrollableTracks;
			default:                                 return EmptyTrackList;
		}
	}

	void SelectHoveredTimingTrack();

	void SelectTimingTrack(const TSharedPtr<FBaseTimingTrack> InTrack, bool bBringTrackIntoView);

	void BringScrollableTrackIntoView(const FBaseTimingTrack& InTrack);
	void BringIntoViewY(float InTopScrollY, float InBottomScrollY);
	void SelectTimeInterval(double IntervalStartTime, double IntervalDuration);
	void SelectToTimeMarker(double InTimeMarker);

	TSharedPtr<const ITimingEvent> GetHoveredEvent() const { return HoveredEvent; }

	static const TCHAR* GetLocationName(ETimingTrackLocation Location);
	
	const FTimingTrackViewport& GetViewport() const  { return Viewport; }
	FTimingTrackViewport& GetViewport() { return Viewport; }
	const FVector2D& GetMousePosition() const { return MousePosition; }

	void RaiseSelectionChanging() const;
	void RaiseSelectionChanged() const;
	
	void UpdateHoveredTimingEvent(float InMousePosX, float InMousePosY);

	void OnSelectedTimingEventChanged() const;

	void SelectLeftTimingEvent();
	void SelectRightTimingEvent();
	void SelectUpTimingEvent();
	void SelectDownTimingEvent();

	void RaiseTimeMarkerChanging(TSharedRef<Xiao::FTimeMarker> InTimeMarker) const;
	void RaiseTimeMarkerChanged(TSharedRef<Xiao::FTimeMarker> InTimeMarker) const;

	const TSharedPtr<FBaseTimingTrack> GetTrackAt(float InPosX, float InPosY) const;

	const TArray<TUniquePtr<ITimingEventRelation>>& GetCurrentRelations() const { return CurrentRelations; }

	bool bShowSessionGraph = true;
	bool bShowProcessorTrack = true;
	bool bShowSessionDetails = false;
	bool bShowScrollable = true;

protected:
	void UpdateVerticalScrollBar() const;
	void ScrollAtPosY(float ScrollPosY);
	void ScrollAtTime(double StartTime);
	void SetTrackPosY(const TSharedPtr<FBaseTimingTrack>& TrackPtr, float TrackPosY) const;

	void BringIntoView(double StartTime, double EndTime);

	FReply AllowTracksToProcessOnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent);
	FReply AllowTracksToProcessOnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent);
	FReply AllowTracksToProcessOnMouseButtonDoubleClick(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent);

	void ShowContextMenu(const FPointerEvent& MouseEvent);

	////////////////////////////////////////////////////////////////////////////////////////////////////

	void UpdatePositionForScrollableTracks();

	double EnforceHorizontalScrollLimits(const double InStartTime);
	float EnforceVerticalScrollLimits(const float InScrollPosY);

	void OnUserScrolled_Horizontal(const float InScrollOffset);
	void OnUserScrolled_Vertical(const float InScrollOffset);

	void UpdateHorizontalScrollBar() const;

	void Jump2NextErrorOrWarning();

protected:
	bool *bRealtime = nullptr;
	uint64 LastTimeCycles = 0;
	double SessionTime = 0.0f;

	FTimingTrackViewport Viewport;

	TMap<uint64, TSharedPtr<FBaseTimingTrack>> AllTracks;
	TArray<TSharedPtr<FBaseTimingTrack>> TopDockedTracks;
	TArray<TSharedPtr<FBaseTimingTrack>> ScrollableTracks;

	bool bScrollableTracksOrderIsDirty = false;

	TSharedRef<FXiaoTimeRulerTrack> TimeRulerTrack;
	
	// SessionGraph
	mutable TMap<FString, class FXiaoTimingGraphTrack*> GraphMap;
	// SessionDetails
	mutable TMap<FString, class FSessionDetailsTrack*> DetailsMap;
	// ProcessorTracks
	mutable TMap<FString, class FProcessTimingTrack*> ProcessMap;
	
	uint32 ErrorIndex = 0;

	FTooltipDrawState Tooltip;

	TSharedPtr<class SOverlay> ExtensionOverlay = nullptr;
	TSharedPtr<SScrollBar> HorizontalScrollBar = nullptr;
	TSharedPtr<SScrollBar> VerticalScrollBar = nullptr;

	enum class ESelectionType
	{
		None,
		TimeRange,
		TimingEvent
	};
	ESelectionType LastSelectionType = ESelectionType::None;

	bool bPreventThrottling = false;
	
	FGeometry ThisGeometry;

	float OverscrollLeft = 0, OverscrollRight = 0, OverscrollTop = 0, OverscrollBottom = 0;
	float LastScrollPosY = 0;

	bool bIsLMB_Pressed = false;
	bool bIsRMB_Pressed = false;

	bool bIsSpaceBarKeyPressed = false;
	bool bIsDragging = false;

	bool bIsPanning = false;
	bool bAllowPanningOnScreenEdges = false;
	float DPIScaleFactor = 0;

	uint32 EdgeFrameCountX = 0;
	uint32 EdgeFrameCountY = 0;

	bool bShouldDisplayDebugInfo = true;

	enum class EPanningMode : uint8
	{
		None = 0,
		Horizontal = 0x01,
		Vertical = 0x02,
		HorizontalAndVertical = Horizontal | Vertical,
	};
	EPanningMode PanningMode = EPanningMode::None;
	
	bool bIsSelecting = false;
	
	double SelectionStartTime = 0;
	double SelectionEndTime = 0;

	TSharedPtr<FBaseTimingTrack> HoveredTrack = nullptr;
	TSharedPtr<const ITimingEvent> HoveredEvent = nullptr;

	TSharedPtr<FBaseTimingTrack> SelectedTrack = nullptr;
	TSharedPtr<const ITimingEvent> SelectedEvent = nullptr;

	FVector2D MousePosition = FVector2D::ZeroVector;

	FVector2D MousePositionOnButtonDown = FVector2D::ZeroVector;
	double ViewportStartTimeOnButtonDown = 0;
	float ViewportScrollPosYOnButtonDown = 0;

	/** Mouse position during the call on mouse button up. */
	FVector2D MousePositionOnButtonUp = FVector2D::ZeroVector;

	const FSlateBrush* WhiteBrush;
	const FSlateFontInfo MainFont;
	const FSlateBrush* RealtimeBrush;

	bool bDrawTopSeparatorLine = true;
	bool bDrawBottomSeparatorLine = true;

	// Debug stats
	int32 NumUpdatedEvents = 0;
	TFixedCircularBuffer<uint64, 32> PreUpdateTracksDurationHistory;
	TFixedCircularBuffer<uint64, 32> UpdateTracksDurationHistory;
	TFixedCircularBuffer<uint64, 32> PostUpdateTracksDurationHistory;
	TFixedCircularBuffer<uint64, 32> TickDurationHistory;
	mutable TFixedCircularBuffer<uint64, 32> PreDrawTracksDurationHistory;
	mutable TFixedCircularBuffer<uint64, 32> DrawTracksDurationHistory;
	mutable TFixedCircularBuffer<uint64, 32> PostDrawTracksDurationHistory;
	mutable TFixedCircularBuffer<uint64, 32> TotalDrawDurationHistory;
	mutable TFixedCircularBuffer<uint64, 32> OnPaintDeltaTimeHistory;
	mutable uint64 LastOnPaintTime = 0;

	UE::Insights::Timing::FSelectionChangedDelegate OnSelectionChangedDelegate;
	UE::Insights::Timing::FTimeMarkerChangedDelegate OnTimeMarkerChangedDelegate;
	UE::Insights::Timing::FCustomTimeMarkerChangedDelegate OnCustomTimeMarkerChangedDelegate;
	UE::Insights::Timing::FHoveredTrackChangedDelegate OnHoveredTrackChangedDelegate;
	UE::Insights::Timing::FHoveredEventChangedDelegate OnHoveredEventChangedDelegate;
	UE::Insights::Timing::FSelectedTrackChangedDelegate OnSelectedTrackChangedDelegate;
	UE::Insights::Timing::FSelectedEventChangedDelegate OnSelectedEventChangedDelegate;
	UE::Insights::Timing::FTrackVisibilityChangedDelegate OnTrackVisibilityChangedDelegate;
	UE::Insights::Timing::FTrackAddedDelegate OnTrackAddedDelegate;
	UE::Insights::Timing::FTrackRemovedDelegate OnTrackRemovedDelegate;

	FActionEventClicked OnEventClicked;
	
	TArray<TUniquePtr<ITimingEventRelation>> CurrentRelations;
};
