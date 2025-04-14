#include "FSessionDetailsTrack.h"
#include "../Helpers/TimingViewDrawHelper.h"

INSIGHTS_IMPLEMENT_RTTI(FSessionDetailsTrack)

FSessionDetailsTrack::FSessionDetailsTrack()
	: FBaseTimingTrack()
{
}

void FSessionDetailsTrack::LoadFromSession(const Xiao::FTraceView& InView, const Xiao::FTraceView::FSession& InSession, const bool bIsRemote)
{
	TraceView = const_cast<Xiao::FTraceView*>(&InView);
	Session = const_cast<Xiao::FTraceView::FSession*>(&InSession);
	bRemote = bIsRemote;
}

void FSessionDetailsTrack::UpdateDetails(const Xiao::FTraceView& InView, const Xiao::FTraceView::FSession& InSession)
{
	TraceView = const_cast<Xiao::FTraceView*>(&InView);
	Session = const_cast<Xiao::FTraceView::FSession*>(&InSession);
}

void FSessionDetailsTrack::Draw(const ITimingTrackDrawContext& Context) const
{
	FBaseTimingTrack::Draw(Context);

	if (const Xiao::FTimingViewDrawHelper* Helper = static_cast<const Xiao::FTimingViewDrawHelper*>(&Context.GetHelper()))
	{
		Helper->BeginDrawTracks();
		Helper->DrawGraphDetails(*this);
		Helper->EndDrawTracks();
	}
}

const Xiao::FTraceView* FSessionDetailsTrack::GetTraceView() const
{
	return TraceView;
}

Xiao::FTraceView::FSession* FSessionDetailsTrack::GetSession() const
{
	return Session;
}
