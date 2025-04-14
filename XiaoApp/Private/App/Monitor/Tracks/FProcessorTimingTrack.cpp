/**
  * @author cxx2020@outlook.com
  * @date 3:12 PM
 */

#include "FProcessorTimingTrack.h"
#include "../Views/SBuildProgressView.h"
#include "Textures/SlateIcon.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "BinaryReader.h"
#include "UbaStats.h"

INSIGHTS_IMPLEMENT_RTTI(FProcessTimingTrack)

#define LOCTEXT_NAMESPACE "FProcessTimingTrack"

void FProcessTimingSharedState::Reset(const UE::Insights::Timing::ITimingViewSession& InSession)
{
	if (&InSession != ProgressView)
	{
		return;
	}
	
	bShowHideAllTracks = true;

	TimingProfilerTimelineCount = 0;
	LoadTimeProfilerTimelineCount = 0;
}

void FProcessTimingSharedState::OnBeginSession(UE::Insights::Timing::ITimingViewSession& InSession)
{
	Reset(InSession);
}

void FProcessTimingSharedState::OnEndSession(UE::Insights::Timing::ITimingViewSession& InSession)
{
	Reset(InSession);
}

void FProcessTimingSharedState::Tick(UE::Insights::Timing::ITimingViewSession& InSession,
	const TraceServices::IAnalysisSession& InAnalysisSession)
{
}


INSIGHTS_IMPLEMENT_RTTI(FProcessTrackEvent)


struct FProcessStats
{
	Xiao::ProcessStats processStats;
	Xiao::SessionStats sessionStats;
	Xiao::StorageStats storageStats;
	Xiao::KernelStats kernelStats;
	Xiao::CacheStats cacheStats;

	Xiao::FTraceView::FProcess Process;
	uint32 Version;
	uint64 Frequency;

	bool bHasWarning = false;

	explicit FProcessStats(const Xiao::FTraceView::FProcess& InProcess, const uint32 InVersion, const uint64 InFrequency)
		: Process(InProcess)
		, Version(InVersion)
		, Frequency(InFrequency)
	{
		Process = InProcess;
		Version = InVersion;
		Frequency = InFrequency;

		for (const auto& Log : InProcess.logLines)
		{
			if (Log.type == Xiao::ELogEntryType::LogEntryType_Warning)
			{
				bHasWarning = true;
				break;
			}
			else if (Log.type == Xiao::ELogEntryType::LogEntryType_Info)
			{
				if (Log.text.Contains(TEXT("")))
				{
					bHasWarning = true;
					break;
				}
			}
		}

		if (Process.stats.Num() == 0)
		{
			return;
		}

		Xiao::FBinaryReader reader(Process.stats.GetData(), 0, Process.stats.Num());

		if (InProcess.cacheFetch)
		{
			cacheStats.Read(reader, InVersion);
			if (reader.GetLeft())
			{
				storageStats.Read(reader, InVersion);
				kernelStats.Read(reader, InVersion);
			}
		}
		else
		{
			processStats.Read(reader, InVersion);

			if (reader.GetLeft())
			{
				if (InProcess.isRemote)
					sessionStats.Read(reader, InVersion);
				storageStats.Read(reader, InVersion);
				kernelStats.Read(reader, InVersion);
			}
		}
	}

