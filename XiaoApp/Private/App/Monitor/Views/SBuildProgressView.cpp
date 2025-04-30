/**
  * @author cxx2020@outlook.com
  * @date 11:02 PM
 */
#pragma once

#include "SBuildProgressView.h"

#include "HAL/PlatformApplicationMisc.h"
#include "SlateOptMacros.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "InsightsCore/Common/PaintUtils.h"
#include "Insights/ViewModels/TimingEventSearch.h"

#include "Widgets/SOverlay.h"
#include "Widgets/Layout/SScrollBar.h"
#include "Widgets/SBoxPanel.h"

#include "../Tracks/FXiaoTimeRulerTrack.h"
#include "../Tracks/FTimingGraphTrack.h"
#include "../Tracks/FSessionDetailsTrack.h"
#include "../Tracks/FProcessorTimingTrack.h"
#include "../Helpers/DrawHelpers.h"
#include "../Helpers/TimingViewDrawHelper.h"

#include "XiaoShare.h"
#include "XiaoStyle.h"
#include "ShareDefine.h"

#include "../Tracks/UbaTraceReader.h"


#define LOCTEXT_NAMESPACE "SBuildProgressView"

#define ACTIVATE_BENCHMARK 0

SBuildProgressView::SBuildProgressView()
	: bScrollableTracksOrderIsDirty(false)
	, TimeRulerTrack(MakeShared<FXiaoTimeRulerTrack>())
	, WhiteBrush(FXiaoStyle::Get().GetBrush("WhiteBrush"))
	, MainFont(FAppStyle::Get().GetFontStyle("SmallFont"))
	, RealtimeBrush(FXiaoStyle::Get().GetBrush("Monitor.Realtime"))
	, PreUpdateTracksDurationHistory()
	, UpdateTracksDurationHistory()
	, PostUpdateTracksDurationHistory()
	, TickDurationHistory()
	, PreDrawTracksDurationHistory()
	, DrawTracksDurationHistory()
	, PostDrawTracksDurationHistory()
	, TotalDrawDurationHistory()
    , OnPaintDeltaTimeHistory()
{
}

SBuildProgressView::~SBuildProgressView()
{
	
}

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SBuildProgressView::Construct(const FArguments& InArgs)
{
	XIAO_LOG(Log, TEXT("SBuildProgressView::Construct::Begin"));
	GLog->Flush();

	OnEventClicked = InArgs._OnActionEventClicked;
	bRealtime = InArgs._bRealtime;

	FTextBlockStyle NoBuildStyle = XiaoH2TextStyle;
	NoBuildStyle.SetColorAndOpacity(XiaoRed);
		
	ChildSlot
	[
		SNew(SOverlay)
		
		+ SOverlay::Slot()
		.VAlign(VAlign_Bottom)
		.Padding(FMargin(0.0f, 0.0f, 8.0f, 0.0f))
		[
			SAssignNew(HorizontalScrollBar, SScrollBar).Visibility(EVisibility::SelfHitTestInvisible)
			.Orientation(Orient_Horizontal)
			.AlwaysShowScrollbar(false)
			.Visibility(EVisibility::Visible)
			.OnUserScrolled(this, &SBuildProgressView::OnUserScrolled_Horizontal)
		]
	
		+ SOverlay::Slot()
		.HAlign(HAlign_Right)
		.Padding(FMargin(0.0f, 0.0f, 2.0f, 0.0f))
		[
			SAssignNew(VerticalScrollBar, SScrollBar).Visibility(EVisibility::SelfHitTestInvisible)
			.Orientation(Orient_Vertical)
			.AlwaysShowScrollbar(false)
			.Visibility(EVisibility::Visible)
			.OnUserScrolled(this, &SBuildProgressView::OnUserScrolled_Vertical)
		]
	
		+ SOverlay::Slot()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		.Padding(FMargin(0.0f))
		[
			SNew(SHorizontalBox).Visibility_Lambda([this]()
			{
				return AllTracks.Num() > 0 ? EVisibility::Collapsed : EVisibility::Visible;
			})
			+ SHorizontalBox::Slot().HAlign(HAlign_Center).VAlign(VAlign_Center)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight().VAlign(VAlign_Center).HAlign(HAlign_Center).Padding(50.0f, 20.0f, 50.0f, 20.0f)
				[
					SNew(SImage).Image(FXiaoStyle::Get().GetBrush("empty_search"))
				]
				+ SVerticalBox::Slot().AutoHeight().VAlign(VAlign_Center).HAlign(HAlign_Center).Padding(50.0f, 20.0f, 50.0f, 20.0f)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("NoBuildFound_Text", "当前没有追踪数据"))
					.TextStyle(&NoBuildStyle)
				]
			]
		]
	];

	UpdateHorizontalScrollBar();
	UpdateVerticalScrollBar();

	TimeRulerTrack->Reset();
	AddTopDockedTrack(TimeRulerTrack);

	OnOptionsChanged.BindLambda([this](const FXiaoTimingGraphTrack* InGraphTrack)
	{
		if (InGraphTrack)
		{
			TArray<FXiaoTimingGraphTrack*> Values;
			GraphMap.GenerateValueArray(Values);
			for (FXiaoTimingGraphTrack* Graph : Values)
			{
				if (Graph && Graph != InGraphTrack)
				{
					Graph->SetEnabledOptions(InGraphTrack->GetEnabledOptions());
					Graph->SetVisibleOptions(InGraphTrack->GetVisibleOptions());
					Graph->SetDirtyFlag();
				}
			}
		}
		
	});
	OnSeriesVisibilityChanged.BindLambda([this](const FXiaoTimingGraphTrack* InTrack, const FXiaoTimingGraphSeries* InSeries)
	{
		static double SLastTrigger = 0.0f;
		if (InSeries)
		{
			if (FPlatformTime::Seconds() - SLastTrigger > 0.1f)
			{
				SLastTrigger = FPlatformTime::Seconds();
				TArray<FXiaoTimingGraphTrack*> Values;
				GraphMap.GenerateValueArray(Values);
				for (FXiaoTimingGraphTrack* GraphTrack : Values)
				{
					if (GraphTrack && GraphTrack != InTrack)
					{
						auto Series = GraphTrack->GetGraphSeries(InSeries->GraphId).Get();
						if (Series && Series != InSeries)
						{
							Series->SetVisibility(InSeries->IsVisible());
							Series->SetDirtyFlag();
						}
					}
				}
			}
		}
	});

	XIAO_LOG(Log, TEXT("SBuildProgressView::Construct::Finish"));
	GLog->Flush();
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION

double SBuildProgressView::LoadFromTraceView(Xiao::FTraceView& OutView)
{
	static bool SbFirstOne = true;
	XIAO_LOG(Verbose, TEXT("LoadFromJsonObject::Begin."));
	
	GraphMap.Empty();
	DetailsMap.Empty();
	ProcessMap.Empty();

	for (int Index = ScrollableTracks.Num()-1; Index >= 0 ; --Index)
	{
		const auto& Track = ScrollableTracks[Index];
		RemoveTrack(Track);
	}
	AllTracks.Empty();
	
	int32 Order = 2;
	int SessionIndex = 0;
	double RealDuration = 0.0f;
	double BaseLineY = FXiaoTimingGraphTrack::GraphTrackHeight;
	for(const auto& Session : OutView.sessions)
	{
		SessionIndex += 1;
		const double LastTime = AddGraphFromSession(Session, BaseLineY, Order++, Session.fullName, Session.clientUid.GetString());
		RealDuration = RealDuration < LastTime ? LastTime : RealDuration;
		BaseLineY += FXiaoTimingGraphTrack::GraphTrackHeight;

		AddDetailsFromSession(OutView, Session, BaseLineY, Order++, Session.clientUid.GetString(), SessionIndex != 1);
		BaseLineY += FSessionDetailsTrack::DetailsTrackHeight;

		int32 ProcessorIndex = 0;
		for(const auto& processor : Session.processors)
		{
			const FString ProcessorName = FString::Printf(TEXT("%d-%d"), SessionIndex, ++ProcessorIndex);
			AddTrackFromProcessor(OutView, processor, BaseLineY, Order++, ProcessorName);
			BaseLineY += FProcessTimingTrack::ProcessTrackHeight;
			if (processor.processes.Num() > 0)
			{
				const double LastProcessStop = FPlatformTime::ToSeconds64(processor.processes.Last().stop);
				RealDuration = RealDuration < LastProcessStop ? LastProcessStop : RealDuration;
			}
		}
	}

	Viewport.RestrictDuration(0.0f, RealDuration);
	Viewport.SetMaxValidTime(RealDuration);
	if (!SbFirstOne)
	{
		Viewport.ZoomOnTimeInterval(RealDuration * 0.5f, RealDuration);
	}
	SbFirstOne = false;

	XIAO_LOG(Verbose, TEXT("LoadFromJsonObject::Finish."));
	return RealDuration;
}

double SBuildProgressView::AddGraphFromSession(const Xiao::FTraceView::FSession& InSession, const float InBaseLineY, const int32 InOrder, const FString& InGraphName, const FString& UniqueName)
{
	double LastTime;
	const auto GraphTrack = MakeShared<FXiaoTimingGraphTrack>();
	GraphTrack->GetSharedValueViewport().SetBaselineY(InBaseLineY);
	GraphTrack->SetName(InGraphName);
	GraphTrack->SetVisibilityFlag(bShowSessionGraph);
	GraphTrack->SetHeight(FXiaoTimingGraphTrack::GraphTrackHeight);
	GraphTrack->SetOrder(InOrder);
	GraphTrack->LoadFromSession(InSession, GetViewport(), LastTime);
	AddTrack(GraphTrack, ETimingTrackLocation::Scrollable);
	GraphMap.Add(UniqueName, &GraphTrack.Get());
	return LastTime;
}

void SBuildProgressView::AddDetailsFromSession(Xiao::FTraceView& InView, const Xiao::FTraceView::FSession& InSession, const float InBaseLineY, const int32 InOrder, const FString& UniqueName, const bool bIsRemote)
{
	const auto DetailsTrack = MakeShared<FSessionDetailsTrack>();
	DetailsTrack->SetVisibilityFlag(bShowSessionDetails);
	DetailsTrack->SetHeight(FSessionDetailsTrack::DetailsTrackHeight);
	DetailsTrack->SetOrder(InOrder);
	DetailsTrack->LoadFromSession(InView, InSession, bIsRemote);
	AddTrack(DetailsTrack, ETimingTrackLocation::Scrollable);
	DetailsMap.Add(UniqueName, &DetailsTrack.Get());
}

void SBuildProgressView::AddTrackFromProcessor(Xiao::FTraceView& InView, const Xiao::FTraceView::FProcessor& InProcessor, const float InBaseLineY, const int32 InOrder, const FString& InTrackName)
{
	const auto ProcessTrack = MakeShared<FProcessTimingTrack>(InTrackName, InTrackName, &InView, InProcessor.processes);
	ProcessTrack->SetName(InTrackName);
	ProcessTrack->SetVisibilityFlag(bShowProcessorTrack);
	ProcessTrack->SetHeight(FProcessTimingTrack::ProcessTrackHeight);
	ProcessTrack->SetOrder(InOrder);
	ProcessTrack->SetPosY(InBaseLineY);
	AddTrack(ProcessTrack, ETimingTrackLocation::Scrollable);
	ProcessMap.Add(InTrackName, &ProcessTrack.Get());
}

