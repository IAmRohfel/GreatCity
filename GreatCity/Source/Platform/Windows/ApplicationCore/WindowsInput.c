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