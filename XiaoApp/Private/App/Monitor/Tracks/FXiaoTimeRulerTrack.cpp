/**
  * @author cxx2020@outlook.com
  * @date 10:26 PM
 */
#include "FXiaoTimeRulerTrack.h"
#include "Fonts/FontMeasure.h"
#include "Framework/Application/SlateApplication.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "InsightsCore/Common/PaintUtils.h"
#include "InsightsCore/Common/TimeUtils.h"
#include "Styling/AppStyle.h"
#include "XiaoStyle.h"
#include "Insights/ViewModels/TimingTrackViewport.h"
#include "../Helpers/DrawHelpers.h"

#define LOCTEXT_NAMESPACE "TimeRulerTrack"

INSIGHTS_IMPLEMENT_RTTI(FXiaoTimeRulerTrack)

FXiaoTimeRulerTrack::FXiaoTimeRulerTrack()
	: FBaseTimingTrack(TEXT("Time Ruler"))
	, WhiteBrush(FXiaoStyle::Get().GetBrush("WhiteBrush"))
	, Font(FAppStyle::Get().GetFontStyle("SmallFont"))
	, CrtMousePosTime(0.0)
	, CrtMousePosTextWidth(0.0f)
{
	SetValidLocations(ETimingTrackLocation::TopDocked);
	SetOrder(FTimingTrackOrder::TimeRuler);
}

FXiaoTimeRulerTrack::~FXiaoTimeRulerTrack()
{
}

void FXiaoTimeRulerTrack::Reset()
{
	FBaseTimingTrack::Reset();
	bIsSelecting = false;
	SelectionStartTime = 0.0;
	SelectionEndTime = 0.0;
	TimeMarkers.Reset();
	bIsScrubbing = false;
	constexpr float TimeRulerHeight = 24.0f;
	SetHeight(TimeRulerHeight);
}

void FXiaoTimeRulerTrack::SetSelection(const bool bInIsSelecting, const double InSelectionStartTime,
								   const double InSelectionEndTime)
{
	bIsSelecting = bInIsSelecting;
	SelectionStartTime = InSelectionStartTime;
	SelectionEndTime = InSelectionEndTime;
}

void FXiaoTimeRulerTrack::AddTimeMarker(TSharedRef<Xiao::FTimeMarker> InTimeMarker)
{
	TimeMarkers.Add(InTimeMarker);
}

void FXiaoTimeRulerTrack::RemoveTimeMarker(TSharedRef<Xiao::FTimeMarker> InTimeMarker)
{
	TimeMarkers.Remove(InTimeMarker);
}

void FXiaoTimeRulerTrack::RemoveAllTimeMarkers()
{
	TimeMarkers.Reset();
}

TSharedPtr<Xiao::FTimeMarker> FXiaoTimeRulerTrack::GetTimeMarkerByName(const FString& InTimeMarkerName)
{
	for (TSharedRef<Xiao::FTimeMarker>& TimeMarker : TimeMarkers)
	{
		if (TimeMarker->GetName().Equals(InTimeMarkerName))
		{
			return TimeMarker;
		}
	}
	return nullptr;
}

TSharedPtr<Xiao::FTimeMarker> FXiaoTimeRulerTrack::GetTimeMarkerAtPos(const FVector2D& InPosition,
																  const FTimingTrackViewport& InViewport)
{
	TSharedPtr<Xiao::FTimeMarker> ClosestTimeMarker = nullptr;
	constexpr float TimeMarkerBoxHeight = 12.0f;
	const float InPositionY = static_cast<float>(InPosition.Y);
	if (InPositionY >= GetPosY() && InPositionY < GetPosY() + TimeMarkerBoxHeight)
	{
		const float InPositionX = static_cast<float>(InPosition.X);
		float MinDX = 42.0f;
		for (const TSharedRef<Xiao::FTimeMarker>& TimeMarker : TimeMarkers)
		{
			if (TimeMarker->IsVisible())
			{
				const float MarkerX = InViewport.TimeToSlateUnitsRounded(TimeMarker->GetTime());
				const float DX = FMath::Abs(InPositionX - MarkerX);
				if (DX <= MinDX)
				{
					MinDX = DX;
					ClosestTimeMarker = TimeMarker;
				}
			}
		}
	}
	return ClosestTimeMarker;
}

