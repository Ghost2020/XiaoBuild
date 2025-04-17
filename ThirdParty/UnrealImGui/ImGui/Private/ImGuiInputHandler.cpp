// Distributed under the MIT License (MIT) (see accompanying LICENSE file)

#include "ImGuiInputHandler.h"

#include "ImGuiContextProxy.h"
#include "ImGuiInputState.h"
#include "ImGuiModuleDebug.h"
#include "ImGuiModuleManager.h"

#include <Framework/Application/SlateApplication.h>
#include <InputCoreTypes.h>
#include <Input/Events.h>

#include <Framework/Commands/InputBindingManager.h>
#include <Framework/Commands/InputChord.h>


DEFINE_LOG_CATEGORY(LogImGuiInputHandler);

namespace
{
	FReply ToReply(const bool bConsume)
	{
		return bConsume ? FReply::Handled() : FReply::Unhandled();
	}
}

FReply UImGuiInputHandler::OnKeyChar(const struct FCharacterEvent& CharacterEvent)
{
	InputState->AddCharacter(CharacterEvent.GetCharacter());
	return ToReply(!ModuleManager->GetProperties().IsKeyboardInputShared());
}

FReply UImGuiInputHandler::OnKeyDown(const FKeyEvent& KeyEvent)
{
	if (KeyEvent.GetKey().IsGamepadKey())
	{
		bool bConsume = false;
		if (InputState->IsGamepadNavigationEnabled())
		{
			InputState->SetGamepadNavigationKey(KeyEvent, true);
			bConsume = !ModuleManager->GetProperties().IsGamepadInputShared();
		}

		return ToReply(bConsume);
	}
	
	// If there is no active ImGui control that would get precedence and this key event is bound to a stop play session
	// command, then ignore that event and let the command execute.
	if (!HasImGuiActiveItem())
	{
		return ToReply(false);
	}
	const bool bConsume = !ModuleManager->GetProperties().IsKeyboardInputShared();
	// With shared input we can leave command bindings for DebugExec to handle, otherwise we need to do it here.
	if (bConsume)
	{
		ModuleManager->GetProperties().ToggleInput();
	}
	InputState->SetKeyDown(KeyEvent, true);
	CopyModifierKeys(KeyEvent);
	return ToReply(bConsume);
}

FReply UImGuiInputHandler::OnKeyUp(const FKeyEvent& KeyEvent)
{
	if (KeyEvent.GetKey().IsGamepadKey())
	{
		bool bConsume = false;
		if (InputState->IsGamepadNavigationEnabled())
		{
			InputState->SetGamepadNavigationKey(KeyEvent, false);
			bConsume = !ModuleManager->GetProperties().IsGamepadInputShared();
		}

		return ToReply(bConsume);
	}
	
	InputState->SetKeyDown(KeyEvent, false);
	CopyModifierKeys(KeyEvent);
	return ToReply(!ModuleManager->GetProperties().IsKeyboardInputShared());
}

FReply UImGuiInputHandler::OnAnalogValueChanged(const FAnalogInputEvent& AnalogInputEvent)
{
	bool bConsume = false;

	if (AnalogInputEvent.GetKey().IsGamepadKey() && InputState->IsGamepadNavigationEnabled())
	{
		InputState->SetGamepadNavigationAxis(AnalogInputEvent, AnalogInputEvent.GetAnalogValue());
		bConsume = !ModuleManager->GetProperties().IsGamepadInputShared();
	}

	return ToReply(bConsume);
}

FReply UImGuiInputHandler::OnMouseButtonDown(const FPointerEvent& MouseEvent)
{
	if (MouseEvent.IsTouchEvent())
	{
		return ToReply(false);
	}

	InputState->SetMouseDown(MouseEvent, true);
	return ToReply(true);
}

FReply UImGuiInputHandler::OnMouseButtonDoubleClick(const FPointerEvent& MouseEvent)
{
	InputState->SetMouseDown(MouseEvent, true);
	return ToReply(true);
}

FReply UImGuiInputHandler::OnMouseButtonUp(const FPointerEvent& MouseEvent)
{
	if (MouseEvent.IsTouchEvent())
	{
		return ToReply(false);
	}

	InputState->SetMouseDown(MouseEvent, false);
	return ToReply(true);
}

