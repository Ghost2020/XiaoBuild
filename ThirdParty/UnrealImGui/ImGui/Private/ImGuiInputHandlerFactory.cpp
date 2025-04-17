// Distributed under the MIT License (MIT) (see accompanying LICENSE file)

#include "ImGuiInputHandlerFactory.h"
#include "ImGuiInputHandler.h"

#include "ImGuiModuleDebug.h"

#include <InputCoreTypes.h>


UImGuiInputHandler* FImGuiInputHandlerFactory::NewHandler(FImGuiModuleManager* ModuleManager, const int32 ContextIndex)
{
	const UClass* HandlerClass = nullptr;
	if (!HandlerClass)
	{
		HandlerClass = UImGuiInputHandler::StaticClass();
	}

	UImGuiInputHandler* Handler = NewObject<UImGuiInputHandler>();
	if (Handler)
	{
		Handler->Initialize(ModuleManager, ContextIndex);
		Handler->AddToRoot();
	}
	else
	{
		UE_LOG(LogImGuiInputHandler, Error, TEXT("Failed attempt to create Input Handler: HandlerClass = %s."), *GetNameSafe(HandlerClass));
	}

	return Handler;
}

void FImGuiInputHandlerFactory::ReleaseHandler(UImGuiInputHandler* Handler)
{
	if (Handler)
	{
		Handler->RemoveFromRoot();
	}
}