void SBuildProgressView::UpdateView(Xiao::FTraceView& InView)
{
	int32 Order = 2;
	double BaseLineY = FXiaoTimingGraphTrack::GraphTrackHeight;

	int SessionIndex = 0;
	for (const auto& Session : InView.sessions)
	{
		// Update TraceTime
		if (Session.updates.Num() > 0)
		{
			const double LastCyles = Session.updates.Last().time;
			LastTimeCycles = LastTimeCycles < LastCyles ? LastCyles : LastTimeCycles;
		}
		
		const FString UniqueName = Session.clientUid.GetString();
		SessionIndex += 1;
		Order += 1;
		// NewGraph
		if (!GraphMap.Contains(UniqueName))
		{
			AddGraphFromSession(Session, BaseLineY, Order, Session.fullName, UniqueName);
		}
		// UpdateGraph
		else
		{
			auto& Graph = GraphMap[UniqueName];
			Graph->SetVisibilityFlag(bShowSessionGraph);
			Graph->SetOrder(Order);
			Graph->UpdateGraph(Session);
		}
		BaseLineY += FXiaoTimingGraphTrack::GraphTrackHeight;

		// NewDetails
		Order += 1;
		if (!DetailsMap.Contains(UniqueName))
		{
			AddDetailsFromSession(InView, Session, BaseLineY, Order, UniqueName, SessionIndex != 1);
		}
		// UpdateDetails
		else
		{
			auto& Details = DetailsMap[UniqueName];
			Details->SetVisibilityFlag(bShowSessionDetails);
			Details->SetOrder(Order);
			Details->UpdateDetails(InView, Session);
		}
		BaseLineY += FSessionDetailsTrack::DetailsTrackHeight;

		// Processors
		int ProssorIndex = 0;
		for (const auto& Processor : Session.processors)
		{
			ProssorIndex += 1;
			Order += 1;
			const FString TrackName = FString::Printf(TEXT("%d-%d"), SessionIndex, ProssorIndex);
			// NewTrack
			if (!ProcessMap.Contains(TrackName))
			{
				AddTrackFromProcessor(InView, Processor, BaseLineY, Order, TrackName);
			}
			// UpdateTrack
			else
			{
				auto& Process = ProcessMap[TrackName];
				Process->SetVisibilityFlag(bShowProcessorTrack);
				Process->SetOrder(Order);
				Process->UpdateProcessor(Processor.processes, LastTimeCycles);
			}
			BaseLineY += FProcessTimingTrack::ProcessTrackHeight;
		}
	}

	if (LastTimeCycles > 0)
	{
		SessionTime = FPlatformTime::ToSeconds64(LastTimeCycles);
		Viewport.RestrictDuration(0.0f, SessionTime);
		Viewport.SetMaxValidTime(SessionTime);
	}
}

