#include "TimingViewDrawHelper.h"
#include "Styling/AppStyle.h"
#include "XiaoStyle.h"
#include "DrawHelpers.h"
#include "InsightsCore/Common/PaintUtils.h"
#include "../Tracks/FTimingGraphTrack.h"
#include "../Tracks/FSessionDetailsTrack.h"
#include "Insights/ViewModels/TimingTrackViewport.h"

namespace Xiao
{
	FTimingViewDrawHelper::FTimingViewDrawHelper(const FDrawContext& InDrawContext, const FTimingTrackViewport& InViewport)
		: DrawContext(InDrawContext)
		, Viewport(InViewport)
		, WhiteBrush(FXiaoStyle::Get().GetBrush("WhiteBrush"))
		, EventBorderBrush(FXiaoStyle::Get().GetBrush("EventBorder"))
		, HoveredEventBorderBrush(FXiaoStyle::Get().GetBrush("HoveredEventBorder"))
		, SelectedEventBorderBrush(FXiaoStyle::Get().GetBrush("SelectedEventBorder"))
		, BackgroundAreaBrush(WhiteBrush)
		, ValidAreaColor(0.07f, 0.07f, 0.07f, 1.0f)
		, InvalidAreaColor(0.1f, 0.07f, 0.07f, 1.0f)
		, EdgeColor(0.05f, 0.05f, 0.05f, 1.0f)
		, EventFont(FAppStyle::Get().GetFontStyle("SmallFont"))
		, ReservedLayerId(0)
		, ValidAreaX(0.0f)
		, ValidAreaW(0.0f)
		, NumEvents(0)
		, NumMergedBoxes(0)
		, NumDrawBoxes(0)
		, NumDrawBorders(0)
		, NumDrawTexts(0)
	{
	}

	FTimingViewDrawHelper::~FTimingViewDrawHelper()
	{
	}

	FLinearColor FTimingViewDrawHelper::GetTrackNameTextColor(const FBaseTimingTrack& Track) const
	{
		return Track.IsHovered() ? FLinearColor(1.0f, 1.0f, 0.0f, 1.0f) :
			  Track.IsSelected() ? FLinearColor(1.0f, 1.0f, 0.5f, 1.0f) :
								   FLinearColor(1.0f, 1.0f, 1.0f, 1.0f);
	}

	void FTimingViewDrawHelper::DrawBackground() const
	{
		const float Y = Viewport.GetPosY();
		const float H = FMath::CeilToFloat(Viewport.GetHeight());
		FDrawHelpers::DrawBackground(DrawContext, BackgroundAreaBrush, Viewport, Y, H, ValidAreaX, ValidAreaW);
	}

	void FTimingViewDrawHelper::BeginDrawTracks() const
	{
		ReservedLayerId = DrawContext.LayerId;
		DrawContext.LayerId += ToInt32(EDrawLayer::Count);
	}

	void FTimingViewDrawHelper::DrawContextSwitchMarkers(const FTimingEventsTrackDrawState& DrawState, float LineY, float LineH, float Opacity, bool bDrawOverlays, bool bDrawVerticalLines) const
	{
		if (LineH > 0.0f)
		{
			const int32 LayerId = ReservedLayerId + ToInt32(EDrawLayer::EventHighlight);

			if (bDrawVerticalLines)
			{
				// Draw vertical lines (merged into large boxes).
				for (const FTimingEventsTrackDrawState::FBoxPrimitive& Box : DrawState.Boxes)
				{
					const FLinearColor Color = Box.Color.CopyWithNewOpacity(Opacity);
					DrawContext.DrawBox(LayerId, Box.X, LineY, Box.W, LineH, WhiteBrush, Color);
				}

				// Draw vertical lines (edges of larger events).
				for (const FTimingEventsTrackDrawState::FBoxPrimitive& Box : DrawState.Borders)
				{
					const FLinearColor Color = Box.Color.CopyWithNewOpacity(Opacity);
					DrawContext.DrawBox(LayerId, Box.X, LineY, 1.0f, LineH, WhiteBrush, Color);
					DrawContext.DrawBox(LayerId, Box.X + Box.W - 1.0f, LineY, 1.0f, LineH, WhiteBrush, Color);
				}
			}

			if (bDrawOverlays)
			{
				// Draw overlay for idle areas.
				const FSlateBrush* IdleBrush = WhiteBrush;
				constexpr FLinearColor IdleColor(0.0f, 0.0f, 0.0f, 0.3f);
				const int32 Count1 = DrawState.Boxes.Num();
				const int32 Count2 = DrawState.Borders.Num();
				int32 Index1 = 0;
				int32 Index2 = 0;
				const float MinX = Viewport.TimeToSlateUnitsRounded(Viewport.GetMinValidTime());
				float CurrentX = FMath::Max(MinX, 0.0f);
				while (Index1 < Count1 || Index2 < Count2)
				{
					const float X1 = (Index1 < Count1) ? DrawState.Boxes[Index1].X : FLT_MAX;
					const float X2 = (Index2 < Count2) ? DrawState.Borders[Index2].X : FLT_MAX;
					if (X1 < X2)
					{
						if (X1 - CurrentX > 0.0f)
						{
							DrawContext.DrawBox(LayerId, CurrentX, LineY, X1 - CurrentX, LineH, IdleBrush, IdleColor);
						}
						CurrentX = FMath::Max(CurrentX, X1 + DrawState.Boxes[Index1].W);
						++Index1;
					}
					else
					{
						if (X2 - CurrentX > 0.0f)
						{
							DrawContext.DrawBox(LayerId, CurrentX, LineY, X2 - CurrentX, LineH, IdleBrush, IdleColor);
						}
						CurrentX = FMath::Max(CurrentX, X2 + DrawState.Borders[Index2].W);
						++Index2;
					}
				}
				const float MaxX = Viewport.TimeToSlateUnitsRounded(Viewport.GetMaxValidTime());
				const float LastAreaW = FMath::Min(MaxX, Viewport.GetWidth()) - CurrentX;
				if (LastAreaW > 0.0f)
				{
					DrawContext.DrawBox(LayerId, CurrentX, LineY, LastAreaW, LineH, IdleBrush, IdleColor);
				}
			}
		}
	}

