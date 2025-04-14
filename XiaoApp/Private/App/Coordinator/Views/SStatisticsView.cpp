/**
  * @author cxx2020@outlook.com
  * @date 11:02 PM
 */
#include "SStatisticsView.h"
#include "SlateOptMacros.h"
#include "ShareDefine.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/STextComboBox.h"
#include "Widgets/SViewport.h"
#ifdef USE_IMGUI
#include "../Widgets/SStatsWidget.h"


#define LOCTEXT_NAMESPACE "SStatisticsView"



namespace
{
	const FText SLast7Days = LOCTEXT("Last7_Text", "最近七天  ");
	const FText SLast15Days = LOCTEXT("Last15_Text", "最近十五天");
	const FText SLast30Days = LOCTEXT("Last30_Text", "最近三十天");
}


BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SStatisticsView::Construct(const FArguments& InArgs)
{
	XIAO_LOG(Log, TEXT("SStatisticsView::Construct::Begin"));
	GLog->Flush();

	OnQueueNotification = InArgs._OnQueueNotification;

	OptionalSource.Add(MakeShareable(new FString(SLast7Days.ToString())));
	OptionalSource.Add(MakeShareable(new FString(SLast15Days.ToString())));
	OptionalSource.Add(MakeShareable(new FString(SLast30Days.ToString())));

	Viewport = SNew(SViewport).ViewportSize_Lambda([this]()
	{
		const TSharedPtr<SWindow> Window = FSlateApplication::Get().FindWidgetWindow(SharedThis(this));
		FVector2D ViewportSize;
		if (Window.IsValid())
		{
			ViewportSize = Window->GetViewportSize();
		}
		else
		{
			ViewportSize = Viewport->GetDesiredSize();
		}
		return ViewportSize;
	});
	Viewport->SetRenderDirectlyToWindow(true);

	Viewport->SetContent(
		SAssignNew(StatsWidget, SStatsWidget)
		.Viewport(Viewport)
	);
	
	ChildSlot
	[
		SNew(SVerticalBox)
#pragma region TopSection
		+ SVerticalBox::Slot().VAlign(VAlign_Top).AutoHeight()
		[
			SNew(SHorizontalBox)
			+SHorizontalBox::Slot()
			[
				SNew(SHorizontalBox)
				+SHorizontalBox::Slot().AutoWidth().Padding(35.0f, 15.0f)
				[
					SNew(STextBlock).Text(LOCTEXT("Date_Text", "时段:"))
				]
				+SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.0f, 15.0f)
				[
					SAssignNew(DateComboBox, STextComboBox)
					.OptionsSource(&OptionalSource)
					.InitiallySelectedItem(OptionalSource[0])
					.OnSelectionChanged_Raw(this, &SStatisticsView::OnDateSelectionChanged)
				]
			]
		]
		+ SVerticalBox::Slot().HAlign(HAlign_Fill).VAlign(VAlign_Fill)
		[
			SNew(SVerticalBox)
			+SVerticalBox::Slot().AutoHeight().Padding(10.0f)
			[
				SNew(SBorder)
				.BorderImage(FXiaoStyle::Get().GetBrush("WhiteBrush"))
				.BorderBackgroundColor(FLinearColor(0.05f, 0.1f, 0.2f, 1.0f))
				[
					SNew(SVerticalBox)
					+SVerticalBox::Slot().AutoHeight().VAlign(VAlign_Top)
					[
						SNew(STextBlock).Text(LOCTEXT("Overview_Text", "总览"))
					]
					
					+SVerticalBox::Slot()
					[
						SNew(SHorizontalBox)
						+SHorizontalBox::Slot()
						[
							SNew(SVerticalBox)
							+SVerticalBox::Slot().AutoHeight()
							[
								SNew(STextBlock).Text(LOCTEXT("TotalNo_Text", "总计构建次数"))
							]
							+SVerticalBox::Slot().AutoHeight()
							[
								SNew(STextBlock).Text_Raw(this, &SStatisticsView::OnGetTotalBuildNum)
							]
						]

						+SHorizontalBox::Slot().AutoWidth()
						[
							SNew(SBorder)
						]

						+SHorizontalBox::Slot()
						[
							SNew(SVerticalBox)
							+SVerticalBox::Slot().AutoHeight()
							[
								SNew(STextBlock).Text(LOCTEXT("TotalDuration_Text", "总计构建时间"))
							]
							+SVerticalBox::Slot().AutoHeight()
							[
								SNew(STextBlock).Text_Raw(this, &SStatisticsView::OnGetTotalBuildDur)
							]
						]
					]
				]
			]
#pragma endregion
#pragma region ImGuiSection
			+SVerticalBox::Slot().HAlign(HAlign_Fill).VAlign(VAlign_Fill)
			[
				SNew(SBorder)
				[
					Viewport.ToSharedRef()
				]
			]
#pragma endregion
		]
	];

	XIAO_LOG(Log, TEXT("SStatisticsView::Construct::Finish"));
	GLog->Flush();
}

void SStatisticsView::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	if (UpdateTimer > 3.0f && GCanUpdate)
	{
		// 读取数据
		UpdateTimer = 0.0f;
	}
}

FText SStatisticsView::OnGetTotalBuildNum() const
{
	return FText::FromString(FString::Printf(TEXT("%d"), 1024));
}

FText SStatisticsView::OnGetTotalBuildDur() const
{
	return FText::FromString(FString::Printf(TEXT("%dh %dm %ds"), 50, 41, 36));
}

void SStatisticsView::OnDateSelectionChanged(TSharedPtr<FString> InDate, ESelectInfo::Type)
{
	
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION


#undef LOCTEXT_NAMESPACE

#endif