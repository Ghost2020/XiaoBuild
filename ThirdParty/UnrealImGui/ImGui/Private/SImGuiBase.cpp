// Distributed under the MIT License (MIT) (see accompanying LICENSE file)

#include "SImGuiBase.h"
#include "SImGuiCanvasControl.h"

#include "ImGuiInputHandler.h"
#include "ImGuiInputHandlerFactory.h"
#include "ImGuiModuleManager.h"

#include <Framework/Application/SlateApplication.h>
#include "Fonts/FontMeasure.h"
#include <SlateOptMacros.h>
#include <Widgets/SViewport.h>

#include "ImGuiModuleSettings.h"

namespace
{
	FORCEINLINE FVector2D MaxVector(const FVector2D& A, const FVector2D& B)
	{
		return FVector2D(FMath::Max(A.X, B.X), FMath::Max(A.Y, B.Y));
	}

	FORCEINLINE FVector2D RoundVector(const FVector2D& Vector)
	{
		return FVector2D(FMath::RoundToFloat(Vector.X), FMath::RoundToFloat(Vector.Y));
	}

	FORCEINLINE FSlateRenderTransform RoundTranslation(const FSlateRenderTransform& Transform)
	{
		return FSlateRenderTransform(Transform.GetMatrix(), RoundVector(Transform.GetTranslation()));
	}
}

// BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SImGuiBase::Construct(const FArguments& InArgs)
{
	checkf(InArgs._Viewport, TEXT("Null Viewport argument"));

	ModuleManager = FImGuiModuleManager::Get();
	Viewport = InArgs._Viewport;
	ContextIndex = InArgs._ContextIndex;

	CreateInputHandler();

	// Initialize state.
	UpdateVisibility();
	UpdateMouseCursor();

	ChildSlot
	[
		SAssignNew(CanvasControlWidget, SImGuiCanvasControl).OnTransformChanged(this, &SImGuiBase::SetImGuiTransform)
	];

	ImGuiTransform = CanvasControlWidget->GetTransform();
}

SImGuiBase::~SImGuiBase()
{
	// Release ImGui Input Handler.
	ReleaseInputHandler();
}

void SImGuiBase::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	Super::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);

	UpdateInputState();
	OnPostImGuiUpdate();
	UpdateTransparentMouseInput(AllottedGeometry);
	HandleWindowFocusLost();
	UpdateCanvasSize();
	OnDraw();
}

FReply SImGuiBase::OnKeyChar(const FGeometry& MyGeometry, const FCharacterEvent& CharacterEvent)
{
	return InputHandler->OnKeyChar(CharacterEvent);
}

FReply SImGuiBase::OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& KeyEvent)
{
	UpdateCanvasControlMode(KeyEvent);
	return InputHandler->OnKeyDown(KeyEvent);
}

FReply SImGuiBase::OnKeyUp(const FGeometry& MyGeometry, const FKeyEvent& KeyEvent)
{
	UpdateCanvasControlMode(KeyEvent);
	return InputHandler->OnKeyUp(KeyEvent);
}

FReply SImGuiBase::OnAnalogValueChanged(const FGeometry& MyGeometry, const FAnalogInputEvent& AnalogInputEvent)
{
	return InputHandler->OnAnalogValueChanged(AnalogInputEvent);
}

FReply SImGuiBase::OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	return InputHandler->OnMouseButtonDown(MouseEvent).LockMouseToWidget(SharedThis(this));
}

FReply SImGuiBase::OnMouseButtonDoubleClick(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	return InputHandler->OnMouseButtonDoubleClick(MouseEvent).LockMouseToWidget(SharedThis(this));
}

FReply SImGuiBase::OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	FReply Reply = InputHandler->OnMouseButtonUp(MouseEvent);
	if (FSlateApplication::Get().GetPressedMouseButtons().Num() <= 0)
	{
		Reply.ReleaseMouseLock();
	}
	return Reply;
}

FReply SImGuiBase::OnMouseWheel(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	return InputHandler->OnMouseWheel(MouseEvent);
}

