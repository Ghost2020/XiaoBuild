/**
  * @author cxx2020@outlook.com
  * @datetime 2024 -8:15 PM
 */
#include "ImGuiModuleManager.h"

#include "ImGuiInteroperability.h"

#include <Framework/Application/SlateApplication.h>
#include <Modules/ModuleManager.h>

#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif

#include <imgui.h>


FImGuiModuleManager* FImGuiModuleManager::Get()
{
	return Singleton;
}

FImGuiModuleManager::FImGuiModuleManager()
	: Settings(Properties)
	, ContextManager(Settings)
{
	// Register in context manager to get information whenever a new context proxy is created.
	ContextManager.OnContextProxyCreated.AddRaw(this, &FImGuiModuleManager::OnContextProxyCreated);
	
	Singleton = this;

	BuildFontAtlasTexture();
}

FImGuiModuleManager::~FImGuiModuleManager()
{
	ContextManager.OnFontAtlasBuilt.RemoveAll(this);
}

void FImGuiModuleManager::BuildFontAtlasTexture()
{
	// Create a font atlas texture.
	ImFontAtlas& Fonts = ContextManager.GetFontAtlas();

	unsigned char* Pixels;
	int Width, Height, Bpp;
	Fonts.GetTexDataAsRGBA32(&Pixels, &Width, &Height, &Bpp);
}

void FImGuiModuleManager::OnContextProxyCreated(int32 ContextIndex, FImGuiContextProxy& ContextProxy)
{
	ContextProxy.OnDraw().AddLambda([this, ContextIndex]() {  });
}