	void FTimingViewDrawHelper::DrawMarkers(const FTimingEventsTrackDrawState& DrawState, float LineY, float LineH, float Opacity) const
	{
		if (LineH > 0.0f)
		{
			const FTimingViewLayout& Layout = Viewport.GetLayout();

			// Draw markers from filled boxes (merged borders).
			for (const FTimingEventsTrackDrawState::FBoxPrimitive& Box : DrawState.Boxes)
			{
				DrawContext.DrawBox(Box.X, LineY, Box.W, LineH, WhiteBrush, Box.Color.CopyWithNewOpacity(Opacity));
			}

			// Draw markers from borders.
			for (const FTimingEventsTrackDrawState::FBoxPrimitive& Box : DrawState.Borders)
			{
				DrawContext.DrawBox(Box.X, LineY, 1.0f, LineH, WhiteBrush, Box.Color.CopyWithNewOpacity(Opacity));
				if (Box.W > 1.0f)
				{
					DrawContext.DrawBox(Box.X + Box.W - 1.0f, LineY, 1.0f, LineH, WhiteBrush, Box.Color.CopyWithNewOpacity(Opacity));
				}
			}

			DrawContext.LayerId++;
		}
	}

	void FTimingViewDrawHelper::DrawTrackHeader(const FBaseTimingTrack& Track) const
	{
		const int32 HeaderLayerId = ReservedLayerId + ToInt32(EDrawLayer::HeaderBackground);
		const int32 HeaderTextLayerId = ReservedLayerId + ToInt32(EDrawLayer::HeaderText);

		DrawTrackHeader(Track, HeaderLayerId, HeaderTextLayerId);
	}

	void FTimingViewDrawHelper::DrawTrackHeader(const FBaseTimingTrack& Track, const int32 HeaderLayerId, const int32 HeaderTextLayerId) const
	{
		const float TrackY = Track.GetPosY();
		const float TrackH = Track.GetHeight();

		if (TrackH > 0.0f)
		{
			// Draw a horizontal line between tracks (top line of a track).
			const int32 TrackBackgroundLayerId = HeaderLayerId - ToInt32(EDrawLayer::HeaderBackground) + ToInt32(EDrawLayer::TrackBackground);
			DrawContext.DrawBox(TrackBackgroundLayerId, 20.0f, TrackY, Viewport.GetWidth(), 1.0f, WhiteBrush, EdgeColor);

			// Draw header with name of tracks.
			if (TrackH > 4.0f)
			{
				const TSharedRef<FSlateFontMeasure> FontMeasureService = FSlateApplication::Get().GetRenderer()->GetFontMeasureService();
				const float FontScale = DrawContext.Geometry.Scale;
				float TextWidth = static_cast<float>(FontMeasureService->Measure(Track.GetName(), EventFont, FontScale).X / FontScale);

				constexpr float PinWidth = 8.0f;
				if (Track.IsSelected())
				{
					TextWidth += PinWidth;
				}

				const float HeaderX = 20.0f;
				const float HeaderW = TextWidth + 4.0f;

				const float HeaderY = TrackY + 1.0f;
				const float HeaderH = FMath::Min(12.0f, Track.GetHeight() - 1.0f);

				if (HeaderH > 0)
				{
					DrawContext.DrawBox(HeaderLayerId, HeaderX, HeaderY, HeaderW, HeaderH, WhiteBrush, EdgeColor);

					const FLinearColor TextColor = GetTrackNameTextColor(Track);

					float TextX = HeaderX + 2.0f;
					const float TextY = HeaderY + HeaderH / 2.0f - 7.0f;

					if (Track.IsSelected())
					{
						// TODO: Use a "pin" image brush instead.
						DrawContext.DrawText(HeaderTextLayerId, TextX, TextY, TEXT("\u25CA"), EventFont, TextColor); // lozenge shape
						TextX += PinWidth;
					}

					DrawContext.DrawText(HeaderTextLayerId, TextX, TextY, Track.GetName(), EventFont, TextColor);
				}
			}
		}
	}

