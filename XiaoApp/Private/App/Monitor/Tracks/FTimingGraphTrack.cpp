/**
  * @author cxx2020@outlook.com
  * @date 3:12 PM
 */
#include "FTimingGraphTrack.h"
#include "Algo/MaxElement.h"
#include "Algo/ForEach.h"
#include "InsightsCore/Common/TimeUtils.h"
#include "Insights/ViewModels/GraphTrackBuilder.h"
#include "Insights/ViewModels/TimingTrackViewport.h"
#include "TraceServices/Model/AnalysisSession.h"
#include "../Helpers/TimingViewDrawHelper.h"
#include "XiaoLog.h"
#include "ShareDefine.h"


static constexpr double SGegaByte = 1024.0f * 1024.0f * 1024.0f;

FXiaoTimingGraphSeries::FXiaoTimingGraphSeries(FXiaoTimingGraphTrack* InTrack, const ESeriesType InType, const uint32 InGraphId)
	: FGraphSeries()
	, Type(InType)
	, GraphId(InGraphId)
	, ParentTrack(InTrack)
{}

FString FXiaoTimingGraphSeries::FormatValue(const double Value) const
{
	const double RealValue = (Value / FXiaoTimingGraphTrack::GraphTrackHeight) * RealMaxValue;
	switch (Type)
	{
	case ESeriesType::Percent: return FString::Printf(TEXT("%.2f %%"), RealValue*100.0);
	case ESeriesType::Interrupts: return FString::Printf(TEXT("%.2f Interrupts/Sec"), RealValue);
	case ESeriesType::Memory: return FString::Printf(TEXT("%.2f GB"), RealValue);
	case ESeriesType::Faults: return FString::Printf(TEXT("%.2f Faults/Sec"), RealValue);
	case ESeriesType::CPUs: return FString::Printf(TEXT("%ld CPUs"), static_cast<uint32>(RealValue));
	case ESeriesType::GHz: return FString::Printf(TEXT("%.2f GHz"), RealValue);
	case ESeriesType::Tasks: return FString::Printf(TEXT("%ld Tasks"), static_cast<uint32>(RealValue));
	case ESeriesType::Secs: return FString::Printf(TEXT("%.2f /Sec"), RealValue);
	case ESeriesType::Files: return FString::Printf(TEXT("%.2f files"), RealValue);
	case ESeriesType::TransferSpeed: return FString::Printf(TEXT("%.2f mpbs"), RealValue/1024.0f/1024.0f);
	default:
		const int64 Int64Value = static_cast<int64>(Value);
		return FText::AsNumber(Int64Value).ToString();
	}
}

static void BuildGraph(const TArray<Xiao::FTraceView::FSessionUpdate>& InSession, const FString& InFieldName, FRawSystemGraph& OutGraph)
{
	OutGraph.Name = InFieldName;
	uint32 Index = 0;
	for (const auto& Update : InSession)
	{
		if (InFieldName == SAccumulateTime)
		{
			OutGraph.Values.Add(FPlatformTime::ToSeconds64(Update.time));
		}
		else if (InFieldName == SSend)
		{
			OutGraph.Values.Add((Index >= 1) ? ((Update.send - InSession[Index - 1].send) / OutGraph.Durations[Index]) : 0.0f);
		}
		else if (InFieldName == SReceive)
		{
			OutGraph.Values.Add((Index >= 1) ? ((Update.recv- InSession[Index-1].recv) / OutGraph.Durations[Index]) : 0.0f);
		}
		else if (InFieldName == SPing)
		{
			OutGraph.Values.Add(Update.ping);
		}
		else if (InFieldName == SMemAvail)
		{
			OutGraph.Values.Add(double(Update.memAvail) / SGegaByte);
		}
		else if (InFieldName == SCpuUsage)
		{
			OutGraph.Values.Add(Update.cpuLoad);
		}
		else if (InFieldName == SConnectionCount)
		{
			OutGraph.Values.Add(Update.connectionCount);
		}
		++Index;
	}
}


INSIGHTS_IMPLEMENT_RTTI(FXiaoTimingGraphTrack)

using namespace Xiao;

FXiaoTimingGraphTrack::FXiaoTimingGraphTrack()
	: FGraphTrack()
{
	EnabledOptions = static_cast<EGraphOptions>(GMonitorSettings.EnabledOptions);
	VisibleOptions = static_cast<EGraphOptions>(GMonitorSettings.VisibleOptions);
}

FXiaoTimingGraphTrack::~FXiaoTimingGraphTrack()
{
	GMonitorSettings.EnabledOptions = static_cast<int>(EnabledOptions);
	GMonitorSettings.VisibleOptions = static_cast<int>(VisibleOptions);
	for (const auto& Series : GetSeries())
	{
		GMonitorSettings.ShowMap.Add(Series->GetName().ToString(), Series->IsVisible());
	}

	GMonitorSettings.Save();
}

