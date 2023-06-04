/*
	Copyright (C) 2023  Rohfel Adyaraka Christianugrah Puspoasmoro

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "ApplicationCore/Application.h"

#include <imgui.h>
#include <imgui_impl_win32.h>

#include <Windows.h>

typedef struct GCPlatformWindow GCWindowsWindow;

extern "C" void GCWindowsWindow_SetAnotherMessageCallback(GCWindowsWindow* const Window, LRESULT(*MessageCallback)(HWND, UINT, WPARAM, LPARAM));
extern "C" HWND GCWindowsWindow_GetWindowHandle(const GCWindowsWindow* const Window);

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

static LRESULT GCImGuiManager_MessageCallback(HWND WindowHandle, UINT Message, WPARAM WParam, LPARAM LParam);

extern "C" void GCImGuiManager_InitializePlatform(void)
{
	GCWindow* const Window = GCApplication_GetWindow();

	ImGui_ImplWin32_Init(GCWindowsWindow_GetWindowHandle(Window));
	GCWindowsWindow_SetAnotherMessageCallback(Window, GCImGuiManager_MessageCallback);
}

extern "C" void GCImGuiManager_BeginFramePlatform(void)
{
	ImGui_ImplWin32_NewFrame();
}

extern "C" void GCImGuiManager_TerminatePlatform(void)
{
	ImGui_ImplWin32_Shutdown();
}

LRESULT GCImGuiManager_MessageCallback(HWND WindowHandle, UINT Message, WPARAM WParam, LPARAM LParam)
{
	return ImGui_ImplWin32_WndProcHandler(WindowHandle, Message, WParam, LParam);
}