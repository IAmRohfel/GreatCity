#include "ApplicationCore/GenericPlatform/Window.h"
#include "ApplicationCore/GenericPlatform/MouseButtonCode.h"
#include "ApplicationCore/Event/Event.h"
#include "ApplicationCore/Event/ApplicationEvent.h"
#include "ApplicationCore/Event/KeyEvent.h"
#include "ApplicationCore/Event/MouseEvent.h"
#include "Core/Memory/Allocator.h"
#include "Core/Container/String.h"

#include <stdint.h>

#include <Windows.h>

typedef struct GCPlatformWindow
{
	GCWindowProperties Properties;
	LPCWSTR ClassName;
	HINSTANCE InstanceHandle;
	HWND WindowHandle;
} GCWindow, GCWindowsWindow;

static GCWindowsWindow* GCWindowsWindow_Create(const GCWindowProperties* const Properties);
static LRESULT CALLBACK GCWindowsWindow_SetupMessageHandler(HWND WindowHandle, UINT Message, WPARAM WParam, LPARAM LParam);
static LRESULT CALLBACK GCWindowsWindow_MessageHandler(HWND WindowHandle, UINT Message, WPARAM WParam, LPARAM LParam);
static void GCWindowsWindow_ProcessEvents(GCWindowsWindow* const Window);
static void GCWindowsWindow_GetWindowSize(const GCWindowsWindow* const Window, uint32_t* const Width, uint32_t* const Height);
static void GCWindowsWindow_Destroy(GCWindowsWindow* Window);

HWND GCWindowsWindow_GetWindowHandle(const GCWindowsWindow* const Window);
HINSTANCE GCWindowsWindow_GetInstanceHandle(const GCWindowsWindow* const Window);

GCWindow* GCWindow_Create(const GCWindowProperties* const Properties)
{
	return GCWindowsWindow_Create(Properties);
}

void GCWindow_ProcessEvents(GCWindow* const Window)
{
	GCWindowsWindow_ProcessEvents(Window);
}

void GCWindow_GetWindowSize(const GCWindow* const Window, uint32_t* const Width, uint32_t* const Height)
{
	GCWindowsWindow_GetWindowSize(Window, Width, Height);
}

void GCWindow_Destroy(GCWindow* Window)
{
	GCWindowsWindow_Destroy(Window);
}

GCWindowsWindow* GCWindowsWindow_Create(const GCWindowProperties* const Properties)
{
	GCWindowsWindow* Window = (GCWindowsWindow*)GCMemory_Allocate(sizeof(GCWindowsWindow));
	Window->Properties = *Properties;
	Window->ClassName = L"GreatCityWindow";
	Window->InstanceHandle = GetModuleHandleW(NULL);
	Window->WindowHandle = NULL;

	WNDCLASSEXW WindowClass;
	ZeroMemory(&WindowClass, sizeof(WNDCLASSEXW));

	WindowClass.cbSize = sizeof(WNDCLASSEXW);
	WindowClass.style = CS_OWNDC;
	WindowClass.lpfnWndProc = GCWindowsWindow_SetupMessageHandler;
	WindowClass.cbClsExtra = 0;
	WindowClass.cbWndExtra = 0;
	WindowClass.hInstance = Window->InstanceHandle;
	WindowClass.hIcon = NULL;
	WindowClass.hCursor = NULL;
	WindowClass.hbrBackground = NULL;
	WindowClass.lpszMenuName = NULL;
	WindowClass.lpszClassName = Window->ClassName;
	WindowClass.hIconSm = NULL;

	RegisterClassExW(&WindowClass);

	wchar_t* WindowTitleUTF16 = GCString_UTF8ToUTF16(Window->Properties.Title);
	Window->WindowHandle = CreateWindowExW(0, Window->ClassName, WindowTitleUTF16, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, Window->Properties.Width, Window->Properties.Height, NULL, NULL, Window->InstanceHandle, Window);
	GCMemory_Free(WindowTitleUTF16);

	ShowWindow(Window->WindowHandle, SW_SHOW);

	return Window;
}

