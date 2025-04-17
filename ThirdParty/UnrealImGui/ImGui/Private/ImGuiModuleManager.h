/**
  * @author cxx2020@outlook.com
  * @datetime 2024 -8:15 PM
 */
#pragma once

#include "ImGuiContextManager.h"
#include "ImGuiModuleProperties.h"
#include "ImGuiModuleSettings.h"


// Central manager that implements module logic. It initializes and controls remaining module components.
class FImGuiModuleManager
{
	// Allow module to control life-cycle of this class.
	friend class FImGuiModule;

public:
	static FImGuiModuleManager* Get();

	// Get interface to module settings.
	FImGuiModuleSettings& GetSettings() { return Settings; }

	// Get interface to module state properties. 
	FImGuiModuleProperties& GetProperties() { return Properties; }

	// Get ImGui contexts manager.
	FImGuiContextManager& GetContextManager() { return ContextManager; }

private:

	FImGuiModuleManager();
	~FImGuiModuleManager();

	FImGuiModuleManager(const FImGuiModuleManager&) = delete;
	FImGuiModuleManager& operator=(const FImGuiModuleManager&) = delete;

	FImGuiModuleManager(FImGuiModuleManager&&) = delete;
	FImGuiModuleManager& operator=(FImGuiModuleManager&&) = delete;

	void BuildFontAtlasTexture();

	void OnContextProxyCreated(int32 ContextIndex, FImGuiContextProxy& ContextProxy);

	// Collection of module state properties.
	FImGuiModuleProperties Properties;

	// ImGui settings proxy (valid in every loading stage).
	FImGuiModuleSettings Settings;

	// Manager for ImGui contexts.
	FImGuiContextManager ContextManager;

	inline static FImGuiModuleManager* Singleton = nullptr;
};