FReply SImGuiBase::OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	return InputHandler->OnMouseMove(TransformScreenPointToImGui(MyGeometry, MouseEvent.GetScreenSpacePosition()), MouseEvent);
}

FReply SImGuiBase::OnFocusReceived(const FGeometry& MyGeometry, const FFocusEvent& FocusEvent)
{
	Super::OnFocusReceived(MyGeometry, FocusEvent);

	const auto Window = FSlateApplication::Get().FindWidgetWindow(SharedThis(this));
	bForegroundWindow = Window->GetNativeWindow()->IsForegroundWindow();
	InputHandler->OnKeyboardInputEnabled();

	FSlateApplication::Get().ResetToDefaultPointerInputSettings();
	return FReply::Handled();
}

void SImGuiBase::OnFocusLost(const FFocusEvent& FocusEvent)
{
	Super::OnFocusLost(FocusEvent);

	InputHandler->OnKeyboardInputDisabled();
}

void SImGuiBase::OnMouseEnter(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	Super::OnMouseEnter(MyGeometry, MouseEvent);

	InputHandler->OnMouseInputEnabled();
}

void SImGuiBase::OnMouseLeave(const FPointerEvent& MouseEvent)
{
	Super::OnMouseLeave(MouseEvent);

	InputHandler->OnMouseInputDisabled();
}

FReply SImGuiBase::OnTouchStarted(const FGeometry& MyGeometry, const FPointerEvent& TouchEvent)
{
	return InputHandler->OnTouchStarted(TransformScreenPointToImGui(MyGeometry, TouchEvent.GetScreenSpacePosition()), TouchEvent);
}

FReply SImGuiBase::OnTouchMoved(const FGeometry& MyGeometry, const FPointerEvent& TouchEvent)
{
	return InputHandler->OnTouchMoved(TransformScreenPointToImGui(MyGeometry, TouchEvent.GetScreenSpacePosition()), TouchEvent);
}

FReply SImGuiBase::OnTouchEnded(const FGeometry& MyGeometry, const FPointerEvent& TouchEvent)
{
	UpdateVisibility();
	return InputHandler->OnTouchEnded(TransformScreenPointToImGui(MyGeometry, TouchEvent.GetScreenSpacePosition()), TouchEvent);
}

int32 SImGuiBase::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyClippingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& WidgetStyle, bool bParentEnabled) const
{
	if (FImGuiContextProxy* ContextProxy = ModuleManager->GetContextManager().GetContextProxy(ContextIndex))
	{
		// Manually update ImGui context to minimise lag between creating and rendering ImGui output. This will also
		// keep frame tearing at minimum because it is executed at the very end of the frame.
		ContextProxy->Tick(FSlateApplication::Get().GetDeltaTime());

		// Calculate transform from ImGui to screen space. Rounding translation is necessary to keep it pixel-perfect
		// in older engine versions.
		const FSlateRenderTransform& WidgetToScreen = AllottedGeometry.GetAccumulatedRenderTransform();
		const FSlateRenderTransform ImGuiToScreen = RoundTranslation(ImGuiRenderTransform.Concatenate(WidgetToScreen));

		for (const auto& DrawList : ContextProxy->GetDrawData())
		{
			DrawList.CopyVertexData(VertexBuffer, ImGuiToScreen);

			int IndexBufferOffset = 0;
			for (int CommandNb = 0; CommandNb < DrawList.NumCommands(); CommandNb++)
			{
				const auto& DrawCommand = DrawList.GetCommand(CommandNb, ImGuiToScreen);

				DrawList.CopyIndexData(IndexBuffer, IndexBufferOffset, DrawCommand.NumElements);

				// Advance offset by number of copied elements to position it for the next command.
				IndexBufferOffset += DrawCommand.NumElements;

				// Transform clipping rectangle to screen space and apply to elements that we draw.
				const FSlateRect ClippingRect = DrawCommand.ClippingRect.IntersectionWith(MyClippingRect);

				OutDrawElements.PushClip(FSlateClippingZone{ ClippingRect });

				// Add elements to the list.
				FSlateDrawElement::MakeCustomVerts(OutDrawElements, LayerId, ResourceHandle, VertexBuffer, IndexBuffer, nullptr, 0, 0);

				OutDrawElements.PopClip();
			}
		}

		const FSlateFontInfo FontInfo = FSlateFontInfo(FCoreStyle::GetDefaultFont(), 10);
		const TSharedRef<FSlateFontMeasure> FontMeasureService = FSlateApplication::Get().GetRenderer()->GetFontMeasureService();
		const uint32 TextLayerId = LayerId + 1;
		for (const auto& Text : ImGui::GTextArray)
		{
			const FVector2D TextSize = FontMeasureService->Measure(Text.Text, FontInfo);
			const FPaintGeometry PaintGeometry = AllottedGeometry.ToPaintGeometry(Text.Pos, TextSize);
			FSlateDrawElement::MakeText(OutDrawElements, TextLayerId, PaintGeometry, Text.Text, FontInfo);
		}
	}

	ImGui::GTextArray.Empty();

	return Super::OnPaint(Args, AllottedGeometry, MyClippingRect, OutDrawElements, LayerId+2, WidgetStyle, bParentEnabled);
}