bool FXiaoTimingGraphTrack::LoadFromSession(const Xiao::FTraceView::FSession& InSession, const FTimingTrackViewport& InViewport, double& OutLastTime)
{
	AllSeries.Reset();
	RawSystemGraph.Reset();

	uint32 GraphId = 0;
	FRawSystemGraph TimeGraph; 
	BuildGraph(InSession.updates, SAccumulateTime, TimeGraph);
	double LastTime = 0.0f;
	for (const double Time : TimeGraph.Values)
	{
		TimeGraph.Durations.Add(Time-LastTime);
		LastTime = Time;
		OutLastTime = OutLastTime < Time ? Time : OutLastTime;
	}
	for(const FString& GraphName : AllGraphNames)
	{
		FRawSystemGraph Graph;
		Graph.Times = TimeGraph.Values;
		Graph.Durations = TimeGraph.Durations;
		BuildGraph(InSession.updates, GraphName, Graph);

		const auto& GraphDesc = SGraphMap[GraphName];
		TSharedRef<FXiaoTimingGraphSeries> Series = MakeShared<FXiaoTimingGraphSeries>(this, GraphDesc.Type, GraphId++);
		Series->SetVisibility(true);
		Series->SetName(GraphName);
		Series->SetDescription(GraphDesc.Desc);
		Series->SetColor(GraphDesc.Color, GraphDesc.BorderColor);
		Series->SetBaselineY(FXiaoTimingGraphTrack::GraphTrackHeight);
		Series->SetScaleY(SharedValueViewport.GetScaleY());
		Series->SetMax(GraphDesc.MaxVal);
		if (GraphName == SMemAvail)
		{
			const double MaxVal = double(InSession.memTotal) / SGegaByte;
			Algo::ForEach(Graph.Values, [&](double& InVal) { InVal = MaxVal - InVal; });
			Series->SetMax(MaxVal);
		}
		else if (GraphName == SSend)
		{
			Series->SetMax(InSession.highestSendPerS);
		}
		else if (GraphName == SReceive)
		{
			Series->SetMax(InSession.highestRecvPerS);
		}
		RawSystemGraph.Add(Graph);

		FGraphTrackBuilder Builder(*this, Series.Get(), InViewport);
		for(int Index = 0; Index < Graph.Times.Num(); ++Index)
		{
			Builder.AddEvent(Graph.Times[Index], Graph.Durations[Index], Graph.Values[Index]);
		}
		AllSeries.Add(Series);
	}
	return true;
}

void FXiaoTimingGraphTrack::Update(const ITimingTrackUpdateContext& Context)
{
	FGraphTrack::Update(Context);

	const bool bIsEntireGraphTrackDirty = IsDirty() || Context.GetViewport().IsHorizontalViewportDirty();
	bool bNeedsUpdate = bIsEntireGraphTrackDirty;

	if (!bNeedsUpdate)
	{
		for (const TSharedPtr<FGraphSeries>& Series : AllSeries)
		{
			if (Series->IsVisible() && Series->IsDirty())
			{
				bNeedsUpdate = true;
				break;
			}
		}
	}

	if (bNeedsUpdate)
	{
		ClearDirtyFlag();

		NumAddedEvents = 0;

		const FTimingTrackViewport& Viewport = Context.GetViewport();

		for (TSharedPtr<FGraphSeries>& Series : AllSeries)
		{
			if (Series->IsVisible() && (bIsEntireGraphTrackDirty || Series->IsDirty()))
			{
				// Clear the flag before updating, because the update itself may further need to set the series as dirty.
				Series->ClearDirtyFlag();

				if(TSharedPtr<FXiaoTimingGraphSeries> TimingSeries = StaticCastSharedPtr<FXiaoTimingGraphSeries>(Series))
				{
					UpdateSeries(*TimingSeries, Viewport);
				}
			}
		}

		UpdateStats();
	}
}

void FXiaoTimingGraphTrack::BuildContextMenu(FMenuBuilder& MenuBuilder)
{
	FGraphTrack::BuildContextMenu(MenuBuilder);
}

void FXiaoTimingGraphTrack::ContextMenu_ToggleOption_Execute(EGraphOptions Option)
{
	FGraphTrack::ContextMenu_ToggleOption_Execute(Option);
	OnOptionsChanged.ExecuteIfBound(this);
}