TSharedRef<Xiao::FTimeMarker> FXiaoTimeRulerTrack::GetScrubbingTimeMarker()
{
	checkf(TimeMarkers.Num() > 0, TEXT("TimeMarkers >=0 "));
	return TimeMarkers.Last();
}

void FXiaoTimeRulerTrack::StartScrubbing(TSharedRef<Xiao::FTimeMarker> InTimeMarker)
{
	// Move the scrubbing time marker at the end of sorting list to be draw on top of other markers.
	TimeMarkers.Remove(InTimeMarker);
	TimeMarkers.Add(InTimeMarker);
	bIsScrubbing = true;
}

void FXiaoTimeRulerTrack::StopScrubbing()
{
	if (bIsScrubbing)
	{
		bIsScrubbing = false;
	}
}

void FXiaoTimeRulerTrack::PostUpdate(const ITimingTrackUpdateContext& Context)
{
	const float MouseY = static_cast<float>(Context.GetMousePosition().Y);
	if (MouseY >= GetPosY() && MouseY < GetPosY() + GetHeight())
	{
		SetHoveredState(true);
	}
	else
	{
		SetHoveredState(false);
	}
}

void FXiaoTimeRulerTrack::Draw(const ITimingTrackDrawContext& Context) const
{
	const FDrawContext& DrawContext = Context.GetDrawContext();
	const FTimingTrackViewport& Viewport = Context.GetViewport();
	constexpr float MinorTickMark = 5.0f;
	constexpr float MajorTickMark = 20 * MinorTickMark;
	constexpr float MajorTickMarkHeight = 11.0f;
	const float TextY = GetPosY() + MajorTickMarkHeight;
	const double Vx = Viewport.GetStartTime() * Viewport.GetScaleX();
	const double MinorN = FMath::FloorToDouble(Vx / static_cast<double>(MinorTickMark));
	const double MajorN = FMath::FloorToDouble(Vx / static_cast<double>(MajorTickMark));
	const float MinorOx = static_cast<float>(FMath::RoundToDouble(MinorN * static_cast<double>(MinorTickMark) - Vx));
	const float MajorOx = static_cast<float>(FMath::RoundToDouble(MajorN * static_cast<double>(MajorTickMark) - Vx));
	// Draw the time ruler's background.
	Xiao::FDrawHelpers::DrawBackground(DrawContext, WhiteBrush, Viewport, GetPosY(), GetHeight());
	// Draw the minor tick marks.
	for (float X = MinorOx; X < Viewport.GetWidth(); X += MinorTickMark)
	{
		constexpr float MinorTickMarkHeight = 5.0f;
		const bool bIsTenth = (static_cast<int32>(((X - MajorOx) / MinorTickMark) + 0.4f) % 2 == 0);
		const float MinorTickH = bIsTenth ? MinorTickMarkHeight : MinorTickMarkHeight - 1.0f;
		DrawContext.DrawBox(X, GetPosY(), 1.0f, MinorTickH, WhiteBrush,
							bIsTenth ? FLinearColor(0.3f, 0.3f, 0.3f, 1.0f) : FLinearColor(0.25f, 0.25f, 0.25f, 1.0f));
	}
	// Draw the major tick marks.
	for (float X = MajorOx; X < Viewport.GetWidth(); X += MajorTickMark)
	{
		DrawContext.DrawBox(X, GetPosY(), 1.0f, MajorTickMarkHeight, WhiteBrush, FLinearColor(0.4f, 0.4f, 0.4f, 1.0f));
	}
	DrawContext.LayerId++;
	const double DT = static_cast<double>(MajorTickMark) / Viewport.GetScaleX();
	const double Precision = FMath::Max(DT / 10.0, UE::Insights::FTimeValue::Nanosecond);
	const TSharedRef<FSlateFontMeasure> FontMeasureService = FSlateApplication::Get().GetRenderer()->
		GetFontMeasureService();
	const float FontScale = DrawContext.Geometry.Scale;
	// Draw the time at major tick marks.
	for (float X = MajorOx; X < Viewport.GetWidth() + MajorTickMark; X += MajorTickMark)
	{
		const double T = Viewport.SlateUnitsToTime(X);
		FString Text = UE::Insights::FormatTime(T, Precision);
		const float TextWidth = static_cast<float>(FontMeasureService->Measure(Text, Font, FontScale).X / FontScale);
		DrawContext.DrawText(X - TextWidth / 2, TextY, Text, Font,
							 (T < Viewport.GetMinValidTime() || T >= Viewport.GetMaxValidTime())
								 ? FLinearColor(0.7f, 0.5f, 0.5f, 1.0f)
								 : FLinearColor(0.8f, 0.8f, 0.8f, 1.0f));
	}
	DrawContext.LayerId++;
}