void SBuildProgressView::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	FStopwatch TickStopwatch;
	TickStopwatch.Start();

	LLM_SCOPE_BYTAG(Insights);

	ThisGeometry = AllottedGeometry;

	Tooltip.SetFontScale(AllottedGeometry.Scale);

	bPreventThrottling = false;

	constexpr float OverscrollFadeSpeed = 2.0f;
	if (OverscrollLeft > 0.0f)
	{
		OverscrollLeft = FMath::Max(0.0f, OverscrollLeft - InDeltaTime * OverscrollFadeSpeed);
	}
	if (OverscrollRight > 0.0f)
	{
		OverscrollRight = FMath::Max(0.0f, OverscrollRight - InDeltaTime * OverscrollFadeSpeed);
	}
	if (OverscrollTop > 0.0f)
	{
		OverscrollTop = FMath::Max(0.0f, OverscrollTop - InDeltaTime * OverscrollFadeSpeed);
	}
	if (OverscrollBottom > 0.0f)
	{
		OverscrollBottom = FMath::Max(0.0f, OverscrollBottom - InDeltaTime * OverscrollFadeSpeed);
	}

	const float ViewWidth = static_cast<float>(AllottedGeometry.GetLocalSize().X);
	const float ViewHeight = static_cast<float>(AllottedGeometry.GetLocalSize().Y);

	////////////////////////////////////////////////////////////////////////////////////////////////////
	// Update viewport.

	Viewport.SetPosY(32.0f); // height of toolbar
	Viewport.UpdateSize(FMath::RoundToFloat(ViewWidth), FMath::RoundToFloat(ViewHeight) - Viewport.GetPosY());

	if (!bIsPanning)
	{
		// Elastic snap to horizontal time limits.
		if (Viewport.EnforceHorizontalScrollLimits(0.5)) // 0.5 is the interpolation factor
		{
			UpdateHorizontalScrollBar();
		}
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////
	// Check the analysis session time.
	if (*bRealtime)
	{
		// Check if horizontal scroll area has changed.
		if (SessionTime > Viewport.GetMaxValidTime() &&
			SessionTime != DBL_MAX &&
			SessionTime != std::numeric_limits<double>::infinity())
		{
			const double PreviousSessionTime = Viewport.GetMaxValidTime();
			if ((PreviousSessionTime >= Viewport.GetStartTime() && PreviousSessionTime <= Viewport.GetEndTime()) ||
				(SessionTime >= Viewport.GetStartTime() && SessionTime <= Viewport.GetEndTime()))
			{
				Viewport.AddDirtyFlags(ETimingTrackViewportDirtyFlags::HClippedSessionTimeChanged);
			}

			UpdateHorizontalScrollBar();
		}
		Viewport.SetMaxValidTime(SessionTime);
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////

	// Animate the (vertical) layout transition (i.e. compact mode <-> normal mode).
	Viewport.UpdateLayout();

	TimeRulerTrack->SetSelection(bIsSelecting, SelectionStartTime, SelectionEndTime);

	////////////////////////////////////////////////////////////////////////////////////////////////////

	class FTimingTrackUpdateContext : public ITimingTrackUpdateContext
	{
	public:
		explicit FTimingTrackUpdateContext(SBuildProgressView* InTimingView, const FGeometry& InGeometry, const double InCurrentTime, const float InDeltaTime)
			: TimingView(InTimingView)
			, Geometry(InGeometry)
			, CurrentTime(InCurrentTime)
			, DeltaTime(InDeltaTime)
		{}
	
		virtual const FGeometry& GetGeometry() const override { return Geometry; }
		virtual const FTimingTrackViewport& GetViewport() const override { return TimingView->GetViewport(); }
		virtual const FVector2D& GetMousePosition() const override { return TimingView->GetMousePosition(); }
		virtual const TSharedPtr<const ITimingEvent> GetHoveredEvent() const override { return TimingView->HoveredEvent; }
		virtual const TSharedPtr<const ITimingEvent> GetSelectedEvent() const override { return TimingView->SelectedEvent; }
		virtual const TSharedPtr<ITimingEventFilter> GetEventFilter() const override { return nullptr; }
		virtual const TArray<TUniquePtr<ITimingEventRelation>>& GetCurrentRelations() const override { return TimingView->GetCurrentRelations(); }
		virtual double GetCurrentTime() const override { return CurrentTime; }
		virtual float GetDeltaTime() const override { return DeltaTime; }
	
	public:
		SBuildProgressView* TimingView = nullptr;
		const FGeometry& Geometry;
		double CurrentTime;
		float DeltaTime;
	};
	
	const FTimingTrackUpdateContext UpdateContext(this, AllottedGeometry, InCurrentTime, InDeltaTime);

	////////////////////////////////////////////////////////////////////////////////////////////////////
	// Pre-Update.
	// The tracks needs to update their size.

	{
		FStopwatch PreUpdateTracksStopwatch;
		PreUpdateTracksStopwatch.Start();

		for (const TSharedPtr<FBaseTimingTrack>& TrackPtr : TopDockedTracks)
		{
			if (TrackPtr->IsVisible())
			{
				TrackPtr->PreUpdate(UpdateContext);
			}
		}

		for (TSharedPtr<FBaseTimingTrack>& TrackPtr : ScrollableTracks)
		{
			if (TrackPtr->IsVisible())
			{
				TrackPtr->PreUpdate(UpdateContext);
			}
		}

		PreUpdateTracksStopwatch.Stop();
		// PreUpdateTracksDurationHistory.AddValue(PreUpdateTracksStopwatch.AccumulatedTime);
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////
	// Update Y postion for the visible top docked tracks.
	// Compute the total height of top docked areas.

	int32 NumVisibleTopDockedTracks = 0;
	float TopOffset = 0.0f;
	for (TSharedPtr<FBaseTimingTrack>& TrackPtr : TopDockedTracks)
	{
		SetTrackPosY(TrackPtr, Viewport.GetPosY() + TopOffset);
		if (TrackPtr->IsVisible())
		{
			TopOffset += TrackPtr->GetHeight();
			NumVisibleTopDockedTracks++;
		}
	}
	if (NumVisibleTopDockedTracks > 0)
	{
		bDrawTopSeparatorLine = true;
		TopOffset += 2.0f;
	}
	else
	{
		bDrawTopSeparatorLine = false;
	}
	Viewport.SetTopOffset(TopOffset);

	////////////////////////////////////////////////////////////////////////////////////////////////////
	// Update Y postion for the visible bottom docked tracks.
	// Compute the total height of bottom docked areas.
	
	Viewport.SetBottomOffset(0.0f);
	////////////////////////////////////////////////////////////////////////////////////////////////////
	// Compute the total height of visible scrollable tracks.

	float ScrollHeight = 0.0f;
	for (TSharedPtr<FBaseTimingTrack>& TrackPtr : ScrollableTracks)
	{
		if (TrackPtr->IsVisible())
		{
			ScrollHeight += TrackPtr->GetHeight();
		}
	}
	ScrollHeight += 1.0f; // allow 1 pixel at the bottom (for last horizontal line)

	////////////////////////////////////////////////////////////////////////////////////////////////////
	// Check if vertical scroll area has changed.

	bool bScrollHeightChanged = false;
	if (ScrollHeight != Viewport.GetScrollHeight())
	{
		bScrollHeightChanged = true;
		Viewport.SetScrollHeight(ScrollHeight);
		UpdateVerticalScrollBar();
	}

	// Set the VerticalScrollBar padding so it is limited to the scrollable area.
	VerticalScrollBar->SetPadding(FMargin(0.0f, Viewport.GetPosY() + TopOffset + 2.0f, 0.0f, FMath::Max(0.0f + 2.0f, 10.0f)));

	////////////////////////////////////////////////////////////////////////////////////////////////////
	const float InitialScrollPosY = Viewport.GetScrollPosY();

	TSharedPtr<FBaseTimingTrack> SelectedScrollableTrack;
	if (SelectedTrack.IsValid() && SelectedTrack->IsVisible())
	{
		if (ScrollableTracks.Contains(SelectedTrack))
		{
			SelectedScrollableTrack = SelectedTrack;
		}
	}

	const float InitialPinnedTrackPosY = SelectedScrollableTrack.IsValid() ? SelectedScrollableTrack->GetPosY() : 0.0f;

	// Update the Y position for visible scrollable tracks.
	UpdatePositionForScrollableTracks();

	// The selected track will be pinned (keeps Y pos fixed unless user scrolls vertically).
	if (SelectedScrollableTrack.IsValid())
	{
		const float ScrollingDY = LastScrollPosY - InitialScrollPosY;
		const float PinnedTrackPosY = SelectedScrollableTrack->GetPosY();
		const float AdjustmentDY = InitialPinnedTrackPosY - PinnedTrackPosY + ScrollingDY;

		if (!FMath::IsNearlyZero(AdjustmentDY, 0.5f))
		{
			ViewportScrollPosYOnButtonDown -= AdjustmentDY;
			ScrollAtPosY(InitialScrollPosY - AdjustmentDY);
			UpdatePositionForScrollableTracks();
		}
	}

	// Elastic snap to vertical scroll limits.
	if (!bIsPanning)
	{
		const float Dy = Viewport.GetScrollHeight() - Viewport.GetScrollableAreaHeight();
		const float MinY = FMath::Min(Dy, 0.0f);
		const float MaxY = Dy - MinY;

		float ScrollPosY = Viewport.GetScrollPosY();

		if (ScrollPosY < MinY)
		{
			if (bScrollHeightChanged || Viewport.IsDirty(ETimingTrackViewportDirtyFlags::VLayoutChanged))
			{
				ScrollPosY = MinY;
			}
			else
			{
				constexpr float U = 0.5f;
				ScrollPosY = ScrollPosY * U + (1.0f - U) * MinY;
				if (FMath::IsNearlyEqual(ScrollPosY, MinY, 0.5f))
				{
					ScrollPosY = MinY;
				}
			}
		}
		else if (ScrollPosY > MaxY)
		{
			if (bScrollHeightChanged || Viewport.IsDirty(ETimingTrackViewportDirtyFlags::VLayoutChanged))
			{
				ScrollPosY = MaxY;
			}
			else
			{
				constexpr float U = 0.5f;
				ScrollPosY = ScrollPosY * U + (1.0f - U) * MaxY;
				if (FMath::IsNearlyEqual(ScrollPosY, MaxY, 0.5f))
				{
					ScrollPosY = MaxY;
				}
			}
			if (ScrollPosY < MinY)
			{
				ScrollPosY = MinY;
			}
		}

		if (ScrollPosY != Viewport.GetScrollPosY())
		{
			ScrollAtPosY(ScrollPosY);
		}
	}

	LastScrollPosY = Viewport.GetScrollPosY();

	////////////////////////////////////////////////////////////////////////////////////////////////////
	// At this point it is assumed all tracks have proper position and size.
	// Update.
	{
		FStopwatch UpdateTracksStopwatch;
		UpdateTracksStopwatch.Start();

		for (const TSharedPtr<FBaseTimingTrack>& TrackPtr : TopDockedTracks)
		{
			if (TrackPtr->IsVisible())
			{
				TrackPtr->Update(UpdateContext);
			}
		}

		for (TSharedPtr<FBaseTimingTrack>& TrackPtr : ScrollableTracks)
		{
			if (TrackPtr->IsVisible())
			{
				TrackPtr->Update(UpdateContext);
			}
		}

		UpdateTracksStopwatch.Stop();
		// UpdateTracksDurationHistory.AddValue(UpdateTracksStopwatch.AccumulatedTime);
	}
	////////////////////////////////////////////////////////////////////////////////////////////////////
	// Post-Update.
	{
		FStopwatch PostUpdateTracksStopwatch;
		PostUpdateTracksStopwatch.Start();

		for (const TSharedPtr<FBaseTimingTrack>& TrackPtr : TopDockedTracks)
		{
			if (TrackPtr->IsVisible())
			{
				TrackPtr->PostUpdate(UpdateContext);
			}
		}

		for (TSharedPtr<FBaseTimingTrack>& TrackPtr : ScrollableTracks)
		{
			if (TrackPtr->IsVisible())
			{
				TrackPtr->PostUpdate(UpdateContext);
			}
		}

		PostUpdateTracksStopwatch.Stop();
		PostUpdateTracksDurationHistory.AddValue(PostUpdateTracksStopwatch.AccumulatedTime);
	}
	////////////////////////////////////////////////////////////////////////////////////////////////////

	Tooltip.Update();
	if (!MousePosition.IsZero())
	{
		Tooltip.SetPosition(MousePosition, 0.0f, Viewport.GetWidth() - 12.0f, Viewport.GetPosY(), Viewport.GetPosY() + Viewport.GetHeight() - 12.0f); // -12.0f is to avoid overlaping the scrollbars
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////
	// Update "hovered" and "selected" flags for all visible tracks.
	//TODO: Move this before PreUpdate (so a track could adjust its size based on hovered/selected flags).

	// Reset hovered/selected flags for all tracks.
	for (const TSharedPtr<FBaseTimingTrack>& TrackPtr : TopDockedTracks)
	{
		if (TrackPtr->IsVisible())
		{
			TrackPtr->SetHoveredState(false);
			TrackPtr->SetSelectedFlag(false);
		}
	}

	for (TSharedPtr<FBaseTimingTrack>& TrackPtr : ScrollableTracks)
	{
		if (TrackPtr->IsVisible())
		{
			TrackPtr->SetHoveredState(false);
			TrackPtr->SetSelectedFlag(false);
		}
	}

	// Set the hovered flag for the actual hovered track, if any.
	if (HoveredTrack.IsValid())
	{
		HoveredTrack->SetHoveredState(true);
	}
	
	// // Set the selected flag for the actual selected track, if any.
	if (SelectedTrack.IsValid())
	{
		SelectedTrack->SetSelectedFlag(true);
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////

	Viewport.ResetDirtyFlags();

	TickStopwatch.Stop();
}

int32 SBuildProgressView::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	FStopwatch Stopwatch;
	Stopwatch.Start();

	const bool bEnabled = ShouldBeEnabled(bParentEnabled);
	const ESlateDrawEffect DrawEffects = bEnabled ? ESlateDrawEffect::None : ESlateDrawEffect::DisabledEffect;
	FDrawContext DrawContext(AllottedGeometry, MyCullingRect, InWidgetStyle, DrawEffects, OutDrawElements, LayerId);

	const TSharedRef<FSlateFontMeasure> FontMeasureService = FSlateApplication::Get().GetRenderer()->GetFontMeasureService();
	const float FontScale = AllottedGeometry.Scale;

	const float ViewWidth = static_cast<float>(AllottedGeometry.GetLocalSize().X);
	const float ViewHeight = static_cast<float>(AllottedGeometry.GetLocalSize().Y);

	////////////////////////////////////////////////////////////////////////////////////////////////////

	class FTimingTrackDrawContext : public ITimingTrackDrawContext
	{
	public:
		explicit FTimingTrackDrawContext(const SBuildProgressView* InTimingView, FDrawContext& InDrawContext, const Xiao::FTimingViewDrawHelper& InHelper)
			: TimingView(InTimingView)
			, DrawContext(InDrawContext)
			, Helper(InHelper)
		{}

		virtual const FTimingTrackViewport& GetViewport() const override { return TimingView->GetViewport(); }
		virtual const FVector2D& GetMousePosition() const override { return TimingView->GetMousePosition(); }
		virtual const TSharedPtr<const ITimingEvent> GetHoveredEvent() const override { return TimingView->GetHoveredEvent(); }
		virtual const TSharedPtr<const ITimingEvent> GetSelectedEvent() const override { return TimingView->SelectedEvent; }
		virtual const TSharedPtr<ITimingEventFilter> GetEventFilter() const override { return nullptr; }
		virtual FDrawContext& GetDrawContext() const override { return DrawContext; }
		virtual const ITimingViewDrawHelper& GetHelper() const override { return Helper; }

	public:
		const SBuildProgressView* TimingView;
		FDrawContext& DrawContext;
		const Xiao::FTimingViewDrawHelper& Helper;
	};
	
	Xiao::FTimingViewDrawHelper Helper(DrawContext, Viewport);
	FTimingTrackDrawContext TimingDrawContext(this, DrawContext, Helper);

	////////////////////////////////////////////////////////////////////////////////////////////////////

	// Draw background.
	Helper.DrawBackground();

	//////////////////////////////////////////////////
	// Pre-Draw
	{
		FStopwatch PreDrawTracksStopwatch;
		PreDrawTracksStopwatch.Start();

		for (const TSharedPtr<FBaseTimingTrack>& TrackPtr : ScrollableTracks)
		{
			if (TrackPtr->IsVisible())
			{
				TrackPtr->PreDraw(TimingDrawContext);
			}
		}

		for (const TSharedPtr<FBaseTimingTrack>& TrackPtr : TopDockedTracks)
		{
			if (TrackPtr->IsVisible())
			{
				TrackPtr->PreDraw(TimingDrawContext);
			}
		}

		PreDrawTracksStopwatch.Stop();
		PreDrawTracksDurationHistory.AddValue(PreDrawTracksStopwatch.AccumulatedTime);
	}

	const FVector2f Position = FVector2f(AllottedGeometry.GetAbsolutePosition());
	const float Scale = AllottedGeometry.GetAccumulatedLayoutTransform().GetScale();

	//////////////////////////////////////////////////
	// Draw
	{
		FStopwatch DrawTracksStopwatch;
		DrawTracksStopwatch.Start();

		Helper.BeginDrawTracks();

		// Draw the scrollable tracks.
		{
			const float TopY = Viewport.GetPosY() + Viewport.GetTopOffset();
			const float BottomY = Viewport.GetPosY() + Viewport.GetHeight() - Viewport.GetBottomOffset();

			{
				const float L = Position.X;
				const float R = Position.X + Viewport.GetWidth() * Scale;
				const float T = Position.Y + TopY * Scale;
				const float B = Position.Y + BottomY * Scale;
				const FSlateClippingZone ClipZone(FSlateRect(L, T, R, B));
				DrawContext.ElementList.PushClip(ClipZone);
			}

			for (const TSharedPtr<FBaseTimingTrack>& TrackPtr : ScrollableTracks)
			{
				if (TrackPtr->IsVisible())
				{
					if (TrackPtr->GetPosY() + TrackPtr->GetHeight() <= TopY)
					{
						continue;
					}
					if (TrackPtr->GetPosY() >= BottomY)
					{
						break;
					}
					TrackPtr->Draw(TimingDrawContext);
				}
			}

			// Draw relations between scrollable tracks.
			const Xiao::FTimingViewDrawHelper& TimingHelper = *static_cast<const Xiao::FTimingViewDrawHelper*>(&TimingDrawContext.GetHelper());
			TimingHelper.DrawRelations(CurrentRelations, ITimingEventRelation::EDrawFilter::BetweenScrollableTracks);

			DrawContext.ElementList.PopClip();
		}

		// Draw the top docked tracks.
		{
			const float TopY = Viewport.GetPosY();
			const float BottomY = Viewport.GetPosY() + Viewport.GetTopOffset();

			{
				const float L = Position.X;
				const float R = Position.X + Viewport.GetWidth() * Scale;
				const float T = Position.Y + TopY * Scale;
				const float B = Position.Y + BottomY * Scale;
				const FSlateClippingZone ClipZone(FSlateRect(L, T, R, B));
				DrawContext.ElementList.PushClip(ClipZone);
			}

			for (const TSharedPtr<FBaseTimingTrack>& TrackPtr : TopDockedTracks)
			{
				if (TrackPtr->IsVisible())
				{
					TrackPtr->Draw(TimingDrawContext);
				}
			}

			if (bDrawTopSeparatorLine)
			{
				// Draw separator line between top docked tracks and scrollable tracks.
				DrawContext.DrawBox(0.0f, BottomY - 2.0f, Viewport.GetWidth(), 2.0f, WhiteBrush, FLinearColor(0.01f, 0.01f, 0.01f, 1.0f));
				++DrawContext.LayerId;
			}

			DrawContext.ElementList.PopClip();
		}
		
		Helper.EndDrawTracks();

		DrawTracksStopwatch.Stop();
		DrawTracksDurationHistory.AddValue(DrawTracksStopwatch.AccumulatedTime);
	}

	//////////////////////////////////////////////////
	// Draw the selected and/or hovered event.

	if (ITimingEvent::AreValidAndEquals(SelectedEvent, HoveredEvent))
	{
		const TSharedRef<const FBaseTimingTrack> TrackPtr = SelectedEvent->GetTrack();
	
		// Highlight the selected and hovered timing event (if any).
		if (TrackPtr->IsVisible())
		{
			SelectedEvent->GetTrack()->DrawEvent(TimingDrawContext, *SelectedEvent, EDrawEventMode::SelectedAndHovered);
		}
	}
	else
	{
		// Highlight the selected timing event (if any).
		if (SelectedEvent.IsValid())
		{
			const TSharedRef<const FBaseTimingTrack> TrackPtr = SelectedEvent->GetTrack();
			if (TrackPtr->IsVisible())
			{
				SelectedEvent->GetTrack()->DrawEvent(TimingDrawContext, *SelectedEvent, EDrawEventMode::Selected);
			}
		}
	
		// Highlight the hovered timing event (if any).
		if (HoveredEvent.IsValid())
		{
			const TSharedRef<const FBaseTimingTrack> TrackPtr = HoveredEvent->GetTrack();
			if (TrackPtr->IsVisible())
			{
				HoveredEvent->GetTrack()->DrawEvent(TimingDrawContext, *HoveredEvent, EDrawEventMode::Hovered);
			}
		}
	}

	// Draw the time range selection.
	Xiao::FDrawHelpers::DrawTimeRangeSelection(DrawContext, Viewport, SelectionStartTime, SelectionEndTime, WhiteBrush, MainFont);

	//////////////////////////////////////////////////
	// Post-Draw
	{
		FStopwatch PostDrawTracksStopwatch;
		PostDrawTracksStopwatch.Start();

		for (const TSharedPtr<FBaseTimingTrack>& TrackPtr : ScrollableTracks)
		{
			if (TrackPtr->IsVisible())
			{
				TrackPtr->PostDraw(TimingDrawContext);
			}
		}

		for (const TSharedPtr<FBaseTimingTrack>& TrackPtr : TopDockedTracks)
		{
			if (TrackPtr->IsVisible())
			{
				TrackPtr->PostDraw(TimingDrawContext);
			}
		}

		PostDrawTracksStopwatch.Stop();
		PostDrawTracksDurationHistory.AddValue(PostDrawTracksStopwatch.AccumulatedTime);
	}

	//////////////////////////////////////////////////
	// Draw relations between docked tracks.
	{
		bool bIsClipZoneSet = false;
		//if (GraphTrack->IsVisible())
		//{
		//	// Avoid overlapping the Main Graph track (when top docked).
		//	const float TopY = GraphTrack->GetPosY() + GraphTrack->GetHeight();
		//	const float BottomY = Viewport.GetPosY() + Viewport.GetHeight();
		//
		//	const float L = Position.X;
		//	const float R = Position.X + Viewport.GetWidth() * Scale;
		//	const float T = Position.Y + TopY * Scale;
		//	const float B = Position.Y + BottomY * Scale;
		//	const FSlateClippingZone ClipZone(FSlateRect(L, T, R, B));
		//	DrawContext.ElementList.PushClip(ClipZone);
		//	bIsClipZoneSet = true;
		//}
		
		const Xiao::FTimingViewDrawHelper& TimingHelper = *static_cast<const Xiao::FTimingViewDrawHelper*>(&TimingDrawContext.GetHelper());
		TimingHelper.DrawRelations(CurrentRelations, ITimingEventRelation::EDrawFilter::BetweenDockedTracks);
		
		if (bIsClipZoneSet)
		{
			DrawContext.ElementList.PopClip();
		}
	}
	//////////////////////////////////////////////////

	// Draw tooltip with info about hovered event.
	Tooltip.Draw(DrawContext);

	// Fill the background of the toolbar.
	DrawContext.DrawBox(0.0f, 0.0f, ViewWidth, Viewport.GetPosY(), WhiteBrush, FSlateColor(EStyleColor::Panel).GetSpecifiedColor());

	// Fill the background of the vertical scrollbar.
	const float ScrollBarHeight = Viewport.GetScrollableAreaHeight();
	if (ScrollBarHeight > 0)
	{
		constexpr float ScrollBarWidth = 12.0f;
		DrawContext.DrawBox(ViewWidth - ScrollBarWidth, Viewport.GetPosY() + Viewport.GetTopOffset(), ScrollBarWidth, ScrollBarHeight, WhiteBrush, FSlateColor(EStyleColor::Panel).GetSpecifiedColor());
	}

	//////////////////////////////////////////////////
	// Draw the overscroll indication lines.

	constexpr float OverscrollLineSize = 1.0f;
	constexpr int32 OverscrollLineCount = 8;

	if (OverscrollLeft > 0.0f)
	{
		// TODO: single box with gradient opacity
		const float OverscrollLineY = Viewport.GetPosY();
		const float OverscrollLineH = Viewport.GetHeight();
		for (int32 LineIndex = 0; LineIndex < OverscrollLineCount; ++LineIndex)
		{
			const float Opacity = OverscrollLeft * static_cast<float>(OverscrollLineCount - LineIndex) / static_cast<float>(OverscrollLineCount);
			DrawContext.DrawBox(LineIndex * OverscrollLineSize, OverscrollLineY, OverscrollLineSize, OverscrollLineH, WhiteBrush, FLinearColor(1.0f, 0.1f, 0.1f, Opacity));
		}
	}
	if (OverscrollRight > 0.0f)
	{
		const float OverscrollLineY = Viewport.GetPosY();
		const float OverscrollLineH = Viewport.GetHeight();
		for (int32 LineIndex = 0; LineIndex < OverscrollLineCount; ++LineIndex)
		{
			const float Opacity = OverscrollRight * static_cast<float>(OverscrollLineCount - LineIndex) / static_cast<float>(OverscrollLineCount);
			DrawContext.DrawBox(ViewWidth - (1 + LineIndex) * OverscrollLineSize, OverscrollLineY, OverscrollLineSize, OverscrollLineH, WhiteBrush, FLinearColor(1.0f, 0.1f, 0.1f, Opacity));
		}
	}
	if (OverscrollTop > 0.0f)
	{
		const float OverscrollLineY = Viewport.GetPosY() + Viewport.GetTopOffset();
		for (int32 LineIndex = 0; LineIndex < OverscrollLineCount; ++LineIndex)
		{
			const float Opacity = OverscrollTop * static_cast<float>(OverscrollLineCount - LineIndex) / static_cast<float>(OverscrollLineCount);
			DrawContext.DrawBox(0.0f, OverscrollLineY + LineIndex * OverscrollLineSize, ViewWidth, OverscrollLineSize, WhiteBrush, FLinearColor(1.0f, 0.1f, 0.1f, Opacity));
		}
	}
	if (OverscrollBottom > 0.0f)
	{
		const float OverscrollLineY = Viewport.GetPosY() + Viewport.GetHeight() - Viewport.GetBottomOffset();
		for (int32 LineIndex = 0; LineIndex < OverscrollLineCount; ++LineIndex)
		{
			const float Opacity = OverscrollBottom * static_cast<float>(OverscrollLineCount - LineIndex) / static_cast<float>(OverscrollLineCount);
			DrawContext.DrawBox(0.0f, OverscrollLineY - (1 + LineIndex) * OverscrollLineSize, ViewWidth, OverscrollLineSize, WhiteBrush, FLinearColor(1.0f, 0.1f, 0.1f, Opacity));
		}
	}

	//////////////////////////////////////////////////
#if 0

	if (bShouldDisplayDebugInfo)
	{
		const FSlateFontInfo& SummaryFont = MainFont;
	
		const float MaxFontCharHeight = static_cast<float>(FontMeasureService->Measure(TEXT("!"), SummaryFont, FontScale).Y / FontScale);
		const float DbgDy = MaxFontCharHeight;
	
		constexpr float DbgW = 320.0f;
		const float DbgH = DbgDy * 9 + 3.0f;
		const float DbgX = ViewWidth - DbgW - 20.0f;
		float DbgY = Viewport.GetPosY() + Viewport.GetTopOffset() + 10.0f;
	
		DrawContext.LayerId++;
	
		DrawContext.DrawBox(DbgX - 2.0f, DbgY - 2.0f, DbgW, DbgH, WhiteBrush, FLinearColor(1.0f, 1.0f, 1.0f, 0.9f));
		DrawContext.LayerId++;
	
		FLinearColor DbgTextColor(0.0f, 0.0f, 0.0f, 0.9f);
	
		//////////////////////////////////////////////////
		// Display the "Draw" performance info.
		// Time interval since last OnPaint call.
		const uint64 CurrentTime = FPlatformTime::Cycles64();
		const uint64 OnPaintDeltaTime = CurrentTime - LastOnPaintTime;
		LastOnPaintTime = CurrentTime;
		OnPaintDeltaTimeHistory.AddValue(OnPaintDeltaTime); // saved for last 32 OnPaint calls
		const uint64 AvgOnPaintDeltaTime = OnPaintDeltaTimeHistory.ComputeAverage();
		const uint64 AvgOnPaintDeltaTimeMs = FStopwatch::Cycles64ToMilliseconds(AvgOnPaintDeltaTime);
		const double AvgOnPaintFps = AvgOnPaintDeltaTimeMs != 0 ? 1.0 / FStopwatch::Cycles64ToSeconds(AvgOnPaintDeltaTime) : 0.0;
	
		const uint64 AvgPreDrawTracksDurationMs = FStopwatch::Cycles64ToMilliseconds(PreDrawTracksDurationHistory.ComputeAverage());
		const uint64 AvgDrawTracksDurationMs = FStopwatch::Cycles64ToMilliseconds(DrawTracksDurationHistory.ComputeAverage());
		const uint64 AvgPostDrawTracksDurationMs = FStopwatch::Cycles64ToMilliseconds(PostDrawTracksDurationHistory.ComputeAverage());
		const uint64 AvgTotalDrawDurationMs = FStopwatch::Cycles64ToMilliseconds(TotalDrawDurationHistory.ComputeAverage());
	
		DrawContext.DrawText(
			DbgX, DbgY,
			FString::Printf(TEXT("D: %llu ms + %llu ms + %llu ms + %llu ms = %llu ms | + %llu ms = %llu ms (%d fps)"),
				AvgPreDrawTracksDurationMs,			// pre-draw tracks time
				AvgDrawTracksDurationMs,			// draw tracks time
				AvgPostDrawTracksDurationMs,		// post-draw tracks time
				AvgTotalDrawDurationMs - AvgPreDrawTracksDurationMs - AvgDrawTracksDurationMs - AvgPostDrawTracksDurationMs, // other draw code
				AvgTotalDrawDurationMs,
				AvgOnPaintDeltaTimeMs - AvgTotalDrawDurationMs, // other overhead to OnPaint calls
				AvgOnPaintDeltaTimeMs,				// average time between two OnPaint calls
				FMath::RoundToInt(AvgOnPaintFps)),	// framerate of OnPaint calls
			SummaryFont, DbgTextColor
		);
		DbgY += DbgDy;


	 	//////////////////////////////////////////////////
	 	// Display the "update" performance info.

		const uint64 AvgPreUpdateTracksDurationMs = FStopwatch::Cycles64ToMilliseconds(PreUpdateTracksDurationHistory.ComputeAverage());
		const uint64 AvgUpdateTracksDurationMs = FStopwatch::Cycles64ToMilliseconds(UpdateTracksDurationHistory.ComputeAverage());
		const uint64 AvgPostUpdateTracksDurationMs = FStopwatch::Cycles64ToMilliseconds(PostUpdateTracksDurationHistory.ComputeAverage());
		const uint64 AvgTickDurationMs = FStopwatch::Cycles64ToMilliseconds(TickDurationHistory.ComputeAverage());
	
	 	DrawContext.DrawText(
			DbgX, DbgY,
			FString::Printf(TEXT("U avg: %llu ms + %llu ms + %llu ms + %llu ms = %llu ms"),
				AvgPreUpdateTracksDurationMs,
				AvgUpdateTracksDurationMs,
				AvgPostUpdateTracksDurationMs,
				AvgTickDurationMs - AvgPreUpdateTracksDurationMs - AvgUpdateTracksDurationMs - AvgPostUpdateTracksDurationMs,
				AvgTickDurationMs),
			SummaryFont, DbgTextColor
		);
		DbgY += DbgDy;
	
	 	//////////////////////////////////////////////////
	 	// Display timing events stats.

		DrawContext.DrawText(
			DbgX, DbgY,
			FString::Format(TEXT("{0} events : {1} ({2}) boxes, {3} borders, {4} texts"),
			{
				FText::AsNumber(Helper.GetNumEvents()).ToString(),
				FText::AsNumber(Helper.GetNumDrawBoxes()).ToString(),
				FText::AsPercent(static_cast<double>(Helper.GetNumDrawBoxes()) / (Helper.GetNumDrawBoxes() + Helper.GetNumMergedBoxes())).ToString(),
				FText::AsNumber(Helper.GetNumDrawBorders()).ToString(),
				FText::AsNumber(Helper.GetNumDrawTexts()).ToString(),
				//OutDrawElements.GetRootDrawLayer().GetElementCount(),
			}),
			SummaryFont, DbgTextColor
		);
		DbgY += DbgDy;

	
	 	//////////////////////////////////////////////////
	 	// Display time markers stats.
	
		if (MarkersTrack->IsVisible())
		{
			DrawContext.DrawText(
				DbgX, DbgY,
				FString::Format(TEXT("{0} logs : {1} boxes, {2} texts"),
				{
					FText::AsNumber(MarkersTrack->GetNumLogMessages()).ToString(),
					FText::AsNumber(MarkersTrack->GetNumBoxes()).ToString(),
					FText::AsNumber(MarkersTrack->GetNumTexts()).ToString(),
				}),
				SummaryFont, DbgTextColor
			);
			DbgY += DbgDy;
		}
	
	 	//////////////////////////////////////////////////
	 	// Display Graph track stats.
	
		if (GraphTrack)
		{
			DrawContext.DrawText(
				DbgX, DbgY,
				FString::Format(TEXT("{0} events : {1} points, {2} lines, {3} boxes"),
					{
						FText::AsNumber(GraphTrack->GetNumAddedEvents()).ToString(),
						FText::AsNumber(GraphTrack->GetNumDrawPoints()).ToString(),
						FText::AsNumber(GraphTrack->GetNumDrawLines()).ToString(),
						FText::AsNumber(GraphTrack->GetNumDrawBoxes()).ToString(),
					}),
					SummaryFont, DbgTextColor
			);
			DbgY += DbgDy;
		}

	
	 	//////////////////////////////////////////////////
	 	// Display viewport's horizontal info.
	
		DrawContext.DrawText(
			DbgX, DbgY,
			FString::Printf(TEXT("SX: %g, ST: %g, ET: %s"), Viewport.GetScaleX(), Viewport.GetStartTime(), *TimeUtils::FormatTimeAuto(Viewport.GetMaxValidTime())),
			SummaryFont, DbgTextColor
		);
		DbgY += DbgDy;
	
		//////////////////////////////////////////////////
		// Display viewport's vertical info.
	
		DrawContext.DrawText(
			DbgX, DbgY,
			FString::Printf(TEXT("Y: %.2f, H: %g, VH: %g"), Viewport.GetScrollPosY(), Viewport.GetScrollHeight(), Viewport.GetHeight()), SummaryFont, DbgTextColor
		);
		DbgY += DbgDy;

		// TODO 当前核心数 // Hz
		DrawContext.DrawText(
			DbgX, DbgY,
			FString::Printf(TEXT("1024 核心/ 8096 Hz")),
			SummaryFont, DbgTextColor
		);
	}
#endif

	if (*bRealtime)
	{
		if (RealtimeBrush != nullptr)
		{
			FSlateDrawElement::MakeBox(
				OutDrawElements,
				DrawContext.LayerId+1,
				AllottedGeometry.ToPaintGeometry(),
				RealtimeBrush
			);
		}
	}
	//////////////////////////////////////////////////

	Stopwatch.Stop();
	TotalDrawDurationHistory.AddValue(Stopwatch.AccumulatedTime);

	return SCompoundWidget::OnPaint(Args, AllottedGeometry, MyCullingRect, OutDrawElements, LayerId, InWidgetStyle, bParentEnabled && IsEnabled());
}

FReply SBuildProgressView::OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	FReply Reply = AllowTracksToProcessOnMouseButtonDown(MyGeometry, MouseEvent);
	if (Reply.IsEventHandled())
	{
		return Reply;
	}

	MousePositionOnButtonDown = MyGeometry.AbsoluteToLocal(MouseEvent.GetScreenSpacePosition());
	MousePosition = MousePositionOnButtonDown;

	if (bAllowPanningOnScreenEdges)
	{
		const FVector2f ScreenSpacePosition = FVector2f(MouseEvent.GetScreenSpacePosition());
		DPIScaleFactor = FPlatformApplicationMisc::GetDPIScaleFactorAtPoint(ScreenSpacePosition.X, ScreenSpacePosition.Y);

		EdgeFrameCountX = 0;
		EdgeFrameCountY = 0;
	}

	bool bStartPanningSelectingOrScrubbing = false;
	bool bStartPanning = false;
	bool bStartSelecting = false;
	bool bStartScrubbing = false;

	if (MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		if (!bIsRMB_Pressed)
		{
			bIsLMB_Pressed = true;
			bStartPanningSelectingOrScrubbing = true;
			SelectHoveredTimingTrack();
		}
	}
	else if (MouseEvent.GetEffectingButton() == EKeys::RightMouseButton)
	{
		if (!bIsLMB_Pressed)
		{
			bIsRMB_Pressed = true;
			bStartPanningSelectingOrScrubbing = true;
			SelectHoveredTimingTrack();
		}
	}

	if (bStartPanningSelectingOrScrubbing)
	{
		bool bIsHoveringTimeRulerTrack = false;
		if (TimeRulerTrack->IsVisible())
		{
			bIsHoveringTimeRulerTrack = MousePositionOnButtonDown.Y >= TimeRulerTrack->GetPosY() && MousePositionOnButtonDown.Y < TimeRulerTrack->GetPosY() + TimeRulerTrack->GetHeight();
			if (bIsHoveringTimeRulerTrack)
			{
			}
		}

		if (bIsSpaceBarKeyPressed)
		{
			bStartPanning = true;
		}
		else if (bIsHoveringTimeRulerTrack || (MouseEvent.GetModifierKeys().IsControlDown() && MouseEvent.GetModifierKeys().IsShiftDown()))
		{
			bStartSelecting = true;
		}
		else
		{
			bStartPanning = true;
		}

		// Capture mouse, so we can drag outside this widget.
		if (bAllowPanningOnScreenEdges)
		{
			Reply = FReply::Handled().CaptureMouse(SharedThis(this)).UseHighPrecisionMouseMovement(SharedThis(this)).SetUserFocus(SharedThis(this), EFocusCause::Mouse);
		}
		else
		{
			Reply = FReply::Handled().CaptureMouse(SharedThis(this));
		}
	}

	if (bPreventThrottling)
	{
		Reply.PreventThrottling();
	}

	if (bStartScrubbing)
	{
		bIsPanning = false;
		bIsDragging = false;
	}
	else if (bStartPanning)
	{
		bIsPanning = true;
		bIsDragging = false;
		TimeRulerTrack->StopScrubbing();

		ViewportStartTimeOnButtonDown = Viewport.GetStartTime();
		ViewportScrollPosYOnButtonDown = Viewport.GetScrollPosY();

		if (MouseEvent.GetModifierKeys().IsControlDown())
		{
			// Allow panning only horizontally.
			PanningMode = EPanningMode::Horizontal;
		}
		else if (MouseEvent.GetModifierKeys().IsShiftDown())
		{
			// Allow panning only vertically.
			PanningMode = EPanningMode::Vertical;
		}
		else
		{
			// Allow panning both horizontally and vertically.
			PanningMode = EPanningMode::HorizontalAndVertical;
		}
	}
	else if (bStartSelecting)
	{
		bIsSelecting = true;
		bIsDragging = false;
		TimeRulerTrack->StopScrubbing();

		SelectionStartTime = Viewport.SlateUnitsToTime(static_cast<float>(MousePositionOnButtonDown.X));
		SelectionEndTime = SelectionStartTime;
		LastSelectionType = ESelectionType::None;
		RaiseSelectionChanging();
	}

	return Reply;
}

FReply SBuildProgressView::OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	FReply Reply = AllowTracksToProcessOnMouseButtonUp(MyGeometry, MouseEvent);
	if (Reply.IsEventHandled())
	{
		return Reply;
	}

	MousePositionOnButtonUp = MyGeometry.AbsoluteToLocal(MouseEvent.GetScreenSpacePosition());
	MousePosition = MousePositionOnButtonUp;

	const bool bIsValidForMouseClick = MousePositionOnButtonUp.Equals(MousePositionOnButtonDown, 2.0f);

	if (MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		if (bIsLMB_Pressed)
		{
			if (bIsPanning)
			{
				PanningMode = EPanningMode::None;
				bIsPanning = false;
			}
			else if (bIsSelecting)
			{
				RaiseSelectionChanged();
				bIsSelecting = false;
			}
			else if (TimeRulerTrack->IsScrubbing())
			{
				RaiseTimeMarkerChanged(TimeRulerTrack->GetScrubbingTimeMarker());
				TimeRulerTrack->StopScrubbing();
			}

			if (bIsValidForMouseClick)
			{
				// Select the hovered timing event (if any).
				UpdateHoveredTimingEvent(static_cast<float>(MousePositionOnButtonUp.X), static_cast<float>(MousePositionOnButtonUp.Y));
				SelectHoveredTimingTrack();
				// SelectHoveredTimingEvent();

				// When clicking on an empty space...
				if (!SelectedEvent.IsValid())
				{
					// ...reset selection.
					SelectionEndTime = SelectionStartTime = 0.0;
					LastSelectionType = ESelectionType::None;
					RaiseSelectionChanged();
				}
			}

			bIsDragging = false;

			// Release mouse as we no longer drag.
			Reply = FReply::Handled().ReleaseMouseCapture();

			bIsLMB_Pressed = false;
		}
	}
	else if (MouseEvent.GetEffectingButton() == EKeys::RightMouseButton)
	{
		if (bIsRMB_Pressed)
		{
			if (bIsPanning)
			{
				PanningMode = EPanningMode::None;
				bIsPanning = false;
			}
			else if (bIsSelecting)
			{
				RaiseSelectionChanged();
				bIsSelecting = false;
			}
			else if (TimeRulerTrack->IsScrubbing())
			{
				RaiseTimeMarkerChanged(TimeRulerTrack->GetScrubbingTimeMarker());
				TimeRulerTrack->StopScrubbing();
			}

			if (bIsValidForMouseClick)
			{
				SelectHoveredTimingTrack();
				ShowContextMenu(MouseEvent);
			}

			bIsDragging = false;
			
			Reply = FReply::Handled().ReleaseMouseCapture();

			bIsRMB_Pressed = false;
		}
	}
	return Reply;
}

FReply SBuildProgressView::OnMouseButtonDoubleClick(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	FReply Reply = AllowTracksToProcessOnMouseButtonDoubleClick(MyGeometry, MouseEvent);
	if (Reply.IsEventHandled())
	{
		return Reply;
	}

	if (MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		if (HoveredEvent.IsValid())
		{
			if (MouseEvent.GetModifierKeys().IsControlDown())
			{
				const double EndTime = Viewport.RestrictEndTime(HoveredEvent->GetEndTime());
				SelectTimeInterval(HoveredEvent->GetStartTime(), EndTime - HoveredEvent->GetStartTime());
			}
			else if (const FProcessTrackEvent* ProcessTrack = static_cast<const FProcessTrackEvent*>(HoveredEvent.Get()))
			{
				const FName Identifier(FString::FromInt(ProcessTrack->GetProcessDesc().id));
				OnEventClicked.ExecuteIfBound(Identifier);
			}
		}

		Reply = FReply::Handled();
	}

	return Reply;
}

FReply SBuildProgressView::OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	FReply Reply = FReply::Unhandled();

	MousePosition = MyGeometry.AbsoluteToLocal(MouseEvent.GetScreenSpacePosition());

	const FVector2D& CursorDelta = MouseEvent.GetCursorDelta();

	if (bIsPanning && bAllowPanningOnScreenEdges)
	{
		if (MouseEvent.GetScreenSpacePosition().X == MouseEvent.GetLastScreenSpacePosition().X)
		{
			++EdgeFrameCountX;
		}
		else
		{
			EdgeFrameCountX = 0;
		}

		if (EdgeFrameCountX > 10) // handle delay between high precision mouse movement and update of the actual cursor position
		{
			MousePositionOnButtonDown.X -= CursorDelta.X / DPIScaleFactor;
		}

		if (MouseEvent.GetScreenSpacePosition().Y == MouseEvent.GetLastScreenSpacePosition().Y)
		{
			++EdgeFrameCountY;
		}
		else
		{
			EdgeFrameCountY = 0;
		}

		if (EdgeFrameCountY > 10) // handle delay between high precision mouse movement and update of the actual cursor position
		{
			MousePositionOnButtonDown.Y -= CursorDelta.Y / DPIScaleFactor;
		}
	}

	if (!CursorDelta.IsZero())
	{
		if (bIsPanning)
		{
			if (HasMouseCapture())
			{
				bIsDragging = true;

				const auto Mode = static_cast<int32>(PanningMode);
				if (Mode & static_cast<int32>(EPanningMode::Horizontal))
				{
					const double StartTime = ViewportStartTimeOnButtonDown + static_cast<double>(MousePositionOnButtonDown.X - MousePosition.X) / Viewport.GetScaleX();
					ScrollAtTime(StartTime);
				}

				if (Mode & static_cast<int32>(EPanningMode::Vertical))
				{
					const float ScrollPosY = ViewportScrollPosYOnButtonDown + static_cast<float>(MousePositionOnButtonDown.Y - MousePosition.Y);
					ScrollAtPosY(ScrollPosY);
				}
			}
		}
		else if (bIsSelecting)
		{
			if (HasMouseCapture())
			{
				bIsDragging = true;

				SelectionStartTime = Viewport.SlateUnitsToTime(static_cast<float>(MousePositionOnButtonDown.X));
				SelectionEndTime = Viewport.SlateUnitsToTime(static_cast<float>(MousePosition.X));
				if (SelectionStartTime > SelectionEndTime)
				{
					const double Temp = SelectionStartTime;
					SelectionStartTime = SelectionEndTime;
					SelectionEndTime = Temp;
				}
				LastSelectionType = ESelectionType::TimeRange;
				RaiseSelectionChanging();
			}
		}
		else if (TimeRulerTrack->IsScrubbing())
		{
			if (HasMouseCapture())
			{
				bIsDragging = true;
				const double Time = Viewport.SlateUnitsToTime(static_cast<float>(MousePosition.X));

				const TSharedRef<Xiao::FTimeMarker> ScrubbingTimeMarker = TimeRulerTrack->GetScrubbingTimeMarker();
				ScrubbingTimeMarker->SetTime(Time);
				RaiseTimeMarkerChanging(ScrubbingTimeMarker);
			}
		}
		else
		{
			UpdateHoveredTimingEvent(static_cast<float>(MousePosition.X), static_cast<float>(MousePosition.Y));
		}

		Reply = FReply::Handled();
	}

	return Reply;
}