void SImGuiBase::CreateInputHandler()
{
	ReleaseInputHandler();

	if (!InputHandler.IsValid())
	{
		InputHandler = FImGuiInputHandlerFactory::NewHandler(ModuleManager,  ContextIndex);
	}
}

void SImGuiBase::ReleaseInputHandler()
{
	if (InputHandler.IsValid())
	{
		FImGuiInputHandlerFactory::ReleaseHandler(InputHandler.Get());
		InputHandler.Reset();
	}
}

void SImGuiBase::UpdateVisibility()
{
	// Make sure that we do not occlude other widgets, if input is disabled or if mouse is set to work in a transparent
	// mode (hit-test invisible).
	SetVisibility(!bTransparentMouseInput ? EVisibility::Visible : EVisibility::HitTestInvisible);
}

void SImGuiBase::UpdateMouseCursor()
{
	if (!bHideMouseCursor)
	{
		const FImGuiContextProxy* ContextProxy = ModuleManager->GetContextManager().GetContextProxy(ContextIndex);
		SetCursor(ContextProxy ? ContextProxy->GetMouseCursor() : EMouseCursor::Default);
	}
	else
	{
		SetCursor(EMouseCursor::None);
	}
}

void SImGuiBase::TakeFocus()
{
	auto& SlateApplication = FSlateApplication::Get();

	PreviousUserFocusedWidget = SlateApplication.GetUserFocusedWidget(SlateApplication.GetUserIndexForKeyboard());
	
	SlateApplication.SetKeyboardFocus(SharedThis(this));
}

void SImGuiBase::ReturnFocus()
{
	if (HasKeyboardFocus())
	{
		const auto FocusWidgetPtr = PreviousUserFocusedWidget.IsValid() ? PreviousUserFocusedWidget.Pin() : Viewport.Pin();
		
		auto& SlateApplication = FSlateApplication::Get();
		SlateApplication.ResetToDefaultPointerInputSettings();
		SlateApplication.SetUserFocus(SlateApplication.GetUserIndexForKeyboard(), FocusWidgetPtr);
	}

	PreviousUserFocusedWidget.Reset();
}

