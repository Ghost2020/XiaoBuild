/**
  * @author cxx2020@outlook.com
  * @date 8:54 PM
 */

#pragma once
#include "CoreMinimal.h"
#include "Fonts/SlateFontInfo.h"
#include "Insights/ViewModels/BaseTimingTrack.h"
#include "TrackHeader.h"

namespace TraceServices
{
	struct FLogCategoryInfo;
	struct FLogMessageInfo;
	class ILogProvider;
}

struct FDrawContext;
struct FSlateBrush;
class FTimingTrackViewport;
class FXiaoMarkersTimingTrack;

namespace Xiao
{
	struct FTimeMarkerBoxInfo
	{
		float X;
		float W;
		FLinearColor Color;
	};

	////////////////////////////////////////////////////////////////////////////////////////////////////

	struct FTimeMarkerTextInfo
	{
		float X;
		FLinearColor Color;
		FString Category; // truncated Category string
		FString Message; // truncated Message string
		uint64 LogIndex;
	};

	class FTimeMarkerTrackBuilder
	{
	public:
		explicit FTimeMarkerTrackBuilder(FXiaoMarkersTimingTrack& InTrack, const FTimingTrackViewport& InViewport, float InFontScale);
		FTimeMarkerTrackBuilder(const FTimeMarkerTrackBuilder&) = delete;
		FTimeMarkerTrackBuilder& operator=(const FTimeMarkerTrackBuilder&) = delete;

		const FTimingTrackViewport& GetViewport() const { return Viewport; }

		void BeginLog(const TraceServices::ILogProvider& LogProvider);
		void AddLogMessage(const TraceServices::FLogMessageInfo& Message);
		void EndLog();

		static FLinearColor GetColorByCategory(const TCHAR* const Category);
		static FLinearColor GetColorByVerbosity(const ELogVerbosity::Type Verbosity);

	private:
		void Flush(float AvailableTextW);
		void AddTimeMarker(const float X, const uint64 LogIndex, const ELogVerbosity::Type Verbosity, const TCHAR* const Category, const TCHAR* Message);

	private:
		FXiaoMarkersTimingTrack& Track;
		const FTimingTrackViewport& Viewport;

		const TSharedRef<class FSlateFontMeasure> FontMeasureService;
		const FSlateFontInfo Font;
		float FontScale;

		const TraceServices::ILogProvider* LogProviderPtr; // valid only between BeginLog() and EndLog()

		float LastX1;
		float LastX2;
		uint64 LastLogIndex;
		ELogVerbosity::Type LastVerbosity;
		const TCHAR* LastCategory;
		const TCHAR* LastMessage;
	};
};
	
class FXiaoMarkersTimingTrack final : public FBaseTimingTrack
{
	friend class Xiao::FTimeMarkerTrackBuilder;
	INSIGHTS_DECLARE_RTTI(FXiaoMarkersTimingTrack, FBaseTimingTrack)
	
public:
	FXiaoMarkersTimingTrack();
	virtual ~FXiaoMarkersTimingTrack() override;

	virtual void Reset() override;

	bool IsCollapsed() const { return Header.IsCollapsed(); }
	void Expand() { Header.SetIsCollapsed(false); }
	void Collapse() { Header.SetIsCollapsed(true); }
	void ToggleCollapsed() { Header.ToggleCollapsed(); }

	bool IsBookmarksTrack() const { return bUseOnlyBookmarks; }
	bool IsLogsTrack() const { return !bUseOnlyBookmarks; }
	void SetBookmarksTrackFlag(const bool bInUseOnlyBookmarks)
	{
		bUseOnlyBookmarks = bInUseOnlyBookmarks;
		UpdateTrackNameAndHeight();
	}
	void SetBookmarksTrack() { SetBookmarksTrackFlag(true); SetDirtyFlag(); }
	void SetLogsTrack() { SetBookmarksTrackFlag(false); SetDirtyFlag(); }

	bool SaveScreenshot_CanExecute() const;
	void SaveScreenshot_Execute() const;

	// Stats
	int32 GetNumLogMessages() const { return NumLogMessages; }
	int32 GetNumBoxes() const { return TimeMarkerBoxes.Num(); }
	int32 GetNumTexts() const { return TimeMarkerTexts.Num(); }

	// ~ FBaseTimingTrack Begin
	virtual void PreUpdate(const ITimingTrackUpdateContext& Context) override;
	virtual void Update(const ITimingTrackUpdateContext& Context) override;
	virtual void PostUpdate(const ITimingTrackUpdateContext& Context) override;
	virtual void Draw(const ITimingTrackDrawContext& Context) const override;
	virtual void PostDraw(const ITimingTrackDrawContext& Context) const override;
	virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseButtonDoubleClick(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual void BuildContextMenu(FMenuBuilder& MenuBuilder) override;
	// ~ FBaseTimingTrack Finish

	double Snap(double Time, double SnapTolerance) const;

private:
	void ResetCache()
	{
		TimeMarkerBoxes.Reset();
		TimeMarkerTexts.Reset();
	}

	void UpdateTrackNameAndHeight();
	void UpdateDrawState(const ITimingTrackUpdateContext& Context);
	void UpdateCategory(const TraceServices::FLogCategoryInfo*& InOutCategory, const TCHAR* CategoryName);

	static bool TryGetHoveredEventScreenshotId(uint32& OutScreenshotId);

private:
	TArray<Xiao::FTimeMarkerBoxInfo> TimeMarkerBoxes;
	TArray<Xiao::FTimeMarkerTextInfo> TimeMarkerTexts;

	bool bUseOnlyBookmarks; // If true, uses only bookmarks; otherwise it uses all log messages.
	const TraceServices::FLogCategoryInfo* BookmarkCategory;
	const TraceServices::FLogCategoryInfo* ScreenshotCategory;

	Xiao::FTrackHeader Header;

	// Stats
	int32 NumLogMessages;
	mutable int32 NumDrawBoxes;
	mutable int32 NumDrawTexts;

	// Slate resources
	const FSlateBrush* WhiteBrush;
	const FSlateFontInfo Font;
	uint32 LastScreenshotId;
};
