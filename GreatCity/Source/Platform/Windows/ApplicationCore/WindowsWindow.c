#include "ApplicationCore/GenericWindow.h"
#include "Core/Memory/Allocator.h"
#include "Core/Container/String.h"

#include <Windows.h>

typedef struct GCPlatformWindow
{
	GCGenericWindowProperties Properties;
	LPCWSTR ClassName;
	HINSTANCE InstanceHandle;
	HWND WindowHandle;
} GCGenericWindow, GCWindowsWindow;

static GCWindowsWindow* GCWindowsWindow_Create(const GCGenericWindowProperties* const Properties);
static LRESULT CALLBACK GCWindowsWindow_SetupMessageHandler(HWND WindowHandle, UINT Message, WPARAM WParam, LPARAM LParam);
static LRESULT CALLBACK GCWindowsWindow_MessageHandler(HWND WindowHandle, UINT Message, WPARAM WParam, LPARAM LParam);
static void GCWindowsWindow_ProcessEvents(GCWindowsWindow* const Window);
static void GCWindowsWindow_Destroy(GCWindowsWindow* Window);

GCGenericWindow* GCGenericWindow_Create(const GCGenericWindowProperties* const Properties)
{
	return GCWindowsWindow_Create(Properties);
}

void GCGenericWindow_ProcessEvents(GCGenericWindow* const Window)
{
	GCWindowsWindow_ProcessEvents(Window);
}

void GCGenericWindow_Destroy(GCGenericWindow* Window)
{
	GCWindowsWindow_Destroy(Window);
}

GCWindowsWindow* GCWindowsWindow_Create(const GCGenericWindowProperties* const Properties)
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
	switch (Message)
	{
		case WM_CLOSE:
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

void GCWindowsWindow_Destroy(GCWindowsWindow* Window)
{
	DestroyWindow(Window->WindowHandle);
	UnregisterClassW(Window->ClassName, Window->InstanceHandle);

	GCMemory_Free(Window);
}