void FXiaoTimeRulerTrack::PostDraw(const ITimingTrackDrawContext& Context) const
{
	const FDrawContext& DrawContext = Context.GetDrawContext();
	const FTimingTrackViewport& Viewport = Context.GetViewport();
	const FVector2D& MousePosition = Context.GetMousePosition();
	const bool bShowMousePos = !MousePosition.IsZero() && !bIsScrubbing;
	const bool bIsMouseOver = bShowMousePos && MousePosition.Y >= GetPosY() && MousePosition.Y < GetPosY() +
		GetHeight();
	if (bShowMousePos)
	{
		constexpr FLinearColor MousePosLineColor(0.9f, 0.9f, 0.9f, 0.1f);
		constexpr FLinearColor MousePosTextBackgroundColor(0.9f, 0.9f, 0.9f, 1.0f);
		FLinearColor MousePosTextForegroundColor(0.1f, 0.1f, 0.1f, 1.0f);
		// Time at current mouse position.
		FString MousePosText;
		const double MousePosTime = Viewport.SlateUnitsToTime(static_cast<float>(MousePosition.X));
		CrtMousePosTime = MousePosTime;
		const double DT = 100.0 / Viewport.GetScaleX();
		const double MousePosPrecision = FMath::Max(DT / 100.0, UE::Insights::FTimeValue::Nanosecond);
		if (bIsMouseOver)
		{
			// If mouse is hovering the time ruler, format time with a better precision (split seconds in ms, us, ns and ps).
			MousePosText = UE::Insights::FormatTimeSplit(MousePosTime, MousePosPrecision);
		}
		else
		{
			// Format current time with one more digit than the time at major tick marks.
			MousePosText = UE::Insights::FormatTime(MousePosTime, MousePosPrecision);
		}
		const TSharedRef<FSlateFontMeasure> FontMeasureService = FSlateApplication::Get().GetRenderer()->GetFontMeasureService();
		const float FontScale = DrawContext.Geometry.Scale;
		const float MousePosTextWidth = FMath::RoundToFloat(static_cast<float>(FontMeasureService->Measure(MousePosText, Font, FontScale).X / FontScale));
		if (!FMath::IsNearlyEqual(CrtMousePosTextWidth, MousePosTextWidth))
		{
			// Animate the box's width (to avoid flickering).
			CrtMousePosTextWidth = CrtMousePosTextWidth * 0.6f + MousePosTextWidth * 0.4f;
		}
		const float TextY = GetPosY() + 11.0f;
		float X = static_cast<float>(MousePosition.X);
		const float W = CrtMousePosTextWidth + 4.0f;
		if (bIsSelecting&& SelectionStartTime<SelectionEndTime)
		{
			// While selecting, display the current time on either left or right side of the selected time range (i.e. to not overlap the selection arrows).
			const float SelectionX1 = Viewport.TimeToSlateUnitsRounded(SelectionStartTime);
			const float SelectionX2 = Viewport.TimeToSlateUnitsRounded(SelectionEndTime);
			if (FMath::Abs(X - SelectionX1) > FMath::Abs(SelectionX2 - X))
			{
				X = SelectionX2 + W / 2.0f;
			}
			else
			{
				X = SelectionX1 - W / 2.0f;
			}
			MousePosTextForegroundColor = FLinearColor(0.01f, 0.05f, 0.2f, 1.0f);
		}
		else
		{
			// Draw horizontal line at mouse position.
			//DrawContext.DrawBox(0.0f, static_cast<float>(MousePosition.Y), Viewport.Width, 1.0f, WhiteBrush, MousePosLineColor);
			// Draw vertical line at mouse position.
			DrawContext.DrawBox(static_cast<float>(MousePosition.X),Viewport.GetPosY(), 1.0f, Viewport.GetHeight(), WhiteBrush, MousePosLineColor);
			// Stroke the vertical line above current time box.
			DrawContext.DrawBox(static_cast<float>(MousePosition.X), GetPosY(), 1.0f, TextY - GetPosY(), WhiteBrush, MousePosTextBackgroundColor);
		}
		// Fill the current time box.
		DrawContext.DrawBox(X - W / 2.0f, TextY, W, 12.0f, WhiteBrush, MousePosTextBackgroundColor);
		DrawContext.LayerId++;
		// Draw current time text.
		DrawContext.DrawText(X - MousePosTextWidth / 2.0f, TextY, MousePosText, Font, MousePosTextForegroundColor);
		DrawContext.LayerId++;
	}
	// Draw the time markers.
	for (const TSharedRef<Xiao::FTimeMarker>& TimeMarker : TimeMarkers)
	{
		DrawTimeMarker(Context, *TimeMarker);
	}
}

