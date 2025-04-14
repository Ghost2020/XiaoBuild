/**
  * @author cxx2020@outlook.com
  * @date 11:02 PM
 */

#pragma once

#include "CoreMinimal.h"
#include "SBaseView.h"

#ifdef USE_IMGUI
class SStatisticsView final : public SBaseView
{
	friend class SStatsWidget;
public:
	SLATE_BEGIN_ARGS(SStatisticsView) {}
		SLATE_EVENT(FOnQueueNotification, OnQueueNotification)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;

protected:
	FText OnGetTotalBuildNum() const;
	FText OnGetTotalBuildDur() const;
	void OnDateSelectionChanged(TSharedPtr<FString> InDate, ESelectInfo::Type);

private:
	TArray<TSharedPtr<FString>> OptionalSource;
	TSharedPtr<class STextComboBox> DateComboBox = nullptr;
	TSharedPtr<class SViewport> Viewport = nullptr;
	TSharedPtr<class SStatsWidget> StatsWidget = nullptr;
};
#endif
