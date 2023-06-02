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

#include "ApplicationCore/GenericPlatform/Input.h"
#include "ApplicationCore/GenericPlatform/KeyCode.h"
#include "ApplicationCore/GenericPlatform/MouseButtonCode.h"
#include "ApplicationCore/Application.h"
#include "Math/Vector2.h"

#include <stdbool.h>

#include <Windows.h>

typedef struct GCPlatformWindow GCWindowsWindow;

extern HWND GCWindowsWindow_GetWindowHandle(const GCWindowsWindow* const Window);

bool GCInput_IsKeyPressed(const GCKeyCode KeyCode)
{
	return GetAsyncKeyState((int)KeyCode) & 0x8000;
}

bool GCInput_IsMouseButtonPressed(const GCMouseButtonCode MouseButtonCode)
{
	return GetAsyncKeyState((int)MouseButtonCode) & 0x8000;
}

GCVector2 GCInput_GetMousePosition(void)
{
	POINT Position = { 0 };

	if (GetCursorPos(&Position))
	{
		if (ScreenToClient(GCWindowsWindow_GetWindowHandle(GCApplication_GetWindow()), &Position))
		{
			return GCVector2_Create((float)Position.x, (float)Position.y);
		}
	}

	return GCVector2_CreateZero();
}