void FXiaoTimeRulerTrack::DrawTimeMarker(const ITimingTrackDrawContext& Context, const Xiao::FTimeMarker& TimeMarker) const
{
	if (!TimeMarker.IsVisible())
	{
		return;
	}
	const FTimingTrackViewport& Viewport = Context.GetViewport();
	const float TimeMarkerX = Viewport.TimeToSlateUnitsRounded(TimeMarker.GetTime());
	const float HalfWidth = TimeMarker.GetCrtTextWidth() / 2;
	if (TimeMarkerX < -HalfWidth || TimeMarkerX > Viewport.GetWidth() + HalfWidth)
	{
		return;
	}
	const float TimeMarkerY = GetPosY();
	constexpr float BoxHeight = 12.0f;
	const FDrawContext& DrawContext = Context.GetDrawContext();
	// Draw the vertical line.
	DrawContext.DrawBox(TimeMarkerX, TimeMarkerY, 1.0f, Viewport.GetPosY() + Viewport.GetHeight() - TimeMarkerY, WhiteBrush, TimeMarker.GetColor());
	DrawContext.LayerId++;
	const FVector2D& MousePosition = Context.GetMousePosition();
	const bool bIsMouseOverTrack = !MousePosition.IsZero() &&
		MousePosition.Y >= GetPosY() &&
		MousePosition.Y < GetPosY() + GetHeight();
	constexpr float FixedHalfWidth = 42.0f;
	const bool bIsMouseOverMarker = bIsMouseOverTrack &&
		MousePosition.Y >= GetPosY() + TimeMarkerY &&
		MousePosition.Y < GetPosY() + TimeMarkerY + BoxHeight &&
		MousePosition.X >= TimeMarkerX - FixedHalfWidth &&
		MousePosition.X < TimeMarkerX + FixedHalfWidth;
	// Decide what text to display.
	FString TimeMarkerText;
	if (bIsMouseOverTrack)
	{
		if (TimeMarker.GetName().Len() > 0)
		{
			TimeMarkerText = TimeMarker.GetName() + TEXT(": ");
		}
		// Format time value with one more digit than the time at major tick marks.
		const double DT = 100.0 / Viewport.GetScaleX();
		const double Precision = FMath::Max(DT / 100.0, UE::Insights::FTimeValue::Nanosecond);
		if (bIsMouseOverMarker)
		{
			// If mouse is hovering the time marker, format time with a better precision (split seconds in ms, us, ns and ps).
			TimeMarkerText += UE::Insights::FormatTimeSplit(TimeMarker.GetTime(), Precision);
		}
		else
		{
			TimeMarkerText += UE::Insights::FormatTime(TimeMarker.GetTime(), Precision);
		}
	}
	else
	{
		if (TimeMarker.GetName().Len() > 0)
		{
			TimeMarkerText = TimeMarker.GetName();
		}
		else
		{
			TimeMarkerText = TEXT("T");
		}
	}
	const TSharedRef<FSlateFontMeasure> FontMeasureService = FSlateApplication::Get().GetRenderer()->GetFontMeasureService();
	const float FontScale = DrawContext.Geometry.Scale;
	const float TextWidth = FMath::RoundToFloat(static_cast<float>(FontMeasureService->Measure(TimeMarkerText, Font, FontScale).X / FontScale));
	if (!FMath::IsNearlyEqual(TimeMarker.GetCrtTextWidth(), TextWidth))
	{
		// Animate the box's width (to avoid flickering).
		TimeMarker.SetCrtTextWidthAnimated(TextWidth);
	}
	const FLinearColor TextBackgroundColor(TimeMarker.GetColor().CopyWithNewOpacity(1.0f));
	constexpr FLinearColor TextForegroundColor(0.07f, 0.07f, 0.07f, 1.0f);
	// Fill the time marker box.
	const float BoxWidth = TimeMarker.GetCrtTextWidth() + 4.0f;
	DrawContext.DrawBox(TimeMarkerX - BoxWidth / 2, TimeMarkerY, BoxWidth, BoxHeight, WhiteBrush, TextBackgroundColor);
	DrawContext.LayerId++;
	// Draw time marker text.
	DrawContext.DrawText(TimeMarkerX - TextWidth / 2, TimeMarkerY, TimeMarkerText, Font, TextForegroundColor);
	DrawContext.LayerId++;
}

