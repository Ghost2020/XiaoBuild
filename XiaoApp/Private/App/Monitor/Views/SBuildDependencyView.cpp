/**
  * @author cxx2020@outlook.com
  * @datetime 2024 -8:15 PM
 */
#include "SBuildDependencyView.h"
#ifdef USE_IMGUI
#include "XiaoShareField.h"
#include "XiaoStyle.h"

#include "SlateOptMacros.h"
#include "Widgets/SViewport.h"
#include "../Widgets/SDependencyWidget.h"
#include "Dom/JsonObject.h"
#include "Dom/JsonValue.h"

#define LOCTEXT_NAMESPACE "SBuildDependencyView"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SBuildDependencyView::Construct(const FArguments& InArgs)
{
	Viewport = SNew(SViewport).ViewportSize_Lambda([this]()
	{
		const TSharedPtr<SWindow> Window = FSlateApplication::Get().FindWidgetWindow(SharedThis(this));
		if (Window.IsValid())
		{
			return Window->GetViewportSize();
		}
		return Viewport->GetDesiredSize();
	});
	Viewport->SetRenderDirectlyToWindow(false);
	
	Viewport->SetContent(
		SAssignNew(DependencyWidget, SDependencyWidget)
		.Viewport(Viewport)
		.ContextIndex(-1)
	);
	ChildSlot
	[
		SNew(SOverlay)
		+SOverlay::Slot().HAlign(HAlign_Fill).VAlign(VAlign_Fill)
		[
			Viewport.ToSharedRef()
		]
	]
	.VAlign(VAlign_Fill).HAlign(HAlign_Fill);
}

bool SBuildDependencyView::LoadFromJsonObject(const TArray<TSharedPtr<FJsonValue>>*& InActionArray) const
{
	if (!InActionArray)
	{
		return false;
	}
	
	DependencyWidget->DependencyGraph->Reset();
	for (const auto Action : *InActionArray)
	{
		if (!Action.IsValid())
		{
			continue;
		}

		const TSharedPtr<FJsonObject>& TrackObject = Action->AsObject();
		if (!TrackObject.IsValid())
		{
			continue;
		}
		
		for (const auto ActionValue : TrackObject->GetArrayField(SAction))
		{
			if(!ActionValue.IsValid())
			{
				continue;		
			}
			const TSharedPtr<FJsonObject>& ActionObject = ActionValue->AsObject();

			int32 NodeId = 0;
			if (!ActionObject->TryGetNumberField(SId, NodeId))
			{
				continue;
			}
			FString NodeTitle;
			if (!ActionObject->TryGetStringField(SName, NodeTitle))
			{
				continue;
			}
			
			TArray<int32> DependencyActions;
			TArray<FString> StrArray;
			FString Value = ActionObject->GetStringField(SDependentActions);
			Value.ParseIntoArray(StrArray, *SSepetator);
			for (const FString Str : StrArray)
			{
				DependencyActions.Add(FCString::Atoi(*Str.Replace(TEXT("Tool"), TEXT(""))));
			}

			FString TempStr;
			TArray<int32> QuotedActions;
			if (ActionObject->TryGetStringField(SQuotedActions, TempStr))
			{
				StrArray.Empty();
				Value.ParseIntoArray(StrArray, *SSepetator);
				for (const FString Str : StrArray)
				{
					QuotedActions.Add(FCString::Atoi(*Str));
				}
			}
			DependencyWidget->DependencyGraph->AddNode(NodeId, NodeTitle, DependencyActions, QuotedActions);
		}
	}
	
	DependencyWidget->DependencyGraph->RebuildGraph();
	return true;
}

void SBuildDependencyView::UpdateDependency(const FJsonObject* InObject)
{

}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION

#undef LOCTEXT_NAMESPACE

#endif