FReply UImGuiInputHandler::OnMouseWheel(const FPointerEvent& MouseEvent)
{
	InputState->AddMouseWheelDelta(MouseEvent.GetWheelDelta());
	return ToReply(true);
}

FReply UImGuiInputHandler::OnMouseMove(const FVector2D& MousePosition, const FPointerEvent& MouseEvent)
{
	if (MouseEvent.IsTouchEvent())
	{
		return ToReply(false);
	}

	return OnMouseMove(MousePosition);
}

FReply UImGuiInputHandler::OnMouseMove(const FVector2D& MousePosition)
{
	InputState->SetMousePosition(MousePosition);
	return ToReply(true);
}

FReply UImGuiInputHandler::OnTouchStarted(const FVector2D& CursorPosition, const FPointerEvent& TouchEvent)
{
	InputState->SetTouchDown(true);
	InputState->SetTouchPosition(CursorPosition);
	return ToReply(true);
}

FReply UImGuiInputHandler::OnTouchMoved(const FVector2D& CursorPosition, const FPointerEvent& TouchEvent)
{
	InputState->SetTouchPosition(CursorPosition);
	return ToReply(true);
}

FReply UImGuiInputHandler::OnTouchEnded(const FVector2D& CursorPosition, const FPointerEvent& TouchEvent)
{
	InputState->SetTouchDown(false);
	return ToReply(true);
}

void UImGuiInputHandler::OnKeyboardInputEnabled()
{
	bKeyboardInputEnabled = true;
}

void UImGuiInputHandler::OnKeyboardInputDisabled()
{
	if (bKeyboardInputEnabled)
	{
		bKeyboardInputEnabled = false;
		InputState->ResetKeyboard();
	}
}

void UImGuiInputHandler::OnMouseInputEnabled()
{
	if (!bMouseInputEnabled)
	{
		bMouseInputEnabled = true;
		UpdateInputStatePointer();
	}
}

void UImGuiInputHandler::OnMouseInputDisabled()
{
	if (bMouseInputEnabled)
	{
		bMouseInputEnabled = false;
		InputState->ResetMouse();
		UpdateInputStatePointer();
	}
}

void UImGuiInputHandler::CopyModifierKeys(const FInputEvent& InputEvent) const
{
	InputState->SetControlDown(InputEvent.IsControlDown());
	InputState->SetShiftDown(InputEvent.IsShiftDown());
	InputState->SetAltDown(InputEvent.IsAltDown());
}

bool UImGuiInputHandler::HasImGuiActiveItem() const
{
	const FImGuiContextProxy* ContextProxy = ModuleManager->GetContextManager().GetContextProxy(ContextIndex);
	return ContextProxy && ContextProxy->HasActiveItem();
}

void UImGuiInputHandler::UpdateInputStatePointer() const
{
	InputState->SetMousePointer(bMouseInputEnabled);
}

void UImGuiInputHandler::OnSoftwareCursorChanged(bool) const
{
	UpdateInputStatePointer();
}

void UImGuiInputHandler::OnPostImGuiUpdate() const
{
	InputState->ClearUpdateState();

	// TODO Replace with delegates after adding property change events.
	InputState->SetKeyboardNavigationEnabled(ModuleManager->GetProperties().IsKeyboardNavigationEnabled());
	InputState->SetGamepadNavigationEnabled(ModuleManager->GetProperties().IsGamepadNavigationEnabled());

	const auto& PlatformApplication = FSlateApplication::Get().GetPlatformApplication();
	InputState->SetGamepad(PlatformApplication.IsValid() && PlatformApplication->IsGamepadAttached());
}

void UImGuiInputHandler::Initialize(FImGuiModuleManager* InModuleManager, const int32 InContextIndex)
{
	ModuleManager = InModuleManager;
	ContextIndex = InContextIndex;

	auto* ContextProxy = ModuleManager->GetContextManager().GetContextProxy(ContextIndex);
	checkf(ContextProxy, TEXT("Missing context during initialization of input handler: ContextIndex = %d"), ContextIndex);
	InputState = &ContextProxy->GetInputState();
}