void FXiaoTimeRulerTrack::BuildContextMenu(FMenuBuilder& MenuBuilder)
{
	MenuBuilder.SetSearchable(false);

	TArray<TSharedRef<Xiao::FTimeMarker>> VisibleTimeMarkers;
	for (TSharedRef<Xiao::FTimeMarker>& TimeMarker : TimeMarkers)
	{
		if (TimeMarker->IsVisible())
		{
			VisibleTimeMarkers.Add(TimeMarker);
		}
	}
	if (VisibleTimeMarkers.Num() > 0)
	{
		// Sort TimeMarkers by name.
		VisibleTimeMarkers.Sort(
			[](const TSharedRef<Xiao::FTimeMarker>& A, const TSharedRef<Xiao::FTimeMarker>& B) -> bool
			{
				return A->GetName().Compare(B->GetName()) <= 0;
			});
		MenuBuilder.BeginSection("TimeMarkers", LOCTEXT("ContextMenu_Section_TimeMarkers", "时间标签"));
		for (const TSharedRef<Xiao::FTimeMarker>& TimeMarker : VisibleTimeMarkers)
		{
			FUIAction Action_MoveTimeMarker
			(
				FExecuteAction::CreateSP(this, &FXiaoTimeRulerTrack::ContextMenu_MoveTimeMarker_Execute, TimeMarker),
				FCanExecuteAction()
			);
			const FString& MarkerNameString = TimeMarker->GetName();
			const FText MarkerNameText = FText::FromString((MarkerNameString.Len() > 0) ? MarkerNameString : TEXT("T"));
			MenuBuilder.AddMenuEntry
			(
				FText::Format(LOCTEXT("ContextMenu_MoveTimeMerker", "Move Time Marker '{0}' Here"), MarkerNameText),
				FText::Format(LOCTEXT("ContextMenu_MoveTimeMerker_Desc",
									  "Move the time marker '{0}' at the current mouse position."), MarkerNameText),
				FSlateIcon(),
				Action_MoveTimeMarker,
				NAME_None,
				EUserInterfaceActionType::Button
			);
		}
		MenuBuilder.EndSection();
	}
}

void FXiaoTimeRulerTrack::ContextMenu_MoveTimeMarker_Execute(const TSharedRef<Xiao::FTimeMarker> InTimeMarker) const
{
	InTimeMarker->SetTime(CrtMousePosTime);
}

#undef LOCTEXT_NAMESPACE
