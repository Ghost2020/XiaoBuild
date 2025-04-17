// Distributed under the MIT License (MIT) (see accompanying LICENSE file)

#include "ImGuiContextManager.h"
#include "ImGuiImplementation.h"
#include "ImGuiModuleSettings.h"

#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif

#include <imgui.h>

FImGuiContextManager::FImGuiContextManager(FImGuiModuleSettings& InSettings)
	: Settings(InSettings)
{
	SetDPIScale(Settings.GetDPIScaleInfo());
	BuildFontAtlas();
}

FImGuiContextManager::~FImGuiContextManager()
{
	
}

void FImGuiContextManager::Tick(float DeltaSeconds)
{
	for (auto& Pair : Contexts)
	{
		auto& ContextData = Pair.Value;
		if (ContextData.CanTick())
		{
			ContextData.ContextProxy->Tick(DeltaSeconds);
		}
	}

	// Once all context tick they should use new fonts and we can release the old resources. Extra countdown is added
	// wait for contexts that ticked outside of this function, before rebuilding fonts.
	if (FontResourcesReleaseCountdown > 0 && !--FontResourcesReleaseCountdown)
	{
		FontResourcesToRelease.Empty();
	}
}

FImGuiContextManager::FContextData& FImGuiContextManager::GetXiaoContextData()
{
	FContextData* Data = Contexts.Find(XIAO_CONTEXT_INDEX);

	if (UNLIKELY(!Data))
	{
		Data = &Contexts.Emplace(XIAO_CONTEXT_INDEX, FContextData{ TEXT("Xiao"), XIAO_CONTEXT_INDEX, FontAtlas, DPIScale });
		OnContextProxyCreated.Broadcast(XIAO_CONTEXT_INDEX, *Data->ContextProxy);
	}

	return *Data;
}

void FImGuiContextManager::SetDPIScale(const FImGuiDPIScaleInfo& ScaleInfo)
{
	const float Scale = ScaleInfo.GetImGuiScale();
	if (DPIScale != Scale)
	{
		DPIScale = Scale;

		// Only rebuild font atlas if it is already built. Otherwise allow the other logic to pick a moment.
		if (FontAtlas.IsBuilt())
		{
			RebuildFontAtlas();
		}

		for (auto& Pair : Contexts)
		{
			if (Pair.Value.ContextProxy)
			{
				Pair.Value.ContextProxy->SetDPIScale(DPIScale);
			}
		}
	}
}

void FImGuiContextManager::BuildFontAtlas()
{
	if (!FontAtlas.IsBuilt())
	{
		ImFontConfig FontConfig = {};
		FontConfig.SizePixels = FMath::RoundFromZero(13.f * DPIScale);
		FontAtlas.AddFontDefault(&FontConfig);

		OnFontAtlasBuilt.Broadcast();
	}
}

void FImGuiContextManager::RebuildFontAtlas()
{
	if (FontAtlas.IsBuilt())
	{
		// Keep the old resources alive for a few frames to give all contexts a chance to bind to new ones.
		FontResourcesToRelease.Add(MakeUnique<ImFontAtlas>());
		Swap(*FontResourcesToRelease.Last(), FontAtlas);

		// Typically, one frame should be enough but since we allow for custom ticking, we need at least to frames to
		// wait for contexts that already ticked and will not do that before the end of the next tick of this manager.
		FontResourcesReleaseCountdown = 3;
	}

	BuildFontAtlas();
}
