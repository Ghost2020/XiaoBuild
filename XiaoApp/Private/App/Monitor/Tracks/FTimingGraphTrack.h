/**
  * @author cxx2020@outlook.com
  * @date 10:26 PM
 */

#pragma once
#include "CoreMinimal.h"
#include "Insights/ViewModels/GraphSeries.h"
#include "Insights/ViewModels/GraphTrack.h"
#include "ShareDefine.h"
#include "UbaTraceReader.h"

class FXiaoTimingGraphTrack;

DECLARE_DELEGATE_TwoParams(FOnSeriesVisibilityChanged, FXiaoTimingGraphTrack*, const class FXiaoTimingGraphSeries*)
inline FOnSeriesVisibilityChanged GOnSeriesVisibilityChanged;

class FXiaoTimingGraphSeries final : public FGraphSeries
{
public:
	explicit FXiaoTimingGraphSeries(FXiaoTimingGraphTrack* InTrack, const ESeriesType InType, const uint32 InGraphId);

	virtual void SetVisibility(bool bOnOff) override
	{
		if (FPlatformTime::Seconds() - LastTrigger > 0.1f)
		{
			LastTrigger = FPlatformTime::Seconds();
			FGraphSeries::SetVisibility(bOnOff);
			GOnSeriesVisibilityChanged.ExecuteIfBound(ParentTrack, this);
		}
	}
	virtual FString FormatValue(double Value) const override;

	double GetMinValue() const { return MinValue; }
	double GetMaxValue() const { return MaxValue; }
	uint32 GetGraphId() const { return GraphId; }
	void SetValueRange(const double Min, const double Max) { MinValue = Min; MaxValue = Max; }
	void SetMax(const double InMax) { MaxValue = RealMaxValue = InMax; }
	double GetRealMax() const { return RealMaxValue; }

	ESeriesType Type;
	uint32 GraphId = 0;
	double LastTrigger = 0.0f;
private:
	FXiaoTimingGraphTrack* ParentTrack = nullptr;
	double MinValue = 0.0;
	double MaxValue = 0.0;
	double RealMaxValue = 0.0;
};

struct FRawSystemGraph
{
	FString Name;
	TArray<double> Times;
	TArray<double> Durations;
	TArray<double> Values;
	double MaxValue;
};

DECLARE_DELEGATE_OneParam(FOnOptionsChanged, const FXiaoTimingGraphTrack*)
inline FOnOptionsChanged GOnOptionsChanged;
	
class FXiaoTimingGraphTrack final : public FGraphTrack
{
	INSIGHTS_DECLARE_RTTI(FXiaoTimingGraphTrack, FGraphTrack)
public:
	FXiaoTimingGraphTrack();
	virtual ~FXiaoTimingGraphTrack() override;

	bool LoadFromSession(const Xiao::FTraceView::FSession& InSession, const FTimingTrackViewport& InViewport, double& OutLastTime);
	
	void UpdateGraph(const Xiao::FTraceView::FSession& InSession);

	virtual void Update(const ITimingTrackUpdateContext& Context) override;
	virtual void BuildContextMenu(FMenuBuilder& MenuBuilder) override;
	virtual void ContextMenu_ToggleOption_Execute(EGraphOptions Option) override;
	virtual void Draw(const ITimingTrackDrawContext& Context) const override;
	
	TSharedPtr<FXiaoTimingGraphSeries> GetGraphSeries(const uint32 InGraphId);
	void RemoveSeries(const uint32 InGraphId);
	
protected:
	void UpdateSeries(FXiaoTimingGraphSeries& Series, const FTimingTrackViewport& Viewport);

public:
	inline static constexpr double GraphTrackHeight = 60.0f;

private:
	TArray<FRawSystemGraph> RawSystemGraph;
};
