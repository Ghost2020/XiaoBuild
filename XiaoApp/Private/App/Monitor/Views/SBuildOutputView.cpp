/**
  * @author cxx2020@outlook.com
  * @date 10:47 PM
 */
#include "SBuildOutputView.h"

#include "ISourceCodeAccessModule.h"
#include "ISourceCodeAccessor.h"

#include "IMessageLogListing.h"
#include "Logging/MessageLog.h"
#include "MessageLogModule.h"

#include "SlateOptMacros.h"
#include "ShareDefine.h"


#define LOCTEXT_NAMESPACE "SBuildOutputView"

static const FName SOutputLogName(TEXT("XiaoOutputLog"));

static const FText SBuildStatusText = LOCTEXT("BuildStatus_TEXT", "构建状态");

static const FText SError = LOCTEXT("Error_TEXT", "错误");
static const FText SWarning = LOCTEXT("Warning_TEXT", "警告");
static const FText SBuildSystemError = LOCTEXT("SystemError_TEXT", "系统错误");
static const FText SBuildSystemWarning = LOCTEXT("SystemWarning_TEXT", "系统警告");


static const FText SBuildTime = LOCTEXT("BuildTime_TEXT", "构建时间");
static const FText SStartOn = LOCTEXT("StartOn_TEXT", "\t\t开始于");
static const FText STotalBuildTime = LOCTEXT("TotalBuildTime_TEXT", "\t\t总共构建时间");
static const FText SLicenseInformation = LOCTEXT("LicenseInfo_TEXT", "许可信息");


struct FOutputDesc
{
	explicit FOutputDesc(const FName InIdentifier, const TArray<FString>& InOutputs, const float InFinishTime)
		: Identifier(InIdentifier)
		, Outputs(InOutputs)
		, FinishTime(InFinishTime)
	{}

	FName Identifier;
	TArray<FString> Outputs;
	float FinishTime = 0.0f;
};


