/**
  * @author cxx2020@outlook.com
  * @datetime 2024 -8:15 PM
 */
#include "ImGuiModule.h"

#include "ImGuiModuleManager.h"
#include "ImGuiImplementation.h"


#define LOCTEXT_NAMESPACE "FImGuiModule"


struct EDelegateCategory
{
	enum
	{
		// Default per-context draw events.
		Default,

		// Multi-context draw event defined in context manager.
		MultiContext
	};
};

FImGuiModuleManager* ImGuiModuleManager = nullptr;

void FImGuiModule::StartupModule()
{
	// Initialize handles to allow cross-module redirections. Other handles will always look for parents in the active
	// module, which means that we can only redirect to started modules. We don't have to worry about self-referencing
	// as local handles are guaranteed to be constructed before initializing pointers.
	// This supports in-editor recompilation and hot-reloading after compiling from the command line. The latter method
	// theoretically doesn't support plug-ins and will not load re-compiled module, but its handles will still redirect
	// to the active one.

	ImGuiContextHandle = &ImGuiImplementation::GetContextHandle();

	// Create managers that implements module logic.

	checkf(!ImGuiModuleManager, TEXT("Instance of the ImGui Module Manager already exists. Instance should be created only during module startup."));
	ImGuiModuleManager = new FImGuiModuleManager();

	ImGuiModuleManager->GetContextManager().GetXiaoContextProxy();
}

void FImGuiModule::ShutdownModule()
{
	// Before we shutdown we need to delete managers that will do all the necessary cleanup.
	checkf(ImGuiModuleManager, TEXT("Null ImGui Module Manager. Module manager instance should be deleted during module shutdown."));
	delete ImGuiModuleManager;
	ImGuiModuleManager = nullptr;
	
	// When shutting down we leave the global ImGui context pointer and handle pointing to resources that are already
	// deleted. This can cause troubles after hot-reload when code in other modules calls ImGui interface functions
	// which are statically bound to the obsolete module. To keep ImGui code functional we can redirect context handle
	// to point to the new module.

	// When shutting down during hot-reloading, we might want to rewire handles used in statically bound functions
	// or move data to a new module.

	FModuleManager::Get().OnModulesChanged().AddLambda([this] (FName Name, EModuleChangeReason Reason)
	{
		if (Reason == EModuleChangeReason::ModuleLoaded && Name == "ImGui")
		{
			const FImGuiModule& LoadedModule = FImGuiModule::Get();
			if (&LoadedModule != this)
			{
				// Statically bound functions can be bound to the obsolete module, so we need to manually redirect.

				if (LoadedModule.ImGuiContextHandle)
				{
					ImGuiImplementation::SetParentContextHandle(*LoadedModule.ImGuiContextHandle);
				}
			}
		}
	});
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FImGuiModule, ImGui)