	void Print(FTooltipDrawState& InOutTooltip, const TMap<uint32, Xiao::FTraceView::FCacheWrite>& InCache) const
	{
		if (Process.cacheFetch)
		{
			if (!Process.returnedReason.IsEmpty())
				InOutTooltip.AddTextLine(TEXT("  Cache:       Miss"), FLinearColor::White);
			else
				InOutTooltip.AddTextLine(TEXT("  Cache:        Hit"), FLinearColor::White);
		}

		if (processStats.hostTotalTime)
		{
			InOutTooltip.AddTextLine(TEXT(""), FLinearColor::Transparent);
			InOutTooltip.AddTitle(TEXT("  ----------- Detours stats -----------"));
			processStats.Print(InOutTooltip, Frequency);
		}

		if (!sessionStats.IsEmpty())
		{
			InOutTooltip.AddTextLine(TEXT(""), FLinearColor::Transparent);
			InOutTooltip.AddTitle(TEXT("  ----------- Session stats -----------"));
			sessionStats.Print(InOutTooltip, Frequency);
		}

		if (!cacheStats.IsEmpty())
		{
			InOutTooltip.AddTextLine(TEXT(""), FLinearColor::Transparent);
			InOutTooltip.AddTitle(TEXT("  ------------ Cache stats ------------"));
			cacheStats.Print(InOutTooltip, Frequency);
		}

		if (!storageStats.IsEmpty())
		{
			InOutTooltip.AddTextLine(TEXT(""), FLinearColor::Transparent);
			InOutTooltip.AddTitle(TEXT("  ----------- Storage stats -----------"));
			storageStats.Print(InOutTooltip, Frequency);
		}

		if (!kernelStats.IsEmpty())
		{
			InOutTooltip.AddTextLine(TEXT(""), FLinearColor::Transparent);
			InOutTooltip.AddTitle(TEXT("  ----------- Kernel stats ------------"));
			kernelStats.Print(InOutTooltip, false, Frequency);
		}

		const auto findIt = InCache.Find(Process.id);
		if (findIt)
		{
			Xiao::FTraceView::FCacheWrite write = *findIt;
			InOutTooltip.AddTextLine(TEXT(""), FLinearColor::Transparent);
			InOutTooltip.AddTitle(TEXT("  --------- Cache write stats ----------"));
			InOutTooltip.AddTextLine(FString::Printf(TEXT("  Duration                    %9s"), *FPlatformTime::PrettyTime((write.end - write.start))), FLinearColor::White);
			InOutTooltip.AddTextLine(FString::Printf(TEXT("  Success                     %9s"), write.success ? TEXT("true") : TEXT("false")), FLinearColor::White);
			InOutTooltip.AddTextLine(FString::Printf(TEXT("  Bytes sent                  %9s"), *Xiao::BytesToText(write.bytesSent)), FLinearColor::White);
		}
	}
};


FProcessTimingTrack::~FProcessTimingTrack()
{
	
}

/*void FProcessTimingTrack::BuildContextMenu(FMenuBuilder& MenuBuilder)
{
	MenuBuilder.AddMenuEntry(
		LOCTEXT("CopyCommand_Text", "拷贝构建参数"),
		LOCTEXT("CopyCommand_ToolTip", "拷贝构建参数到粘贴板"),
		FSlateIcon(),
		FUIAction(
			FExecuteAction::CreateLambda([]()
			{
				
			}),
			FCanExecuteAction::CreateLambda([]()
			{
				return false;
			})
		)
	);

	MenuBuilder.AddSeparator();
}*/

void FProcessTimingTrack::BuildDrawState(ITimingEventsTrackDrawStateBuilder& Builder, const ITimingTrackUpdateContext& Context)
{
	for (const auto& Process : Processes)
	{
		const bool done = Process.stop != ~uint64(0);
		uint32 EventColor = 0xFF338833; //success
		if (!Process.returnedReason.IsEmpty())
		{
			EventColor = 0xFFFF9900;
		}

		if (!done)
		{
			EventColor = 0xFF99ff33;	//inProgress
		}
		else if (Process.cacheFetch)
		{
			EventColor = 0xFF333388;	//cacheFetch
		}
		else if (Process.exitCode != 0)
		{
			EventColor = 0xFF883333;	//error
		}

		const uint64 Stop = done ? Process.stop : LastTime;
		double StartTime = FPlatformTime::ToSeconds64(Process.start);
		double StopTime = FPlatformTime::ToSeconds64(Stop);
		
		if (done)
		{
			if (!StatsMap.Contains(Process.id))
			{
				StatsMap.Add(MakeTuple(Process.id, MakeShared<FProcessStats>(Process, TraceView->version, TraceView->frequency)));
			}

			if (Process.isRemote)
			{
				// 准备时间
				const double ResponseTime = FPlatformTime::ToSeconds64(Process.start + StatsMap[Process.id]->processStats.waitOnResponse.time);
				Builder.AddEvent(StartTime, ResponseTime, 0, TEXT(""), 0, 0xFF306630);

				// 发送时间
				const double StorageStartTime = StopTime - FPlatformTime::ToSeconds64(StatsMap[Process.id]->processStats.sendFiles.time);
				Builder.AddEvent(StorageStartTime, StopTime, 0, TEXT(""), 0, 0xFF4a9f4a);

				// 运算时间
				StartTime = ResponseTime;
				StopTime = StorageStartTime;
			}

			if (StatsMap[Process.id]->bHasWarning)
			{
				EventColor = 0xFF338888;
				if (Process.exitCode != 0)
				{
					EventColor = 0xFF883333;	//error
				}
			}
		}
		Builder.AddEvent(StartTime, StopTime, 0, *Process.description, 0, EventColor);
	}
}

