/**
  * @author cxx2020@outlook.com
  * @date 3:12 PM
 */

#pragma once

#include "CoreMinimal.h"
#include "Insights/ITimingViewExtender.h"
#include "Insights/ViewModels/TimingEventsTrack.h"
#include "Insights/ViewModels/TimingEvent.h"
#include "InsightsCore/Common/SimpleRtti.h"

#include "ShareDefine.h"

#include "UbaTraceReader.h"

class SBuildProgressView;
class FProcessTrackEvent;

class FProcessTimingSharedState final : public UE::Insights::Timing::ITimingViewExtender, public TSharedFromThis<FProcessTimingSharedState>
{
public:
	explicit FProcessTimingSharedState(SBuildProgressView* InProgressView) : ProgressView(InProgressView) {}
	virtual ~FProcessTimingSharedState() override = default;

	void Reset(const UE::Insights::Timing::ITimingViewSession& InSession);

	const TMap<uint32, TSharedPtr<FProcessTrackEvent>>& GetAllTracks() { return AllTracks; }

	// ~ ITimingViewExtender interface Begin
	virtual void OnBeginSession(UE::Insights::Timing::ITimingViewSession& InSession) override;
	virtual void OnEndSession(UE::Insights::Timing::ITimingViewSession& InSession) override;
	virtual void Tick(UE::Insights::Timing::ITimingViewSession& InSession, const TraceServices::IAnalysisSession& InAnalysisSession) override;
	// ~ ITimingViewExtender interface End

private:
	SBuildProgressView* ProgressView = nullptr;

	TMap<uint32, TSharedPtr<FProcessTrackEvent>> AllTracks;

	bool bShowHideAllTracks = true;

	uint64 TimingProfilerTimelineCount = 0;
	uint64 LoadTimeProfilerTimelineCount = 0;
};

class FProcessTrackEvent final : public FTimingEvent
{
	INSIGHTS_DECLARE_RTTI(FProcessTrackEvent, FTimingEvent)

public:
	FProcessTrackEvent(const TSharedRef<const FBaseTimingTrack> InTrack, const Xiao::FTraceView::FProcess& InDesc, const uint64 LastTime, const uint32 InDepth)
	: FTimingEvent(InTrack, FPlatformTime::ToSeconds64(InDesc.start), FPlatformTime::ToSeconds64(InDesc.stop != ~uint64(0) ? InDesc.stop : LastTime), InDepth)
	, Process(InDesc)
	{}
	virtual ~FProcessTrackEvent() override {}

	const Xiao::FTraceView::FProcess& GetProcessDesc() const { return Process; }

private:
	Xiao::FTraceView::FProcess Process;
};

class FProcessTimingTrack final : public FTimingEventsTrack
{
	INSIGHTS_DECLARE_RTTI(FProcessTimingTrack, FTimingEventsTrack)
public:
	explicit FProcessTimingTrack(const FString& InName, const FString& InUniqueName, Xiao::FTraceView* InTraceView, const TArray<Xiao::FTraceView::FProcess>& InProcesses)
		: FTimingEventsTrack(InName)
		, UniqueName(*InUniqueName)
		, TraceView(InTraceView)
		, Processes(InProcesses)
	{}

	virtual ~FProcessTimingTrack() override;

	// virtual void BuildContextMenu(FMenuBuilder& MenuBuilder) override;
	virtual void BuildDrawState(ITimingEventsTrackDrawStateBuilder& Builder, const ITimingTrackUpdateContext& Context) override;
	virtual const TSharedPtr<const ITimingEvent> SearchEvent(const FTimingEventSearchParameters& InSearchParameters) const override;
	virtual void InitTooltip(FTooltipDrawState& InOutTooltip, const ITimingEvent& InTooltipEvent) const override;
	virtual FReply OnMouseButtonDoubleClick(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;

	const FString& GetUniqueName() const { return UniqueName; }
	void UpdateProcessor(const TArray<Xiao::FTraceView::FProcess>& InProcesses, const uint64 InLastTime);
	const TArray<Xiao::FTraceView::FProcess>& GetProcesses() const { return Processes; }

	inline static constexpr double ProcessTrackHeight = 20.0f;
	
private:
	FString UniqueName;
	uint64 LastTime = 0.0f;
	Xiao::FTraceView* TraceView;
	TArray<Xiao::FTraceView::FProcess> Processes;
	TMap<int32, TSharedPtr<struct FProcessStats>> StatsMap;
};