void SBuildProgressView::OnMouseEnter(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
}

void SBuildProgressView::OnMouseLeave(const FPointerEvent& MouseEvent)
{
	if (!HasMouseCapture())
	{
		// No longer dragging (unless we have mouse capture).
		bIsDragging = false;
		bIsPanning = false;
		bIsSelecting = false;

		bIsLMB_Pressed = false;
		bIsRMB_Pressed = false;

		MousePosition = FVector2D::ZeroVector;

		if (HoveredTrack.IsValid())
		{
			HoveredTrack.Reset();
			OnHoveredTrackChangedDelegate.Broadcast(HoveredTrack);
		}
		if (HoveredEvent.IsValid())
		{
			HoveredEvent.Reset();
			OnHoveredEventChangedDelegate.Broadcast(HoveredEvent);
		}
		Tooltip.SetDesiredOpacity(0.0f);
	}
}

FReply SBuildProgressView::OnMouseWheel(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	if (MouseEvent.GetModifierKeys().IsShiftDown())
	{
		if (ScrollableTracks.Num() > 0)
		{
			if (auto GraphTrack = StaticCastSharedPtr<FGraphTrack>(ScrollableTracks[0]))
			{
				if (GraphTrack->IsVisible() &&
					MousePosition.Y >= GraphTrack->GetPosY() &&
					MousePosition.Y < GraphTrack->GetPosY() + GraphTrack->GetHeight())
				{
					// Zoom in/out vertically.
					const double Delta = MouseEvent.GetWheelDelta();
					constexpr double ZoomStep = 0.25; // as percent
					double ScaleY = GraphTrack->GetSharedValueViewport().GetScaleY();
					if (Delta > 0)
					{
						ScaleY *= FMath::Pow(1.0 + ZoomStep, Delta);
						if (constexpr double MaxScaleY = 1.0e10; ScaleY > MaxScaleY)
						{
							ScaleY = MaxScaleY;
						}
					}
					else
					{
						ScaleY *= FMath::Pow(1.0 / (1.0 + ZoomStep), -Delta);
						if (constexpr double MinScaleY = 0.0001; ScaleY < MinScaleY)
						{
							ScaleY = MinScaleY;
						}
					}
					GraphTrack->GetSharedValueViewport().SetScaleY(ScaleY);

					for (const TSharedPtr<FGraphSeries>& Series : GraphTrack->GetSeries())
					{
						if (Series->IsUsingSharedViewport())
						{
							Series->SetScaleY(ScaleY);
							Series->SetDirtyFlag();
						}
					}
				}
				else
				{
					// Scroll vertically.
					constexpr float ScrollSpeedY = 16.0f * 3;
					const float NewScrollPosY = Viewport.GetScrollPosY() - ScrollSpeedY * MouseEvent.GetWheelDelta();
					ScrollAtPosY(EnforceVerticalScrollLimits(NewScrollPosY));
				}
			}
			
		}
	}
	else if (MouseEvent.GetModifierKeys().IsControlDown())
	{
		// Scroll horizontally.
		const double ScrollSpeedX = Viewport.GetDurationForViewportDX(16.0 * 3);
		const double NewStartTime = Viewport.GetStartTime() - ScrollSpeedX * MouseEvent.GetWheelDelta();
		ScrollAtTime(EnforceHorizontalScrollLimits(NewStartTime));
	}
	else
	{
		// Zoom in/out horizontally.
		const float Delta = MouseEvent.GetWheelDelta();
		if (Viewport.RelativeZoomWithFixedX(Delta, static_cast<float>(MousePosition.X)))
		{
			UpdateHorizontalScrollBar();
		}
	}

	return FReply::Handled();
}

