/**
  * @author cxx2020@outlook.com
  * @date 10:26 PM
 */

#pragma once
#include "CoreMinimal.h"
#include "Insights/ViewModels/BaseTimingTrack.h"
#include "UbaTraceReader.h"
	
class FSessionDetailsTrack final : public FBaseTimingTrack
{
	INSIGHTS_DECLARE_RTTI(FSessionDetailsTrack, FBaseTimingTrack)
public:
	FSessionDetailsTrack();

	void LoadFromSession(const Xiao::FTraceView& InView, const Xiao::FTraceView::FSession& InSession, const bool bIsRemote);
	void UpdateDetails(const Xiao::FTraceView& InView, const Xiao::FTraceView::FSession& InSession);

	virtual void Draw(const ITimingTrackDrawContext& Context) const override;

	const Xiao::FTraceView* GetTraceView() const;
	Xiao::FTraceView::FSession* GetSession() const;
	bool IsRemote() const { return bRemote; }

public:
	inline static constexpr double DetailsTrackHeight = 70.0f;

private:
	Xiao::FTraceView* TraceView = nullptr;
	Xiao::FTraceView::FSession* Session = nullptr;
	bool bRemote = false;
};