	void FTimingViewDrawHelper::DrawGraphDetails(const FBaseTimingTrack& InGraphTrack) const
	{
		if (const FSessionDetailsTrack* SessionTrack = static_cast<const FSessionDetailsTrack*>(&InGraphTrack))
		{
			const int32 HeaderTextLayerId = ReservedLayerId + ToInt32(EDrawLayer::HeaderText);

			const float HeaderX = 20.0f;
			const float TextHeight = 10.0f;

			const float TrackY = InGraphTrack.GetPosY();
			const float TrackH = InGraphTrack.GetHeight();

			if (auto* Session = SessionTrack->GetSession())
			{
				const auto* TraceView = SessionTrack->GetTraceView();
				FString FinishedMsg, ActiveMsg, Msg;
				FString RecvMsg, SendMsg, ProxyMsg;
				FString FetchedMsg, StoredMsg;

				float TextX = HeaderX + 2.0f;
				float TextY = TrackY;

				auto GetFiles = [&](const FString FileType, TArray<FTraceView::FFileTransfer>& files, uint64 bytes, uint32& maxVisibleFiles)
				{
					FString Meg;
					Meg = FString::Printf(TEXT("%ls Files: %u (%s)"), *FileType, uint32(files.Num()), *BytesToText(bytes));
					uint32 fileCount = 0;
					for (auto rit = files.rbegin(), rend = files.rend(); rit != rend; ++rit)
					{
						FTraceView::FFileTransfer& file = *rit;
						if (file.stop != ~uint64(0))
							continue;
						if (fileCount++ > 5)
							break;
					}
					maxVisibleFiles = FMath::Max(maxVisibleFiles, fileCount);
					return Meg;
				};
				
				if (SessionTrack->IsRemote())
				{
					FinishedMsg = FString::Printf(TEXT("Finished Process:%u"), Session->processExitedCount);
					ActiveMsg = FString::Printf(TEXT("Active Process:%d"), Session->processActiveCount);
					if (!Session->updates.IsEmpty())
					{
						auto& u = Session->updates.Last();
						uint64 sendPerS = 0;
						uint64 recvPerS = 0;
						if (float duration = FPlatformTime::ToSeconds64(u.time - Session->prevUpdateTime))
						{
							sendPerS = uint64((u.send - Session->prevSend) / duration);
							recvPerS = uint64((u.recv - Session->prevRecv) / duration);
						}
						Msg = FString::Printf(TEXT("ClientId:%u TopCount: %u"), Session->clientUid.data1, u.connectionCount);
						RecvMsg = FString::Printf(TEXT("Recv:%ls (%sps)"), *BytesToText(u.recv), *BytesToText(recvPerS * 8));
						SendMsg = FString::Printf(TEXT("Send:%ls (%sps)"), *BytesToText(u.send), *BytesToText(sendPerS * 8));
					}

					if (Session->disconnectTime == ~uint64(0))
					{
						if (Session->proxyCreated)
							ProxyMsg = FString::Printf(TEXT("Proxy(HOSTED): %ls"), *Session->proxyName);
						else if (!Session->proxyName.IsEmpty())
							ProxyMsg = FString::Printf(TEXT("Proxy: %ls"), *Session->proxyName);
						else
							ProxyMsg = TEXT("Proxy: None");
					}
				}
				else
				{
					FinishedMsg = FString::Printf(TEXT("Finished Process:%u (local: %u)"), TraceView->totalProcessExitedCount, Session->processExitedCount);
					ActiveMsg = FString::Printf(TEXT("Active Processes: %u (local: %u)"), TraceView->totalProcessActiveCount, Session->processActiveCount);
					Msg = FString::Printf(TEXT("Active Helpers: %u"), FMath::Max(1u, TraceView->activeSessionCount) - 1);
					if (!Session->updates.IsEmpty())
					{
						auto& u = Session->updates.Last();
						if (u.send || u.recv)
						{
							uint64 sendPerS = 0;
							uint64 recvPerS = 0;
							if (float duration = FPlatformTime::ToSeconds64(u.time - Session->prevUpdateTime))
							{
								sendPerS = uint64((u.send - Session->prevSend) / duration);
								recvPerS = uint64((u.recv - Session->prevRecv) / duration);
							}
							
							RecvMsg = FString::Printf(TEXT("Recv: %ls (%sps)"), *BytesToText(u.recv), *BytesToText(recvPerS));
							SendMsg = FString::Printf(TEXT("Send: %ls (%sps)"), *BytesToText(u.send), *BytesToText(sendPerS));
						}
					}
				}

				DrawContext.DrawText(HeaderTextLayerId, TextX, TextY, FinishedMsg, EventFont, FLinearColor::White);
				if (SessionTrack->IsRemote())
				{
					FetchedMsg = GetFiles(TEXT("Fetched"), Session->fetchedFiles, Session->fetchedFilesBytes, Session->maxVisibleFiles);
					DrawContext.DrawText(HeaderTextLayerId, TextX + 350, TextY, FetchedMsg, EventFont, FLinearColor::White);
					StoredMsg = GetFiles(TEXT("Stored"), Session->storedFiles, Session->storedFilesBytes, Session->maxVisibleFiles);
					DrawContext.DrawText(HeaderTextLayerId, TextX + 650, TextY, StoredMsg, EventFont, FLinearColor::White);
				}

				TextY += TextHeight + 5.0f;
				DrawContext.DrawText(HeaderTextLayerId, TextX, TextY, ActiveMsg, EventFont, FLinearColor::White);

				if (!Msg.IsEmpty())
				{
					TextY += TextHeight + 5.0f;
					DrawContext.DrawText(HeaderTextLayerId, TextX, TextY, Msg, EventFont, FLinearColor::White);
				}

				if (!RecvMsg.IsEmpty())
				{
					TextY += TextHeight + 5.0f;
					DrawContext.DrawText(HeaderTextLayerId, TextX, TextY, RecvMsg, EventFont, FLinearColor::White);
				}

				if (!SendMsg.IsEmpty())
				{
					TextY += TextHeight + 5.0f;
					DrawContext.DrawText(HeaderTextLayerId, TextX, TextY, SendMsg, EventFont, FLinearColor::White);
				}
			}
		}
	}

