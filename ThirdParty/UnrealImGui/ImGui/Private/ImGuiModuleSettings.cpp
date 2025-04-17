// Distributed under the MIT License (MIT) (see accompanying LICENSE file)

#include "ImGuiModuleSettings.h"
#include "ImGuiModuleProperties.h"

#include <Misc/ConfigCacheIni.h>


//====================================================================================================
// FImGuiDPIScaleInfo
//====================================================================================================

FImGuiDPIScaleInfo::FImGuiDPIScaleInfo()
{
	// if (FRichCurve* Curve = DPICurve.GetRichCurve())
	// {
	// 	Curve->AddKey(   0.0f, 1.f);
	//
	// 	Curve->AddKey(2159.5f, 1.f);
	// 	Curve->AddKey(2160.0f, 2.f);
	//
	// 	Curve->AddKey(4319.5f, 2.f);
	// 	Curve->AddKey(4320.0f, 4.f);
	// }
}

float FImGuiDPIScaleInfo::CalculateResolutionBasedScale() const
{
	static const float ResolutionBasedScale = 1.f;
	return ResolutionBasedScale;
}

//====================================================================================================
// UImGuiSettings
//====================================================================================================

UImGuiSettings* UImGuiSettings::DefaultInstance = nullptr;

FSimpleMulticastDelegate UImGuiSettings::OnSettingsLoaded;

void UImGuiSettings::PostInitProperties()
{
	Super::PostInitProperties();

	if (IsTemplate())
	{
		DefaultInstance = this;
		OnSettingsLoaded.Broadcast();
	}
}

void UImGuiSettings::BeginDestroy()
{
	Super::BeginDestroy();

	if (DefaultInstance == this)
	{
		DefaultInstance = nullptr;
	}
}

//====================================================================================================
// FImGuiModuleSettings
//====================================================================================================

FImGuiModuleSettings::FImGuiModuleSettings(FImGuiModuleProperties& InProperties)
	: Properties(InProperties)
{
#if WITH_EDITOR
	FCoreUObjectDelegates::OnObjectPropertyChanged.AddRaw(this, &FImGuiModuleSettings::OnPropertyChanged);
#endif

	// Delegate initializer to support settings loaded after this object creation (in stand-alone builds) and potential
	// reloading of settings.
	UImGuiSettings::OnSettingsLoaded.AddRaw(this, &FImGuiModuleSettings::InitializeAllSettings);

	// Call initializer to support settings already loaded (editor).
	InitializeAllSettings();
}

FImGuiModuleSettings::~FImGuiModuleSettings()
{
	UImGuiSettings::OnSettingsLoaded.RemoveAll(this);
}

void FImGuiModuleSettings::InitializeAllSettings()
{
	UpdateSettings();
	UpdateDPIScaleInfo();
}

void FImGuiModuleSettings::UpdateSettings()
{
	if (const UImGuiSettings* SettingsObject = UImGuiSettings::Get())
	{
		SetShareKeyboardInput(SettingsObject->bShareKeyboardInput);
		SetShareMouseInput(SettingsObject->bShareMouseInput);
		SetToggleInputKey(SettingsObject->ToggleInput);
		SetCanvasSizeInfo(SettingsObject->CanvasSize);
	}
}

void FImGuiModuleSettings::UpdateDPIScaleInfo()
{
	if (const UImGuiSettings* SettingsObject = UImGuiSettings::Get())
	{
		SetDPIScaleInfo(SettingsObject->DPIScale);
	}
}

void FImGuiModuleSettings::SetShareKeyboardInput(const bool bShare)
{
	if (bShareKeyboardInput != bShare)
	{
		bShareKeyboardInput = bShare;
		Properties.SetKeyboardInputShared(bShare);
	}
}

void FImGuiModuleSettings::SetShareMouseInput(const bool bShare)
{
	if (bShareMouseInput != bShare)
	{
		bShareMouseInput = bShare;
		Properties.SetMouseInputShared(bShare);
	}
}

void FImGuiModuleSettings::SetToggleInputKey(const FImGuiKeyInfo& KeyInfo)
{
	if (ToggleInputKey != KeyInfo)
	{
		ToggleInputKey = KeyInfo;
	}
}

void FImGuiModuleSettings::SetCanvasSizeInfo(const FImGuiCanvasSizeInfo& CanvasSizeInfo)
{
	if (CanvasSize != CanvasSizeInfo)
	{
		CanvasSize = CanvasSizeInfo;
	}
}

void FImGuiModuleSettings::SetDPIScaleInfo(const FImGuiDPIScaleInfo& ScaleInfo)
{
	DPIScale = ScaleInfo;
}