void SImGuiBase::UpdateInputState()
{
	auto& Properties = ModuleManager->GetProperties();
	const auto* ContextProxy = ModuleManager->GetContextManager().GetContextProxy(ContextIndex);

	const bool bEnableTransparentMouseInput = Properties.IsMouseInputShared() && !(ContextProxy->WantsMouseCapture() || ContextProxy->HasActiveItem());
	if (bTransparentMouseInput != bEnableTransparentMouseInput)
	{
		bTransparentMouseInput = bEnableTransparentMouseInput;
		
		UpdateVisibility();
	}

	const bool bEnableInput = Properties.IsInputEnabled();
	// if (bInputEnabled != bEnableInput)
	// {
	// 	bInputEnabled = bEnableInput;
	//
	// 	UpdateVisibility();
	// 	UpdateMouseCursor();
	//
	// 	
	// 	// We won't get mouse enter, if viewport is already hovered.
	// 	if (Viewport.Pin()->IsHovered())
	// 	{
	// 		InputHandler->OnMouseInputEnabled();
	// 	}
	// 	TakeFocus();
	// }
	// else if(bInputEnabled)
	// {
		if (bTransparentMouseInput)
		{
			// If mouse is in transparent input mode and focus is lost to viewport, let viewport keep it and disable
			// the whole input to match that state.
			if (Viewport.Pin()->HasMouseCapture())
			{
				Properties.SetInputEnabled(false);
				UpdateInputState();
			}
		}
		else
		{
			// Widget tends to lose keyboard focus after console is opened. With non-transparent mouse we can fix that
			// by manually restoring it.
			if (!HasKeyboardFocus() && (Viewport.Pin()->HasKeyboardFocus() || Viewport.Pin()->HasFocusedDescendants()))
			{
				TakeFocus();
			}
		}
	// }
}

void SImGuiBase::UpdateTransparentMouseInput(const FGeometry& AllottedGeometry) const
{
	if (bTransparentMouseInput)
	{
		if (!Viewport.Pin()->HasMouseCapture())
		{
			InputHandler->OnMouseMove(TransformScreenPointToImGui(AllottedGeometry, FSlateApplication::Get().GetCursorPos()));
		}
	}
}

void SImGuiBase::HandleWindowFocusLost()
{
	// We can use window foreground status to notify about application losing or receiving focus. In some situations
	// we get mouse leave or enter events, but they are only sent if mouse pointer is inside of the viewport.
	if (HasKeyboardFocus())
	{
		const auto Window = FSlateApplication::Get().FindWidgetWindow(SharedThis(this));
		if (bForegroundWindow != Window->GetNativeWindow()->IsForegroundWindow())
		{
			bForegroundWindow = !bForegroundWindow;

			if (bForegroundWindow)
			{
				InputHandler->OnKeyboardInputEnabled();
			}
			else
			{
				InputHandler->OnKeyboardInputDisabled();
			}
		}
	}
}

void SImGuiBase::UpdateCanvasSize()
{
	if (bUpdateCanvasSize)
	{
		if (auto* ContextProxy = ModuleManager->GetContextManager().GetContextProxy(ContextIndex))
		{
			CanvasSize = MinCanvasSize;
			if (bAdaptiveCanvasSize && Viewport.IsValid())
			{
				const FVector2D ViewportSize = Viewport.Pin()->ComputeDesiredSize(1.0f);;
				CanvasSize = MaxVector(CanvasSize, ViewportSize);
			}
			else
			{
				// No need for more updates, if we successfully processed fixed-canvas size.
				bUpdateCanvasSize = false;
			}

			// Clamping DPI Scale to keep the canvas size from getting too big.
			CanvasSize /= FMath::Max(DPIScale, 0.01f);
			CanvasSize = RoundVector(CanvasSize);

			ContextProxy->SetDisplaySize(CanvasSize);
		}
	}
}

void SImGuiBase::UpdateCanvasControlMode(const FInputEvent& InputEvent) const
{
	if (bCanvasControlEnabled)
	{
		CanvasControlWidget->SetActive(InputEvent.IsLeftAltDown() && InputEvent.IsLeftShiftDown());
	}
}

void SImGuiBase::OnPostImGuiUpdate()
{
	ImGuiRenderTransform = ImGuiTransform;
	UpdateMouseCursor();
}

FVector2D SImGuiBase::TransformScreenPointToImGui(const FGeometry& MyGeometry, const FVector2D& Point) const
{
	const FSlateRenderTransform ImGuiToScreen = ImGuiTransform.Concatenate(MyGeometry.GetAccumulatedRenderTransform());
	return ImGuiToScreen.Inverse().TransformPoint(Point);
}

FVector2D SImGuiBase::ComputeDesiredSize(const float Scale) const
{
	return CanvasSize * Scale;
}

// END_SLATE_FUNCTION_BUILD_OPTIMIZATION