FCursorReply SBuildProgressView::OnCursorQuery(const FGeometry& MyGeometry, const FPointerEvent& CursorEvent) const
{
	if (bIsPanning)
	{
		if (bIsDragging)
		{
			return FCursorReply::Cursor(EMouseCursor::GrabHand);
		}
	}
	else if (bIsSelecting)
	{
		if (bIsDragging)
		{
			return FCursorReply::Cursor(EMouseCursor::ResizeLeftRight);
		}
	}
	else if (bIsSpaceBarKeyPressed)
	{
		return FCursorReply::Cursor(EMouseCursor::GrabHand);
	}

	return FCursorReply::Unhandled();
}

FReply SBuildProgressView::OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent)
{
	if (InKeyEvent.GetKey() == EKeys::B)
	{
		if (!InKeyEvent.GetModifierKeys().IsControlDown() && !InKeyEvent.GetModifierKeys().IsShiftDown())
		{
			return FReply::Handled();
		}
	}
	else if (InKeyEvent.GetKey() == EKeys::M)
	{
		if (!InKeyEvent.GetModifierKeys().IsControlDown() && !InKeyEvent.GetModifierKeys().IsShiftDown())
		{
			return FReply::Handled();
		}
	}
	else if (InKeyEvent.GetKey() == EKeys::F)
	{
		if (!InKeyEvent.GetModifierKeys().IsControlDown() && !InKeyEvent.GetModifierKeys().IsShiftDown())
		{
			// FrameSelection();
			return FReply::Handled();
		}
	}
	else if (InKeyEvent.GetKey() == EKeys::C)
	{
		if (InKeyEvent.GetModifierKeys().IsControlDown())
		{
			if (SelectedEvent.IsValid())
			{
				SelectedEvent->GetTrack()->OnClipboardCopyEvent(*SelectedEvent);
			}
			return FReply::Handled();
		}
	}
	else if (InKeyEvent.GetKey() == EKeys::Equals || InKeyEvent.GetKey() == EKeys::Add)
	{
		// Zoom In
		const double ScaleX = Viewport.GetScaleX() * 1.25;
		if (Viewport.ZoomWithFixedX(ScaleX, Viewport.GetWidth() / 2))
		{
			UpdateHorizontalScrollBar();
		}
		return FReply::Handled();
	}
	else if (InKeyEvent.GetKey() == EKeys::Hyphen || InKeyEvent.GetKey() == EKeys::Subtract)
	{
		// Zoom Out
		const double ScaleX = Viewport.GetScaleX() / 1.25;
		if (Viewport.ZoomWithFixedX(ScaleX, Viewport.GetWidth() / 2))
		{
			UpdateHorizontalScrollBar();
		}
		return FReply::Handled();
	}
	else if (InKeyEvent.GetKey() == EKeys::Left)
	{
		if (InKeyEvent.GetModifierKeys().IsControlDown())
		{
			// Scroll Left
			const double NewStartTime = Viewport.GetStartTime() - Viewport.GetDuration() * 0.05;
			ScrollAtTime(EnforceHorizontalScrollLimits(NewStartTime));
		}
		else
		{
			SelectLeftTimingEvent();
		}
		return FReply::Handled();
	}
	else if (InKeyEvent.GetKey() == EKeys::Right)
	{
		if (InKeyEvent.GetModifierKeys().IsControlDown())
		{
			// Scroll Right
			const double NewStartTime = Viewport.GetStartTime() + Viewport.GetDuration() * 0.05;
			ScrollAtTime(EnforceHorizontalScrollLimits(NewStartTime));
		}
		else
		{
			SelectRightTimingEvent();
		}
		return FReply::Handled();
	}
	else if (InKeyEvent.GetKey() == EKeys::Up)
	{
		if (InKeyEvent.GetModifierKeys().IsControlDown())
		{
			// Scroll Up
			const float NewScrollPosY = Viewport.GetScrollPosY() - 16.0f * 3;
			ScrollAtPosY(EnforceVerticalScrollLimits(NewScrollPosY));
		}
		else
		{
			SelectUpTimingEvent();
		}
		return FReply::Handled();
	}
	else if (InKeyEvent.GetKey() == EKeys::Down)
	{
		if (InKeyEvent.GetModifierKeys().IsControlDown())
		{
			// Scroll Down
			const float NewScrollPosY = Viewport.GetScrollPosY() + 16.0f * 3;
			ScrollAtPosY(EnforceVerticalScrollLimits(NewScrollPosY));
		}
		else
		{
			SelectDownTimingEvent();
		}
		return FReply::Handled();
	}
	else if (InKeyEvent.GetKey() == EKeys::Enter)
	{
		// Enter: Selects the time range of the currently selected timing event.
		if (SelectedEvent.IsValid())
		{
			const double Duration = Viewport.RestrictDuration(SelectedEvent->GetStartTime(), SelectedEvent->GetEndTime());
			SelectTimeInterval(SelectedEvent->GetStartTime(), Duration);
		}
		return FReply::Handled();
	}
	else if (InKeyEvent.GetKey() == EKeys::SpaceBar)
	{
		bIsSpaceBarKeyPressed = true;
		FSlateApplication::Get().QueryCursor();
		return FReply::Handled();
	}
	else if (InKeyEvent.GetKey() == EKeys::D)
	{
		if (InKeyEvent.GetModifierKeys().IsControlDown() && InKeyEvent.GetModifierKeys().IsShiftDown())
		{
			// Ctrl+Shift+D: Toggles down-sampling on/off (for debugging purposes only).
			FTimingEventsTrack::bUseDownSampling = !FTimingEventsTrack::bUseDownSampling;
			Viewport.AddDirtyFlags(ETimingTrackViewportDirtyFlags::HInvalidated);
			return FReply::Handled();
		}
	}
	else if (InKeyEvent.GetKey() == EKeys::A)
	{
		if (InKeyEvent.GetModifierKeys().IsControlDown())
		{
			// Ctrl+A: Selects the entire time range of session.
			SelectTimeInterval(0, Viewport.GetMaxValidTime());
			return FReply::Handled();
		}
	}
	else if (InKeyEvent.GetKey() == EKeys::F8)
	{
		Jump2NextErrorOrWarning();
		return FReply::Handled();
	}
	return SCompoundWidget::OnKeyDown(MyGeometry, InKeyEvent);
}