SBuildOutputView::~SBuildOutputView()
{
	if (MessageLogListing.IsValid())
	{
		FMessageLogModule& MessageLogModule = FModuleManager::LoadModuleChecked<FMessageLogModule>("MessageLog");
		if (MessageLogModule.IsRegisteredLogListing(SOutputLogName))
		{
			MessageLogModule.UnregisterLogListing(SOutputLogName);
		}
	}
}

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SBuildOutputView::Construct(const FArguments& InArgs)
{
	XIAO_LOG(Log, TEXT("SBuildOutputView::Construct::Begin"));
	GLog->Flush();

	FMessageLogModule& MessageLogModule = FModuleManager::LoadModuleChecked<FMessageLogModule>("MessageLog");
	FMessageLogInitializationOptions InitOptions;
	InitOptions.bShowFilters = true;
	InitOptions.bShowPages = false;
	InitOptions.bAllowClear = false;
	InitOptions.bShowInLogWindow = false;
	InitOptions.bDiscardDuplicates = true;
	InitOptions.bScrollToBottom = true;
	MessageLogModule.RegisterLogListing(SOutputLogName, FText::FromString(SOutputLogName.ToString()), InitOptions);
	MessageLogListing = MessageLogModule.GetLogListing(SOutputLogName);

	ChildSlot
	[
		MessageLogModule.CreateLogListingWidget(MessageLogListing.ToSharedRef())
	];

	XIAO_LOG(Log, TEXT("SBuildOutputView::Construct::Finish"));
	GLog->Flush();
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SBuildOutputView::LoadFromTraceView(const Xiao::FTraceView& InTraceView)
{
	if (MessageLogListing.IsValid())
	{
		MessageLogListing->ClearMessages();
	}
	MessageIdentifierSet.Empty();

	TArray<Xiao::FTraceView::FProcess> AllProcesses;
	for (const auto& Session : InTraceView.sessions)
	{
		for (const auto& Processor : Session.processors)
		{
			AllProcesses.Append(Processor.processes);
		}
	}
	AllProcesses.Sort([](const Xiao::FTraceView::FProcess& L, const Xiao::FTraceView::FProcess& R)
	{
		return L.start < R.start;
	});

	for (const auto& Process: AllProcesses)
	{
		AddProcessesOutput(Process);
	}
	
	AddSummaryLog(InTraceView);
}

void SBuildOutputView::UpdateLogList(const Xiao::FTraceView& InTraceView)
{
	TArray<Xiao::FTraceView::FProcess> AllProcesses;
	for(int32 SessionIndex = 0; SessionIndex < InTraceView.sessions.Num(); ++SessionIndex)
	{
		const auto& Session = InTraceView.sessions[SessionIndex];
		auto& SessionMap = Session2Prosessor.FindOrAdd(SessionIndex);
		
		for (int32 ProcessorIndex = 0; ProcessorIndex < Session.processors.Num(); ++ProcessorIndex)
		{
			const auto& Prossor = Session.processors[ProcessorIndex];
			uint32& ProsessorStart = SessionMap.FindOrAdd(ProcessorIndex);
			for (int32 ProcessIndex = ProsessorStart; ProcessIndex < Prossor.processes.Num(); ++ProcessIndex)
			{
				const auto& Process = Prossor.processes[ProcessIndex];
				if (Process.stop != ~uint64(0))
				{
					ProsessorStart = ProcessIndex + 1;
					AllProcesses.Add(Process);
				}
			}
		}
	}

	AllProcesses.Sort([](const Xiao::FTraceView::FProcess& L, const Xiao::FTraceView::FProcess& R)
	{
		return L.start < R.start;
	});
	for (const auto& Process : AllProcesses)
	{
		AddProcessesOutput(Process);
	}
}

void SBuildOutputView::FocusMessage(const FName& InIdentifier) const
{
	MessageLogListing->ClearSelectedMessages();
	if (MessageLogListing.IsValid())
	{
		for (const auto& Message : MessageLogListing->GetFilteredMessages())
		{
			if (Message->GetIdentifier() == InIdentifier)
			{
				MessageLogListing->SelectMessage(Message, true);
				return;
			}
		}
	}
}

void SBuildOutputView::AddSummaryLog(const Xiao::FTraceView& InTraceView) const
{
	FMessageLog MessageLog(SOutputLogName);
	for (const auto& Session : InTraceView.sessions)
	{
		for (const auto& Summary : Session.summary)
		{
			const TSharedRef<FTokenizedMessage> SummaryMessage = FTokenizedMessage::Create(EMessageSeverity::Type::Info, FText::FromString(Summary));
			MessageLog.AddMessage(SummaryMessage);
		}	
	}
}

void SBuildOutputView::AddProcessesOutput(const Xiao::FTraceView::FProcess& InProcess)
{
	FMessageLog MessageLog(SOutputLogName);

	const FName Identifier(FString::FromInt(InProcess.id));
	
	const double StartTime = FPlatformTime::ToSeconds64(InProcess.start);
	const double EndTime = FPlatformTime::ToSeconds64(InProcess.stop);
	FString NodeTitle;
	auto MessageSeverity = EMessageSeverity::Type::Info;
	if (InProcess.returnedReason.IsEmpty())
	{
		NodeTitle = FString::Printf(TEXT("%s (%.1f s +%.1f s)"), *InProcess.description, StartTime, EndTime - StartTime);
		MessageSeverity = InProcess.exitCode == 0 ? EMessageSeverity::Type::Info : EMessageSeverity::Type::Error;
	}
	else
	{
		NodeTitle = FString::Printf(TEXT("%s ReturnReason::%s"), *InProcess.description, *InProcess.returnedReason);
		MessageSeverity = EMessageSeverity::Type::Warning;
	}

	const TSharedRef<FTokenizedMessage> ProcessMessage = FTokenizedMessage::Create(MessageSeverity, FText::FromString(NodeTitle));
	ProcessMessage->SetIdentifier(Identifier);
	MessageLog.AddMessage(ProcessMessage);

	TArray<FString> Outputs;
	for (const auto& Log : InProcess.logLines)
	{
		if (!Log.text.IsEmpty())
		{
			Outputs.Add(Log.text);
		}
	}
	MessageLog.AddMessages(ParseLog(Identifier, Outputs));
}

TArray<TSharedRef<FTokenizedMessage>> SBuildOutputView::ParseLog(const FName InIdentifier, const TArray<FString>& InOutputs)
{
	TArray< TSharedRef<FTokenizedMessage> > Messages;

	TArray< FString > MessageLines = InOutputs;

	// delete any trailing empty lines
	if (MessageLines.Num() > 0)
	{
		for (int32 i = MessageLines.Num() - 1; i >= 0; --i)
		{
			if (!MessageLines[i].IsEmpty())
			{
				if (i < MessageLines.Num() - 1)
				{
					MessageLines.RemoveAt(i + 1, MessageLines.Num() - (i + 1));
				}
				break;
			}
		}
	}

	for (int32 i = 0; i < MessageLines.Num(); ++i)
	{
		FString Line = MessageLines[i];
		if (Line.EndsWith(TEXT("\r"), ESearchCase::CaseSensitive))
		{
			Line.LeftChopInline(1, EAllowShrinking::No);
		}
		Line.ConvertTabsToSpacesInline(4);
		Line.TrimEndInline();

		// handle output line error message if applicable
		// @todo Handle case where there are parenthesis in path names
		// @todo Handle errors reported by Clang
		FString LeftStr, RightStr;
		FString FullPath, LineNumberString;
		if (Line.Split(TEXT(")"), &LeftStr, &RightStr, ESearchCase::CaseSensitive) &&
			LeftStr.Split(TEXT("("), &FullPath, &LineNumberString, ESearchCase::CaseSensitive) &&
			LineNumberString.IsNumeric() && (FCString::Strtoi(*LineNumberString, nullptr, 10) > 0) &&
			RightStr.Contains(TEXT(": error")))
		{
			EMessageSeverity::Type Severity = EMessageSeverity::Error;
			FString FullPathTrimmed = FullPath;
			FullPathTrimmed.TrimStartInline();
			if (FullPathTrimmed.Len() != FullPath.Len()) // check for leading whitespace
			{
				Severity = EMessageSeverity::Info;
			}

			TSharedRef<FTokenizedMessage> Message = FTokenizedMessage::Create(Severity);
			if (Severity == EMessageSeverity::Info)	// add whitespace now
			{
				FString Whitespace = FullPath.Left(FullPath.Len() - FullPathTrimmed.Len());
				Message->AddToken(FTextToken::Create(FText::FromString(Whitespace)));
				FullPath = FullPathTrimmed;
			}

			FString Link = FullPath + TEXT("(") + LineNumberString + TEXT(")");
			Message->AddToken(FTextToken::Create(FText::FromString(Link))->OnMessageTokenActivated(FOnMessageTokenActivated::CreateStatic(&SBuildOutputView::OnGotoError)));
			Message->AddToken(FTextToken::Create(FText::FromString(RightStr)));
			
			Messages.Add(Message);

			if (Severity == EMessageSeverity::Error)
			{
				FPlatformMisc::LowLevelOutputDebugStringf(TEXT("%s"), *Line);
			}
		}
		else
		{
			EMessageSeverity::Type Severity = EMessageSeverity::Info;
			if (Line.Contains(TEXT("Error"), ESearchCase::IgnoreCase))
			{
				Severity = EMessageSeverity::Error;
				FPlatformMisc::LowLevelOutputDebugStringf(TEXT("%s"), *Line);
			}
			else if (Line.Contains(TEXT("Warning"), ESearchCase::IgnoreCase))
			{
				Severity = EMessageSeverity::Warning;
			}
			
			TSharedRef<FTokenizedMessage> Message = FTokenizedMessage::Create(Severity);
			if (i == 0)
			{
				MessageIdentifierSet.Add(InIdentifier);
				Message->SetIdentifier(InIdentifier);
			}
			Message->AddToken(FTextToken::Create(FText::FromString(Line)));
			Messages.Add(Message);
		}
	}

	return Messages;
}

void SBuildOutputView::OnGotoError(const TSharedRef<IMessageToken>& Token)
{
	FString FullPath, LineNumberString;
	if (Token->ToText().ToString().Split(TEXT("("), &FullPath, &LineNumberString, ESearchCase::CaseSensitive))
	{
		LineNumberString.LeftChopInline(1, EAllowShrinking::No); // remove right parenthesis
		const int32 LineNumber = FCString::Strtoi(*LineNumberString, nullptr, 10);
		const ISourceCodeAccessModule& SourceCodeAccessModule = FModuleManager::LoadModuleChecked<ISourceCodeAccessModule>(FName("SourceCodeAccess"));
		auto& Accessor = SourceCodeAccessModule.GetAccessor();

		if (!Accessor.OpenFileAtLine(FullPath, LineNumber, 0))
		{
			XIAO_LOG(Warning, TEXT("Can't open file at line"));
		}
	}
}

#undef LOCTEXT_NAMESPACE