	void FTimingViewDrawHelper::EndDrawTracks() const
	{
		if (Viewport.GetWidth() > 0.0f)
		{
			const int32 TrackBackgroundLayerId = ReservedLayerId + ToInt32(EDrawLayer::TrackBackground);

			const float TopY = Viewport.GetPosY() + Viewport.GetTopOffset();
			const float BottomY = Viewport.GetPosY() + Viewport.GetHeight() - Viewport.GetBottomOffset();

			if (TopY < BottomY)
			{
				// Y position of the first pixel below the last track.
				const float Y = TopY + Viewport.GetScrollHeight() - Viewport.GetScrollPosY() - 1.0f;

				if (Y >= TopY && Y < BottomY)
				{
					// Draw a last horizontal line.
					DrawContext.DrawBox(TrackBackgroundLayerId, 0.0f, Y, Viewport.GetWidth(), 1.0f, WhiteBrush, EdgeColor);
				}

				// Note: ValidAreaX and ValidAreaW are computed in DrawBackground.
				if (ValidAreaW > 0.0f)
				{
					const float TopInvalidAreaH = FMath::Min(0.0f - Viewport.GetScrollPosY(), Viewport.GetScrollableAreaHeight());
					if (TopInvalidAreaH > 0.0f)
					{
						// Draw invalid area (top).
						DrawContext.DrawBox(TrackBackgroundLayerId, ValidAreaX, TopY, ValidAreaW, TopInvalidAreaH, BackgroundAreaBrush, InvalidAreaColor);
					}

					const float BottomInvalidAreaH = FMath::Min(BottomY - Y - 1.0f, Viewport.GetScrollableAreaHeight());
					if (BottomInvalidAreaH > 0.0f)
					{
						// Draw invalid area (bottom).
						DrawContext.DrawBox(TrackBackgroundLayerId, ValidAreaX, BottomY - BottomInvalidAreaH, ValidAreaW, BottomInvalidAreaH, BackgroundAreaBrush, InvalidAreaColor);
					}
				}
			}
		}
	}

	void FTimingViewDrawHelper::DrawRelations(const TArray<TUniquePtr<ITimingEventRelation>>& Relations, ITimingEventRelation::EDrawFilter Filter) const
	{
		for (const TUniquePtr<ITimingEventRelation>& Relation : Relations)
		{
			Relation->Draw(DrawContext, Viewport, *this, Filter);
		}
	}
}