const TSharedPtr<const ITimingEvent> FProcessTimingTrack::SearchEvent(const FTimingEventSearchParameters& InSearchParameters) const
{
	TSharedPtr<FProcessTrackEvent> FoundEvent;
	uint32 Index = 0;
	for(const auto& Process : Processes)
	{
		const double Start = FPlatformTime::ToSeconds64(Process.start);
		const bool done = Process.stop != ~uint64(0);
		const double Stop = FPlatformTime::ToSeconds64(done ? Process.stop : LastTime);
		if(Start < InSearchParameters.StartTime && Stop > InSearchParameters.StartTime)
		{
			FoundEvent = MakeShared<FProcessTrackEvent>(SharedThis(this), Process, LastTime, 0);
			break;
		}
	}

	return FoundEvent;
}

void FProcessTimingTrack::InitTooltip(FTooltipDrawState& InOutTooltip, const ITimingEvent& InTooltipEvent) const
{
	InOutTooltip.ResetContent();
	if (!InTooltipEvent.CheckTrack(this) || !InTooltipEvent.Is<FProcessTrackEvent>())
	{
		return;
	}

	const FProcessTrackEvent& ProcessEvent = InTooltipEvent.As<FProcessTrackEvent>();
	const auto& Process = ProcessEvent.GetProcessDesc();
	const bool done = Process.stop != ~uint64(0);
	InOutTooltip.AddTitle(Process.description);
	InOutTooltip.AddNameValueTextLine(TEXT("Start Time:"), FString::Printf(TEXT("%.1f"), ProcessEvent.GetStartTime()));
	if (done)
	{
		InOutTooltip.AddNameValueTextLine(TEXT("End   Time:"), FString::Printf(TEXT("%.1f"), ProcessEvent.GetEndTime()));
		InOutTooltip.AddNameValueTextLine(TEXT("Duration  :"), FString::Printf(TEXT("%.1f"), ProcessEvent.GetDuration()));
		InOutTooltip.AddNameValueTextLine(TEXT("ExitCode  :"), FString::Printf(TEXT("%d"), Process.exitCode));
	}
	
	if (!Process.returnedReason.IsEmpty())
	{
		InOutTooltip.AddNameValueTextLine(TEXT("ReturedReason   :"), FString::Printf(TEXT("%s"), *Process.returnedReason));
	}
	
	if (!Process.breadcrumbs.IsEmpty())
	{
		constexpr int maxLineLen = 37;
		InOutTooltip.AddTextLine(TEXT(""), FLinearColor::Transparent);
		InOutTooltip.AddTitle(TEXT("------------ Breadcrumbs ------------"));
		for (int32 lineStart = 0, lineEnd = 0; lineEnd < Process.breadcrumbs.Len(); lineStart = lineEnd + 1)
		{
			lineEnd = Process.breadcrumbs.Find(TEXT("\n"), 1, ESearchCase::Type::IgnoreCase, ESearchDir::Type::FromStart, lineStart);
			FString Line = (lineEnd == std::string::npos ? Process.breadcrumbs.Mid(0, lineStart) : Process.breadcrumbs.Mid(lineStart, lineEnd - lineStart));

			if (Line.Len() > maxLineLen)
			{
				for (int32 sectionStart = 0, sectionEnd = 0; sectionStart < Line.Len(); sectionStart = sectionEnd)
				{
					const int32 maxSectionLen = sectionStart == 0 ? maxLineLen : maxLineLen - 2;
					sectionEnd = std::min<int32>(sectionEnd + maxSectionLen, Line.Len());
					FString Section = (sectionStart == 0 ? TEXT("  ") : TEXT("    ")) + Line.Mid(sectionStart, sectionEnd - sectionStart);
					InOutTooltip.AddTextLine(Section, FLinearColor::White);
				}
			}
			else
			{
				Line = TEXT("  ") + Line;
				InOutTooltip.AddTextLine(Line, FLinearColor::White);
			}
		}
	}

	if (done && !Process.stats.IsEmpty())
	{
		if (StatsMap.Contains(Process.id))
		{
			StatsMap[Process.id]->Print(InOutTooltip, TraceView->cacheWrites);
		}
	}

	InOutTooltip.UpdateLayout();
}

FReply FProcessTimingTrack::OnMouseButtonDoubleClick(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	return FReply::Unhandled();
}

void FProcessTimingTrack::UpdateProcessor(const TArray<Xiao::FTraceView::FProcess>& InProcesses, const uint64 InLastTime)
{
	LastTime = InLastTime;
	
	if (Processes.Num() > 0)
	{
		Processes.Last() = InProcesses[Processes.Num()-1];
	}
	
	for (int Index = Processes.Num(); Index < InProcesses.Num(); ++Index)
	{
		Processes.Add(InProcesses[Index]);
	}
	
	this->SetDirtyFlag();
}

#undef LOCTEXT_NAMESPACE
