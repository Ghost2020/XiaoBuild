/**
  * @author cxx2020@outlook.com
  * @datetime 2024 -8:15 PM
 */
#pragma once

#include <Modules/ModuleManager.h>


class FImGuiModule : public IModuleInterface
{
public:

	/**
	 * Singleton-like access to this module's interface. This is just for convenience!
	 * Beware of calling this during the shutdown phase, though. Your module might have been unloaded already.
	 *
	 * @return Returns singleton instance, loading the module on demand if needed
	 */
	static inline FImGuiModule& Get()
	{
		return FModuleManager::LoadModuleChecked<FImGuiModule>("ImGui");
	}

	/**
	 * Checks to see if this module is loaded and ready. It is only valid to call Get() if IsAvailable() returns true.
	 *
	 * @return True if the module is loaded and ready to use
	 */
	static inline bool IsAvailable()
	{
		return FModuleManager::Get().IsModuleLoaded("ImGui");
	}

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:
	struct FImGuiContextHandle* ImGuiContextHandle = nullptr;
	friend struct FImGuiContextHandle;
};
