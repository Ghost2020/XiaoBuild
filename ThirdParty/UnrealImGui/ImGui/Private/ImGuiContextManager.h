// Distributed under the MIT License (MIT) (see accompanying LICENSE file)

#pragma once

#include "ImGuiContextProxy.h"

class FImGuiModuleSettings;
struct FImGuiDPIScaleInfo;

static constexpr int32 XIAO_CONTEXT_INDEX = -1;

// TODO: It might be useful to broadcast FContextProxyCreatedDelegate to users, to support similar cases to our ImGui
// demo, but we would need to remove from that interface internal classes.

// Delegate called when new context proxy is created.
// @param ContextIndex - Index for that world
// @param ContextProxy - Created context proxy
DECLARE_MULTICAST_DELEGATE_TwoParams(FContextProxyCreatedDelegate, int32, FImGuiContextProxy&);

// Manages ImGui context proxies.
class FImGuiContextManager
{
public:

	FImGuiContextManager(FImGuiModuleSettings& InSettings);

	FImGuiContextManager(const FImGuiContextManager&) = delete;
	FImGuiContextManager& operator=(const FImGuiContextManager&) = delete;

	FImGuiContextManager(FImGuiContextManager&&) = delete;
	FImGuiContextManager& operator=(FImGuiContextManager&&) = delete;

	~FImGuiContextManager();

	ImFontAtlas& GetFontAtlas() { return FontAtlas; }
	const ImFontAtlas& GetFontAtlas() const { return FontAtlas; }
	
	// Get or create editor ImGui context proxy.
	FORCEINLINE FImGuiContextProxy& GetXiaoContextProxy() { return *GetXiaoContextData().ContextProxy; }

	// Get context proxy by index, or null if context with that index doesn't exist.
	FORCEINLINE FImGuiContextProxy* GetContextProxy(const int32 ContextIndex)
	{
		const FContextData* Data = Contexts.Find(ContextIndex);
		return Data ? Data->ContextProxy.Get() : nullptr;
	}

	// Delegate called when a new context proxy is created.
	FContextProxyCreatedDelegate OnContextProxyCreated;

	// Delegate called after font atlas is built.
	FSimpleMulticastDelegate OnFontAtlasBuilt;

	void Tick(float DeltaSeconds);

private:

	struct FContextData
	{
		FContextData(const FString& ContextName, const int32 ContextIndex, ImFontAtlas& FontAtlas, const float DPIScale)
			: ContextProxy(new FImGuiContextProxy(ContextName, ContextIndex, &FontAtlas, DPIScale))
		{
		}

		FORCEINLINE bool CanTick() const { return true;/*return PIEInstance < 0 || GEngine->GetWorldContextFromPIEInstance(PIEInstance);*/ }

		int32 PIEInstance = -1;
		TUniquePtr<FImGuiContextProxy> ContextProxy;
	};
	
	FContextData& GetXiaoContextData();

	void SetDPIScale(const FImGuiDPIScaleInfo& ScaleInfo);
	void BuildFontAtlas();
	void RebuildFontAtlas();

	TMap<int32, FContextData> Contexts;

	ImFontAtlas FontAtlas;
	TArray<TUniquePtr<ImFontAtlas>> FontResourcesToRelease;

	FImGuiModuleSettings& Settings;

	float DPIScale = 1.0f;//-1.f;
	int32 FontResourcesReleaseCountdown = 0;
};