FReply SBuildProgressView::OnKeyUp(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent)
{
	if (InKeyEvent.GetKey() == EKeys::SpaceBar)
	{
		bIsSpaceBarKeyPressed = false;
		FSlateApplication::Get().QueryCursor();
		return FReply::Handled();
	}

	return SCompoundWidget::OnKeyUp(MyGeometry, InKeyEvent);
}

void SBuildProgressView::InvalidateScrollableTracksOrder()
{
	bScrollableTracksOrderIsDirty = true;
}

void SBuildProgressView::AddTrack(const TSharedPtr<FBaseTimingTrack> Track, const ETimingTrackLocation Location)
{
	check(Track.IsValid());

	check(Location == ETimingTrackLocation::Scrollable ||
		  Location == ETimingTrackLocation::TopDocked ||
		  Location == ETimingTrackLocation::BottomDocked ||
		  Location == ETimingTrackLocation::Foreground);

	const TCHAR* LocationName = GetLocationName(Location);
	TArray<TSharedPtr<FBaseTimingTrack>>& TrackList = const_cast<TArray<TSharedPtr<FBaseTimingTrack>>&>(GetTrackList(Location));

	constexpr int32 MaxNumTracks = 8192;
	if (TrackList.Num() >= MaxNumTracks)
	{
		XIAO_LOG(Fatal, TEXT("Too many tracks already created (%d tracks)! Ignoring %s track : %s (\"%s\")"),
			TrackList.Num(),
			LocationName,
			*Track->GetTypeName().ToString(),
			*Track->GetName());
		return;
	}

	ensure(Track->GetLocation() == ETimingTrackLocation::None);
	Track->SetLocation(Location);

	check(!AllTracks.Contains(Track->GetId()));
	AllTracks.Add(Track->GetId(), Track);

	TrackList.Add(Track);
	Algo::SortBy(TrackList, &FBaseTimingTrack::GetOrder);

	if (Location == ETimingTrackLocation::Scrollable)
	{
		InvalidateScrollableTracksOrder();
	}
}