void FXiaoTimingGraphTrack::UpdateGraph(const Xiao::FTraceView::FSession& InSession)
{
	const auto& Updates = InSession.updates;
	double LastTime = 0.0f;
	if (RawSystemGraph[0].Times.Num() > 0)
	{
		LastTime = RawSystemGraph[0].Times.Last();
	}
	for(int Index = RawSystemGraph[0].Times.Num(); Index < Updates.Num(); Index++)
	{
		const auto& Update = Updates[Index];
		const double Time = FPlatformTime::ToSeconds64(Update.time);
		if (Time > LastTime)
		{
			// Send
			auto& Graph0 = RawSystemGraph[0];
			const double duration = Time - LastTime;
			Graph0.Times.Add(Time);
			Graph0.Values.Add((Index >= 1) ? ((Update.send - Graph0.Values.Last()) / duration) : 0.0f);
			Graph0.Durations.Add(duration);
			if (Graph0.Values.Last() > Graph0.MaxValue)
			{
				Graph0.MaxValue = Graph0.Values.Last();
				if (Graph0.MaxValue > 100)
				{
					GetGraphSeries(0)->SetMax(Graph0.MaxValue);
				}
			}

			// Recv
			auto& Graph1 = RawSystemGraph[1];
			Graph1.Times.Add(Time);
			Graph1.Values.Add((Index >= 1) ? ((Update.recv - Graph1.Values.Last()) / duration) : 0.0f);
			Graph1.Durations.Add(duration);
			if (Graph1.Values.Last() > Graph1.MaxValue)
			{
				Graph1.MaxValue = Graph1.Values.Last();
				if (Graph1.MaxValue > 100)
				{
					GetGraphSeries(1)->SetMax(Graph1.MaxValue);
				}
			}

			// Ping
			auto& Graph2 = RawSystemGraph[2];
			Graph2.Times.Add(Time);
			Graph2.Values.Add(Update.ping);
			Graph2.Durations.Add(duration);
			if (Update.ping > Graph2.MaxValue)
			{
				Graph2.MaxValue = Update.ping;
				GetGraphSeries(2)->SetMax(Graph2.MaxValue);
			}

			// Mem
			auto& Graph3 = RawSystemGraph[3];
			Graph3.Times.Add(Time);
			const double CurMem = double(Update.memAvail / SGegaByte);
			Graph3.Values.Add(CurMem);
			Graph3.Durations.Add(duration);
			if (CurMem > Graph3.MaxValue)
			{
				Graph3.MaxValue = CurMem;
				GetGraphSeries(3)->SetMax(Graph3.MaxValue);
			}

			// Cpu
			auto& Graph4 = RawSystemGraph[4];
			Graph4.Times.Add(Time);
			Graph4.Values.Add(Update.cpuLoad);
			Graph4.Durations.Add(duration);

			// Connection
			auto& Graph5 = RawSystemGraph[5];
			Graph5.Times.Add(Time);
			Graph5.Values.Add(Update.connectionCount);
			Graph5.Durations.Add(duration);
			if (Update.connectionCount > Graph5.MaxValue)
			{
				Graph5.MaxValue = Update.connectionCount;
			}
		}
	}

	for (auto& Serie : AllSeries)
	{
		Serie->SetDirtyFlag();
	}

	SetDirtyFlag();
}

void FXiaoTimingGraphTrack::Draw(const ITimingTrackDrawContext& Context) const
{
	FGraphTrack::Draw(Context);

	if (const Xiao::FTimingViewDrawHelper* Helper = static_cast<const Xiao::FTimingViewDrawHelper*>(&Context.GetHelper()))
	{
		Helper->BeginDrawTracks();
		Helper->DrawTrackHeader(*this);
		Helper->EndDrawTracks();
	}
}

TSharedPtr<FXiaoTimingGraphSeries> FXiaoTimingGraphTrack::GetGraphSeries(const uint32 InGraphId)
{
	const TSharedPtr<FGraphSeries>* Ptr = AllSeries.FindByPredicate([InGraphId](const TSharedPtr<FGraphSeries>& Series)
	{
		const TSharedPtr<FXiaoTimingGraphSeries> TimingSeries = StaticCastSharedPtr<FXiaoTimingGraphSeries>(Series);
		return TimingSeries->GetGraphId() == InGraphId;
	});
	return (Ptr != nullptr) ? StaticCastSharedPtr<FXiaoTimingGraphSeries>(*Ptr) : nullptr;
}

void FXiaoTimingGraphTrack::RemoveSeries(const uint32 InGraphId)
{
	AllSeries.RemoveAll([InGraphId](const TSharedPtr<FGraphSeries>& Series)
	{
		const TSharedPtr<FXiaoTimingGraphSeries> TimingSeries = StaticCastSharedPtr<FXiaoTimingGraphSeries>(Series);
		return TimingSeries->GetGraphId() == InGraphId;
	});
}

void FXiaoTimingGraphTrack::UpdateSeries(FXiaoTimingGraphSeries& Series, const FTimingTrackViewport& Viewport)
{
	FGraphTrackBuilder Builder(*this, Series, Viewport);
	const auto& TargetRawGraph = RawSystemGraph[Series.GetGraphId()];

    if(TargetRawGraph.Values.Num() > 0)
    {
		for(int32 Index = 0; Index < TargetRawGraph.Values.Num(); ++Index)
    	{
			Builder.AddEvent(TargetRawGraph.Times[Index], TargetRawGraph.Durations[Index], TargetRawGraph.Values[Index] / Series.GetRealMax() * FXiaoTimingGraphTrack::GraphTrackHeight);
    	}
    }
}