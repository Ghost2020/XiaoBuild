/**
  * @author cxx2020@outlook.com
  * @date 10:47 PM
 */

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Framework/Docking/TabManager.h"
#include "ShareDefine.h"
#include "../Tracks/UbaTraceReader.h"
#include "../Tracks/UbaNetworkTrace.h"

class FJsonValue;
class FJsonObject;
class SDockTab;

class SBuildView final : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SBuildView){}
		SLATE_ARGUMENT( TWeakPtr<SWindow>, OwnerWindow)
	SLATE_END_ARGS()
	
	void Construct(const FArguments& InArgs);

	bool StartReadFile(const FString& InBuildFile);
	bool StartReadNetwork(const FString& InHostIp, const int32 InListenPort);
	bool StartReadNamed(const FString& InNamedTrace);

	void UpdateProgress(const bool bFirst = false);

	bool GetRealtime() const;

	void OnViewSessionGraph();
	bool IsShowingSesionGraph() const;
	void OnViewDetailsTrack();
	bool IsShowingDetailsTrack() const;
	void OnViewProcessorTrack();
	bool IsShowingProcessorTrack() const;
	void OnToggleCompactMode();
	bool IsCompactMode() const;

	void ShowTraceView() const;
	bool IsTraceViewClosed() const;
	void ShowOutputView() const;
	bool IsOutputViewClosed() const;

private:
	void ConstructWidgets();
	TSharedRef<SDockTab> OnSpawnTab_ProgressView(const FSpawnTabArgs& InTab) const;
	TSharedRef<SDockTab> OnSpawnTab_OutputView(const FSpawnTabArgs& InTab) const ;

private:
	bool bRealtime = false;
	bool bNetwork = false;

	FTSTicker::FDelegateHandle TickHandle = nullptr;

	Xiao::FTraceReader TraceReader;
	Xiao::FTraceView TraceView;
	Xiao::FNetworkTrace NetworkTrace;

	TWeakPtr<SWindow> OwnerWindow = nullptr;
	
	TSharedPtr<FTabManager> TabManager = nullptr;

	TSharedPtr<class SBuildProgressView> ProgressView = nullptr;
	TSharedPtr<class SBuildOutputView> OutputView = nullptr;

	TSharedPtr<class SProgressBar> ProgressBar = nullptr;
	TSharedPtr<STextBlock> ProgressText = nullptr;

	uint8_t ErrorCount = 0;
	uint16 SchedulerPort = 0;

	FSystemGraphShowFlags ShowFlags;
};