bool SBuildProgressView::RemoveTrack(const TSharedPtr<FBaseTimingTrack> Track)
{
	check(Track.IsValid());

	if (AllTracks.Remove(Track->GetId()) > 0)
	{
		const ETimingTrackLocation Location = Track->GetLocation();
		check(Location == ETimingTrackLocation::Scrollable ||
			  Location == ETimingTrackLocation::TopDocked ||
			  Location == ETimingTrackLocation::BottomDocked ||
			  Location == ETimingTrackLocation::Foreground);

		Track->SetLocation(ETimingTrackLocation::None);

		const TCHAR* LocationName = GetLocationName(Location);
		TArray<TSharedPtr<FBaseTimingTrack>>& TrackList = const_cast<TArray<TSharedPtr<FBaseTimingTrack>>&>(GetTrackList(Location));

		TrackList.Remove(Track);

		if (Location == ETimingTrackLocation::Scrollable)
		{
			InvalidateScrollableTracksOrder();
		}

		return true;
	}
	return false;
}

TSharedPtr<FBaseTimingTrack> SBuildProgressView::FindTrack(const uint64 InTrackId)
{
	TSharedPtr<FBaseTimingTrack>* TrackPtrPtr = AllTracks.Find(InTrackId);
	return TrackPtrPtr ? *TrackPtrPtr : nullptr;
}

#if (ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >=6)
void SBuildProgressView::EnumerateTracks(TFunctionRef<void(TSharedPtr<FBaseTimingTrack>Track)> Callback)
{
	for (auto& Entry : AllTracks)
	{
		Callback(Entry.Value);
	}
}
#endif

double SBuildProgressView::GetTimeMarker() const
{
	return SessionTime;
}

void SBuildProgressView::SetTimeMarker(const double InTimeMarker)
{
	// DefaultTimeMarker->SetTime(InTimeMarker);
	// RaiseTimeMarkerChanged(DefaultTimeMarker);
}

void SBuildProgressView::SetAndCenterOnTimeMarker(double InTimeMarker)
{
}

Insights::FTrackVisibilityChangedDelegate& SBuildProgressView::OnTrackVisibilityChanged()
{
	return OnTrackVisibilityChangedDelegate;
}

Insights::FTrackAddedDelegate& SBuildProgressView::OnTrackAdded()
{
	return OnTrackAddedDelegate;
}

Insights::FTrackRemovedDelegate& SBuildProgressView::OnTrackRemoved()
{
	return OnTrackRemovedDelegate;
}

void SBuildProgressView::ResetSelectedEvent()
{
	if (SelectedEvent)
	{
		SelectedEvent.Reset();
		OnSelectedEventChanged();
	}
}

void SBuildProgressView::PreventThrottling()
{
	bPreventThrottling = true;
}

void SBuildProgressView::AddOverlayWidget(const TSharedRef<SWidget>& InWidget)
{
	if (ExtensionOverlay.IsValid())
	{
		ExtensionOverlay->AddSlot()
		[
			InWidget
		];
	}
}

void SBuildProgressView::SelectHoveredTimingTrack()
{
	SelectTimingTrack(HoveredTrack, true);
}

void SBuildProgressView::SelectTimingTrack(const TSharedPtr<FBaseTimingTrack> InTrack, const bool bBringTrackIntoView)
{
	if (SelectedTrack != InTrack)
	{
		SelectedTrack = InTrack;

		if (SelectedTrack.IsValid())
		{
			if (bBringTrackIntoView && SelectedTrack->GetLocation() == ETimingTrackLocation::Scrollable)
			{
				const TSharedPtr<FBaseTimingTrack> ParentTrack = SelectedTrack->GetParentTrack().Pin();
				if (ParentTrack.IsValid())
				{
					BringScrollableTrackIntoView(*ParentTrack);
				}
				else
				{
					BringScrollableTrackIntoView(*SelectedTrack);
				}
			}
		}

		OnSelectedTrackChangedDelegate.Broadcast(SelectedTrack);
	}
}

void SBuildProgressView::BringScrollableTrackIntoView(const FBaseTimingTrack& InTrack)
{
	if (ensure(InTrack.GetLocation() == ETimingTrackLocation::Scrollable))
	{
		const float TopScrollY = InTrack.GetPosY() + Viewport.GetScrollPosY() - Viewport.GetTopOffset() - Viewport.GetPosY();
		BringIntoViewY(TopScrollY, TopScrollY + InTrack.GetHeight());
	}
}

void SBuildProgressView::BringIntoViewY(const float InTopScrollY, const float InBottomScrollY)
{
	const float ScrollY = Viewport.GetScrollPosY();
	const float ScrollH = Viewport.GetScrollableAreaHeight();
	if (ScrollY > InTopScrollY)
	{
		ScrollAtPosY(InTopScrollY);
	}
	else if (ScrollY + ScrollH < InBottomScrollY)
	{
		ScrollAtPosY(FMath::Min(InTopScrollY, InBottomScrollY - ScrollH));
	}
}

void SBuildProgressView::SelectTimeInterval(const double IntervalStartTime, const double IntervalDuration)
{
	SelectionStartTime = IntervalStartTime;
	SelectionEndTime = IntervalStartTime + IntervalDuration;
	LastSelectionType = ESelectionType::TimeRange;
	RaiseSelectionChanged();
}

void SBuildProgressView::SelectToTimeMarker(const double InTimeMarker)
{
	const double TimeMarker = GetTimeMarker();
	if (TimeMarker < InTimeMarker)
	{
		SelectTimeInterval(TimeMarker, InTimeMarker - TimeMarker);
	}
	else
	{
		SelectTimeInterval(InTimeMarker, TimeMarker - InTimeMarker);
	}

	SetTimeMarker(InTimeMarker);
}

const TCHAR* SBuildProgressView::GetLocationName(const ETimingTrackLocation Location)
{
	switch (Location)
	{
		case ETimingTrackLocation::TopDocked:		return TEXT("Top Docked");
		case ETimingTrackLocation::BottomDocked:	return TEXT("Bottom Docked");
		default:									return nullptr;
	}
}

void SBuildProgressView::UpdatePositionForScrollableTracks()
{
	// Update the Y postion for the visible scrollable tracks.
	float ScrollableTrackPosY = Viewport.GetPosY() + Viewport.GetTopOffset() - Viewport.GetScrollPosY();
	for (TSharedPtr<FBaseTimingTrack>& TrackPtr : ScrollableTracks)
	{
		SetTrackPosY(TrackPtr, ScrollableTrackPosY);
		if (TrackPtr->IsVisible())
		{
			ScrollableTrackPosY += TrackPtr->GetHeight();
		}
	}
}

double SBuildProgressView::EnforceHorizontalScrollLimits(const double InStartTime)
{
	double NewStartTime = InStartTime;

	double MinT, MaxT;
	Viewport.GetHorizontalScrollLimits(MinT, MaxT);

	if (NewStartTime > MaxT)
	{
		NewStartTime = MaxT;
		OverscrollRight = 1.0f;
	}

	if (NewStartTime < MinT)
	{
		NewStartTime = MinT;
		OverscrollLeft = 1.0f;
	}

	return NewStartTime;
}

float SBuildProgressView::EnforceVerticalScrollLimits(const float InScrollPosY)
{
	float NewScrollPosY = InScrollPosY;

	const float Dy = Viewport.GetScrollHeight() - Viewport.GetScrollableAreaHeight();
	const float MinY = FMath::Min(Dy, 0.0f);

	if (const float MaxY = Dy - MinY; NewScrollPosY > MaxY)
	{
		NewScrollPosY = MaxY;
		OverscrollBottom = 1.0f;
	}

	if (NewScrollPosY < MinY)
	{
		NewScrollPosY = MinY;
		OverscrollTop = 1.0f;
	}

	return NewScrollPosY;
}

void SBuildProgressView::OnUserScrolled_Horizontal(const float InScrollOffset)
{
	Viewport.OnUserScrolled(HorizontalScrollBar, InScrollOffset);
}

void SBuildProgressView::OnUserScrolled_Vertical(const float InScrollOffset)
{
	Viewport.OnUserScrolledY(VerticalScrollBar, InScrollOffset);
}

void SBuildProgressView::RaiseSelectionChanging() const
{
	OnSelectionChangedDelegate.Broadcast(UE::Insights::Timing::ETimeChangedFlags::Interactive, SelectionStartTime, SelectionEndTime);
}

void SBuildProgressView::RaiseSelectionChanged() const
{
	OnSelectionChangedDelegate.Broadcast(UE::Insights::Timing::ETimeChangedFlags::None, SelectionStartTime, SelectionEndTime);
}

void SBuildProgressView::UpdateHoveredTimingEvent(const float InMousePosX, const float InMousePosY)
{
	const TSharedPtr<FBaseTimingTrack> NewHoveredTrack = GetTrackAt(InMousePosX, InMousePosY);
	if (NewHoveredTrack != HoveredTrack)
	{
		HoveredTrack = NewHoveredTrack;
		OnHoveredTrackChangedDelegate.Broadcast(HoveredTrack);
	}

	TSharedPtr<const ITimingEvent> NewHoveredEvent;
	if (HoveredTrack.IsValid())
	{
		FStopwatch Stopwatch;
		Stopwatch.Start();

		NewHoveredEvent = HoveredTrack->GetEvent(InMousePosX, InMousePosY, Viewport);

		Stopwatch.Stop();
		if (const double DT = Stopwatch.GetAccumulatedTime(); DT > 0.001)
		{
			XIAO_LOG(Log, TEXT("HoveredTrack [%g, %g] GetEvent: %.1f ms"), InMousePosX, InMousePosY, DT * 1000.0);
		}
	}

	if (NewHoveredEvent.IsValid())
	{
		if (!HoveredEvent.IsValid() || !NewHoveredEvent->Equals(*HoveredEvent))
		{
			FStopwatch Stopwatch;
			Stopwatch.Start();

			HoveredEvent = NewHoveredEvent;
			ensure(HoveredTrack == HoveredEvent->GetTrack() || HoveredTrack->GetChildTrack() == HoveredEvent->GetTrack());
			HoveredTrack->UpdateEventStats(const_cast<ITimingEvent&>(*HoveredEvent));

			Stopwatch.Update();
			const double T1 = Stopwatch.GetAccumulatedTime();

			HoveredTrack->InitTooltip(Tooltip, *HoveredEvent);

			Stopwatch.Update();
			const double T2 = Stopwatch.GetAccumulatedTime();

			OnHoveredEventChangedDelegate.Broadcast(HoveredEvent);

			Stopwatch.Update();
			const double T3 = Stopwatch.GetAccumulatedTime();
			if (T3 > 0.001)
			{
				XIAO_LOG(Log, TEXT("HoveredTrack [%g, %g] Tooltip: %.1f ms (%.1f + %.1f + %.1f)"),
					InMousePosX, InMousePosY, T3 * 1000.0, T1 * 1000.0, (T2 - T1) * 1000.0, (T3 - T2) * 1000.0);
			}
		}
		Tooltip.SetDesiredOpacity(1.0f);
	}
	else
	{
		if (HoveredEvent.IsValid())
		{
			HoveredEvent.Reset();
			OnHoveredEventChangedDelegate.Broadcast(HoveredEvent);
		}
		Tooltip.SetDesiredOpacity(0.0f);
	}
}

void SBuildProgressView::OnSelectedTimingEventChanged() const
{
	if (SelectedEvent.IsValid())
	{
		SelectedEvent->GetTrack()->UpdateEventStats(const_cast<ITimingEvent&>(*SelectedEvent));
		SelectedEvent->GetTrack()->OnEventSelected(*SelectedEvent);
	}
	OnSelectedEventChangedDelegate.Broadcast(SelectedEvent);
}

void SBuildProgressView::SelectLeftTimingEvent()
{
	if (SelectedEvent.IsValid())
	{
		const uint32 Depth = SelectedEvent->GetDepth();
		const double StartTime = SelectedEvent->GetStartTime();
		const double EndTime = SelectedEvent->GetEndTime();

		auto EventFilter = [Depth, StartTime, EndTime](const double EventStartTime, const double EventEndTime, const uint32 EventDepth)
		{
			return EventDepth == Depth
				&& (EventStartTime < StartTime || EventEndTime < EndTime);
		};

		const TSharedPtr<const ITimingEvent> LeftEvent = SelectedEvent->GetTrack()->SearchEvent(
			FTimingEventSearchParameters(0.0, StartTime, ETimingEventSearchFlags::SearchAll, EventFilter));

		if (LeftEvent.IsValid())
		{
			SelectedEvent = LeftEvent;
			BringIntoView(SelectedEvent->GetStartTime(), SelectedEvent->GetEndTime());
			OnSelectedTimingEventChanged();
		}
	}
}

