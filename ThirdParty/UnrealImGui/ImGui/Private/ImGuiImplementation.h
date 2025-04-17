// Distributed under the MIT License (MIT) (see accompanying LICENSE file)

#pragma once

struct FImGuiContextHandle;

// Gives access to selected ImGui implementation features.
namespace ImGuiImplementation
{
	// Get the handle to the ImGui Context pointer.
	FImGuiContextHandle& GetContextHandle();

	// Set the ImGui Context pointer handle.
	void SetParentContextHandle(FImGuiContextHandle& Parent);
}