LRESULT CALLBACK GCWindowsWindow_SetupMessageHandler(HWND WindowHandle, UINT Message, WPARAM WParam, LPARAM LParam)
{
	if (Message == WM_NCCREATE)
	{
		const CREATESTRUCTW* const CreateStruct = (const CREATESTRUCTW* const)LParam;
		const GCWindowsWindow* const Window = (const GCWindowsWindow* const)CreateStruct->lpCreateParams;

		SetWindowLongPtrW(WindowHandle, GWLP_USERDATA, (LONG_PTR)Window);
		SetWindowLongPtrW(WindowHandle, GWLP_WNDPROC, (LONG_PTR)GCWindowsWindow_MessageHandler);

		return GCWindowsWindow_MessageHandler(WindowHandle, Message, WParam, LParam);
	}

	return DefWindowProcW(WindowHandle, Message, WParam, LParam);
}

LRESULT CALLBACK GCWindowsWindow_MessageHandler(HWND WindowHandle, UINT Message, WPARAM WParam, LPARAM LParam)
{
	GCWindowsWindow* const Window = (GCWindowsWindow* const)GetWindowLongPtrW(WindowHandle, GWLP_USERDATA);

	switch (Message)
	{
		case WM_KEYDOWN:
		case WM_SYSKEYDOWN:
		case WM_KEYUP:
		case WM_SYSKEYUP:
		{
			WORD KeyCode = LOWORD(WParam);
			const WORD KeyFlags = HIWORD(LParam);
			WORD ScanCode = LOBYTE(KeyFlags);
			const BOOL IsExtendedKey = (KeyFlags & KF_EXTENDED) == KF_EXTENDED;

			if (IsExtendedKey)
			{
				ScanCode = MAKEWORD(ScanCode, 0xE0);
			}

			switch (KeyCode)
			{
				case VK_SHIFT:
				case VK_CONTROL:
				case VK_MENU:
				{
					KeyCode = LOWORD(MapVirtualKeyW(ScanCode, MAPVK_VSC_TO_VK_EX));

					break;
				}
			}

			if (Message == WM_KEYDOWN || Message == WM_SYSKEYDOWN)
			{
				const GCKeyPressedEvent EventDetail = GCKeyPressedEvent_Create((GCKeyCode)KeyCode);
				GCEvent Event = GCEvent_Create(GCEventType_KeyPressed, &EventDetail);
				Window->Properties.EventCallback(Window, &Event);
			}
			else if (Message == WM_KEYUP || Message == WM_SYSKEYUP)
			{
				const GCKeyReleasedEvent EventDetail = GCKeyReleasedEvent_Create((GCKeyCode)KeyCode);
				GCEvent Event = GCEvent_Create(GCEventType_KeyReleased, &EventDetail);
				Window->Properties.EventCallback(Window, &Event);
			}

			break;
		}

		case WM_LBUTTONDOWN:
		{
			const GCMouseButtonPressedEvent EventDetail = GCMouseButtonPressedEvent_Create(GCMouseButtonCode_Left);
			GCEvent Event = GCEvent_Create(GCEventType_MouseButtonPressed, &EventDetail);
			Window->Properties.EventCallback(Window, &Event);

			break;
		}
		case WM_RBUTTONDOWN:
		{
			const GCMouseButtonPressedEvent EventDetail = GCMouseButtonPressedEvent_Create(GCMouseButtonCode_Right);
			GCEvent Event = GCEvent_Create(GCEventType_MouseButtonPressed, &EventDetail);
			Window->Properties.EventCallback(Window, &Event);

			break;
		}
		case WM_MBUTTONDOWN:
		{
			const GCMouseButtonPressedEvent EventDetail = GCMouseButtonPressedEvent_Create(GCMouseButtonCode_Middle);
			GCEvent Event = GCEvent_Create(GCEventType_MouseButtonPressed, &EventDetail);
			Window->Properties.EventCallback(Window, &Event);

			break;
		}
		case WM_LBUTTONUP:
		{
			const GCMouseButtonReleasedEvent EventDetail = GCMouseButtonReleasedEvent_Create(GCMouseButtonCode_Left);
			GCEvent Event = GCEvent_Create(GCEventType_MouseButtonReleased, &EventDetail);
			Window->Properties.EventCallback(Window, &Event);

			break;
		}
		case WM_RBUTTONUP:
		{
			const GCMouseButtonReleasedEvent EventDetail = GCMouseButtonReleasedEvent_Create(GCMouseButtonCode_Right);
			GCEvent Event = GCEvent_Create(GCEventType_MouseButtonReleased, &EventDetail);
			Window->Properties.EventCallback(Window, &Event);

			break;
		}
		case WM_MBUTTONUP:
		{
			const GCMouseButtonReleasedEvent EventDetail = GCMouseButtonReleasedEvent_Create(GCMouseButtonCode_Middle);
			GCEvent Event = GCEvent_Create(GCEventType_MouseButtonReleased, &EventDetail);
			Window->Properties.EventCallback(Window, &Event);

			break;
		}

		case WM_MOUSEMOVE:
		{
			const POINTS Position = MAKEPOINTS(LParam);

			const GCMouseMovedEvent EventDetail = GCMouseMovedEvent_Create(Position.x, Position.y);
			GCEvent Event = GCEvent_Create(GCEventType_MouseMoved, &EventDetail);
			Window->Properties.EventCallback(Window, &Event);

			break;
		}
		case WM_MOUSEWHEEL:
		{
			const GCMouseScrolledEvent EventDetail = GCMouseScrolledEvent_Create(0.0f, (SHORT)HIWORD(WParam) / (float)WHEEL_DELTA);
			GCEvent Event = GCEvent_Create(GCEventType_MouseScrolled, &EventDetail);
			Window->Properties.EventCallback(Window, &Event);

			break;
		}

		case WM_SIZE:
		{
			const uint32_t Width = LOWORD(LParam);
			const uint32_t Height = HIWORD(LParam);

			const GCWindowResizedEvent EventDetail = GCWindowResizedEvent_Create(Width, Height);
			GCEvent Event = GCEvent_Create(GCEventType_WindowResized, &EventDetail);
			Window->Properties.EventCallback(Window, &Event);

			break;
		}
		case WM_CLOSE:
		{
			const GCWindowClosedEvent EventDetail = GCWindowClosedEvent_Create();
			GCEvent Event = GCEvent_Create(GCEventType_WindowClosed, &EventDetail);
			Window->Properties.EventCallback(Window, &Event);

			break;
		}
		case WM_DESTROY:
		{
			PostQuitMessage(0);

			break;
		}
	}

	return DefWindowProcW(WindowHandle, Message, WParam, LParam);
}

void GCWindowsWindow_ProcessEvents(GCWindowsWindow* const Window)
{
	(void)Window;

	MSG MessageHandle;

	while (PeekMessageW(&MessageHandle, NULL, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&MessageHandle);
		DispatchMessageW(&MessageHandle);
	}
}

void GCWindowsWindow_GetWindowSize(const GCWindowsWindow* const Window, uint32_t* const Width, uint32_t* const Height)
{
	RECT Coordinate = { 0 };
	GetClientRect(Window->WindowHandle, &Coordinate);

	if (Width)
	{
		*Width = Coordinate.right;
	}

	if (Height)
	{
		*Height = Coordinate.bottom;
	}
}

void GCWindowsWindow_Destroy(GCWindowsWindow* Window)
{
	DestroyWindow(Window->WindowHandle);
	UnregisterClassW(Window->ClassName, Window->InstanceHandle);

	GCMemory_Free(Window);
}

HWND GCWindowsWindow_GetWindowHandle(const GCWindowsWindow* const Window)
{
	return Window->WindowHandle;
}

HINSTANCE GCWindowsWindow_GetInstanceHandle(const GCWindowsWindow* const Window)
{
	return Window->InstanceHandle;
}