void SBuildProgressView::SelectRightTimingEvent()
{
	if (SelectedEvent.IsValid())
	{
		const uint32 Depth = SelectedEvent->GetDepth();
		const double StartTime = SelectedEvent->GetStartTime();
		const double EndTime = SelectedEvent->GetEndTime();

		auto EventFilter = [Depth, StartTime, EndTime](const double EventStartTime, const double EventEndTime, const uint32 EventDepth)
		{
			return EventDepth == Depth && (EventStartTime > StartTime || EventEndTime > EndTime);
		};

		const TSharedPtr<const ITimingEvent> RightEvent = SelectedEvent->GetTrack()->SearchEvent(
			FTimingEventSearchParameters(EndTime, Viewport.GetMaxValidTime(), ETimingEventSearchFlags::StopAtFirstMatch, EventFilter));

		if (RightEvent.IsValid())
		{
			SelectedEvent = RightEvent;
			BringIntoView(SelectedEvent->GetStartTime(), SelectedEvent->GetEndTime());
			OnSelectedTimingEventChanged();
		}
	}
}

void SBuildProgressView::SelectUpTimingEvent()
{
	if (SelectedEvent.IsValid() &&
		SelectedEvent->GetDepth() > 0)
	{
		const uint32 Depth = SelectedEvent->GetDepth() - 1;
		const double StartTime = SelectedEvent->GetStartTime();
		const double EndTime = SelectedEvent->GetEndTime();

		auto EventFilter = [Depth, StartTime, EndTime](const double EventStartTime, const double EventEndTime, uint32 EventDepth)
		{
			return EventDepth == Depth
				&& EventStartTime <= EndTime
				&& EventEndTime >= StartTime;
		};

		const TSharedPtr<const ITimingEvent> UpEvent = SelectedEvent->GetTrack()->SearchEvent(
			FTimingEventSearchParameters(StartTime, EndTime, ETimingEventSearchFlags::StopAtFirstMatch, EventFilter));

		if (UpEvent.IsValid())
		{
			SelectedEvent = UpEvent;
			BringIntoView(SelectedEvent->GetStartTime(), SelectedEvent->GetEndTime());
			OnSelectedTimingEventChanged();
		}
	}
}

void SBuildProgressView::SelectDownTimingEvent()
{
	if (SelectedEvent.IsValid())
	{
		const uint32 Depth = SelectedEvent->GetDepth() + 1;
		const double StartTime = SelectedEvent->GetStartTime();
		const double EndTime = SelectedEvent->GetEndTime();
		double LargestDuration = 0.0;

		auto EventFilter = [Depth, StartTime, EndTime, &LargestDuration](const double EventStartTime, const double EventEndTime, const uint32 EventDepth)
		{
			const double Duration = EventEndTime - EventStartTime;
			return Duration > LargestDuration
				&& EventDepth == Depth
				&& EventStartTime <= EndTime
				&& EventEndTime >= StartTime;
		};

		auto EventMatched = [&LargestDuration](const double EventStartTime, const double EventEndTime, uint32 EventDepth)
		{
			const double Duration = EventEndTime - EventStartTime;
			LargestDuration = Duration;
		};

		const TSharedPtr<const ITimingEvent> DownEvent = SelectedEvent->GetTrack()->SearchEvent(
			FTimingEventSearchParameters(StartTime, EndTime, ETimingEventSearchFlags::SearchAll, EventFilter, EventMatched));

		if (DownEvent.IsValid())
		{
			SelectedEvent = DownEvent;
			BringIntoView(SelectedEvent->GetStartTime(), SelectedEvent->GetEndTime());
			OnSelectedTimingEventChanged();
		}
	}
}

void SBuildProgressView::RaiseTimeMarkerChanging(const TSharedRef<Xiao::FTimeMarker> InTimeMarker) const
{
	/*if (InTimeMarker == DefaultTimeMarker)
	{
		const double Time = DefaultTimeMarker->GetTime();
		OnTimeMarkerChangedDelegate.Broadcast(Insights::ETimeChangedFlags::Interactive, Time);
	}*/
	OnCustomTimeMarkerChangedDelegate.Broadcast(UE::Insights::Timing::ETimeChangedFlags::Interactive, InTimeMarker);
}

void SBuildProgressView::RaiseTimeMarkerChanged(const TSharedRef<Xiao::FTimeMarker> InTimeMarker) const
{
	/*if (InTimeMarker == DefaultTimeMarker)
	{
		const double Time = DefaultTimeMarker->GetTime();
		OnTimeMarkerChangedDelegate.Broadcast(Insights::ETimeChangedFlags::None, Time);
	}*/
	OnCustomTimeMarkerChangedDelegate.Broadcast(UE::Insights::Timing::ETimeChangedFlags::None, InTimeMarker);
}

const TSharedPtr<FBaseTimingTrack> SBuildProgressView::GetTrackAt(float InPosX, const float InPosY) const
{
	TSharedPtr<FBaseTimingTrack> FoundTrack;

	if (InPosY < Viewport.GetPosY())
	{
		// above viewport
	}
	else if (InPosY < Viewport.GetPosY() + Viewport.GetTopOffset())
	{
		// Top Docked Tracks
		for (const TSharedPtr<FBaseTimingTrack>& TrackPtr : TopDockedTracks)
		{
			const FBaseTimingTrack& Track = *TrackPtr;
			if (TrackPtr->IsVisible())
			{
				if (InPosY >= Track.GetPosY() && InPosY < Track.GetPosY() + Track.GetHeight())
				{
					FoundTrack = TrackPtr;
					break;
				}
			}
		}
	}
	else if (InPosY < Viewport.GetPosY() + Viewport.GetHeight() - Viewport.GetBottomOffset())
	{
		// Scrollable Tracks
		for (const TSharedPtr<FBaseTimingTrack>& TrackPtr : ScrollableTracks)
		{
			const FBaseTimingTrack& Track = *TrackPtr;
			if (Track.IsVisible())
			{
				if (InPosY >= Track.GetPosY() && InPosY < Track.GetPosY() + Track.GetHeight())
				{
					FoundTrack = TrackPtr;
					break;
				}
			}
		}
	}

	return FoundTrack;
}

void SBuildProgressView::UpdateHorizontalScrollBar() const
{
	Viewport.UpdateScrollBar(HorizontalScrollBar);
}

void SBuildProgressView::Jump2NextErrorOrWarning()
{
	uint32 Index = 0;
	for (const auto& Iter : ProcessMap)
	{
		if (Iter.Value)
		{
			const int32 PrcessNum = Iter.Value->GetProcesses().Num();
			if (Index + PrcessNum > ErrorIndex)
			{
				for (int32 InnerIndex = (ErrorIndex>=Index) ? (ErrorIndex-Index) : 0; InnerIndex < PrcessNum; ++InnerIndex)
				{
					++Index;
					const auto& Process = Iter.Value->GetProcesses()[InnerIndex];
					if (Index > ErrorIndex && (Process.exitCode != 0 || !Process.returnedReason.IsEmpty()))
					{
						ErrorIndex = Index;
						SelectTimingTrack(SharedThis(Iter.Value), true);
						FTimingEventSearchParameters SearchParams(FPlatformTime::ToSeconds64(Process.start + 1), FPlatformTime::ToSeconds64(Process.stop - 1), ETimingEventSearchFlags::StopAtFirstMatch);
						SelectedEvent = Iter.Value->SearchEvent(SearchParams);
						if (SelectedEvent.IsValid())
						{
							BringIntoView(SelectedEvent->GetStartTime(), SelectedEvent->GetEndTime());
							OnSelectedTimingEventChanged();
							if (const FProcessTrackEvent* ProcessTrack = static_cast<const FProcessTrackEvent*>(SelectedEvent.Get()))
							{
								const FName Identifier(FString::FromInt(ProcessTrack->GetProcessDesc().id));
								OnEventClicked.ExecuteIfBound(Identifier);
							}
						}
						return;
					}
				}
			}
			else
			{
				Index += PrcessNum;
			}
		}
	}
	FXiaoStyle::DoModel(LOCTEXT("NoErrorOrEnd_Text", "没有错误发现"), true);
	ErrorIndex = 0;
}

void SBuildProgressView::UpdateVerticalScrollBar() const
{
	Viewport.UpdateScrollBarY(VerticalScrollBar);
}

void SBuildProgressView::ScrollAtPosY(const float ScrollPosY)
{
	if (ScrollPosY != Viewport.GetScrollPosY())
	{
		Viewport.SetScrollPosY(ScrollPosY);
		UpdateVerticalScrollBar();
	}
}

void SBuildProgressView::ScrollAtTime(const double StartTime)
{
	if (Viewport.ScrollAtTime(StartTime))
	{
		UpdateHorizontalScrollBar();
	}
}

void SBuildProgressView::SetTrackPosY(const TSharedPtr<FBaseTimingTrack>& TrackPtr, const float TrackPosY) const
{
	TrackPtr->SetPosY(TrackPosY);
	if (TrackPtr->GetChildTrack().IsValid())
	{
		TrackPtr->GetChildTrack()->SetPosY(TrackPosY + this->GetViewport().GetLayout().TimelineDY + 1.0f);
	}
}

void SBuildProgressView::BringIntoView(double StartTime, double EndTime)
{
	EndTime = Viewport.RestrictEndTime(EndTime);

	// Increase interval with 8% (of view size) on each side.
	const double DT = Viewport.GetDuration() * 0.08;
	StartTime -= DT;
	EndTime += DT;

	double NewStartTime = Viewport.GetStartTime();

	if (EndTime > Viewport.GetEndTime())
	{
		NewStartTime += EndTime - Viewport.GetEndTime();
	}

	if (StartTime < NewStartTime)
	{
		NewStartTime = StartTime;
	}

	ScrollAtTime(NewStartTime);
}

FReply SBuildProgressView::AllowTracksToProcessOnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	for (const TSharedPtr<FBaseTimingTrack>& TrackPtr : TopDockedTracks)
	{
		if (TrackPtr->IsVisible())
		{
			FReply Reply = TrackPtr->OnMouseButtonDown(MyGeometry, MouseEvent);
			if (Reply.IsEventHandled())
			{
				return Reply;
			}
		}
	}

	return FReply::Unhandled();
}

FReply SBuildProgressView::AllowTracksToProcessOnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	for (const TSharedPtr<FBaseTimingTrack>& TrackPtr : TopDockedTracks)
	{
		if (TrackPtr->IsVisible())
		{
			FReply Reply = TrackPtr->OnMouseButtonUp(MyGeometry, MouseEvent);
			if (Reply.IsEventHandled())
			{
				return Reply;
			}
		}
	}

	return FReply::Unhandled();
}

FReply SBuildProgressView::AllowTracksToProcessOnMouseButtonDoubleClick(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	for (const TSharedPtr<FBaseTimingTrack>& TrackPtr : TopDockedTracks)
	{
		if (TrackPtr->IsVisible())
		{
			FReply Reply = TrackPtr->OnMouseButtonDoubleClick(MyGeometry, MouseEvent);
			if (Reply.IsEventHandled())
			{
				return Reply;
			}
		}
	}

	for (TSharedPtr<FBaseTimingTrack>& TrackPtr : ScrollableTracks)
	{
		if (TrackPtr->IsVisible())
		{
			FReply Reply = TrackPtr->OnMouseButtonDoubleClick(MyGeometry, MouseEvent);
			if (Reply.IsEventHandled())
			{
				return Reply;
			}
		}
	}

	return FReply::Unhandled();
}

void SBuildProgressView::ShowContextMenu(const FPointerEvent& MouseEvent)
{
	FMenuBuilder MenuBuilder(true, nullptr);
	
	if(HoveredTrack.IsValid())
	{
		MenuBuilder.BeginSection(TEXT("Track"), FText::FromString(HoveredTrack->GetName()));
		MenuBuilder.EndSection();

		HoveredTrack->BuildContextMenu(MenuBuilder);
		MenuBuilder.AddSeparator();
	}
	MenuBuilder.SetSearchable(false);
	
	const TSharedRef<SWidget> MenuWidget = MenuBuilder.MakeWidget();

	const FWidgetPath EventPath = MouseEvent.GetEventPath() != nullptr ? *MouseEvent.GetEventPath() : FWidgetPath();
	const FVector2D ScreenSpacePosition = MouseEvent.GetScreenSpacePosition();
	FSlateApplication::Get().PushMenu(SharedThis(this), EventPath, MenuWidget, ScreenSpacePosition, FPopupTransitionEffect::ContextMenu);
}

#undef ACTIVATE_BENCHMARK
#undef LOCTEXT_NAMESPACE