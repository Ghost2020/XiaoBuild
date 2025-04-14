/**
  * @author cxx2020@outlook.com
  * @date 10:47 PM
 */

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "../Tracks/UbaTraceReader.h"

class FJsonValue;
class FJsonObject;
class IMessageLogListing;
class FTokenizedMessage;
class IMessageToken;


class SBuildOutputView final : public SCompoundWidget
{
	struct FProgress
	{
		uint32 ProcessorIndex;
		uint32 ProcessIndex;
	};
public:
	SLATE_BEGIN_ARGS(SBuildOutputView){}
	SLATE_END_ARGS()

	virtual ~SBuildOutputView() override;

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs);

	void LoadFromTraceView(const Xiao::FTraceView& InTraceView);
	void UpdateLogList(const Xiao::FTraceView& InTraceView);
	void FocusMessage(const FName& InIdentifier) const;
	void AddSummaryLog(const Xiao::FTraceView& InTraceView) const;

protected:
	void AddProcessesOutput(const Xiao::FTraceView::FProcess& InProcess);
	TArray<TSharedRef<FTokenizedMessage>> ParseLog(const FName InIdentifier, const TArray<FString>& InOutputs);
	static void OnGotoError(const TSharedRef<IMessageToken>& Token);

private:
	TSharedPtr<IMessageLogListing> MessageLogListing = nullptr;
	TSet<FName> MessageIdentifierSet;
	TMap<uint32, TMap<uint32, uint32>> Session2Prosessor;
};