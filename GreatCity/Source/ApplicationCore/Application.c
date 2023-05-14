#include "ApplicationCore/Application.h"
#include "ApplicationCore/GenericPlatform/Window.h"
#include "ApplicationCore/Event/Event.h"
#include "ApplicationCore/Event/ApplicationEvent.h"
#include "Core/Memory/Allocator.h"
#include "Renderer/Renderer.h"

#include <stdbool.h>

typedef struct GCApplication
{
	GCWindow* Window;
	bool IsRunning;
	bool IsMinimized;
} GCApplication;

static GCApplication* Application = NULL;

static void GCApplication_OnEvent(GCWindow* const Window, GCEvent* const Event);
static bool GCApplication_OnWindowResized(GCEvent* const Event, void* CustomData);
static bool GCApplication_OnWindowClosed(GCEvent* const Event, void* CustomData);

void GCApplication_Create(void)
{
	Application = (GCApplication*)GCMemory_Allocate(sizeof(GCApplication));
	Application->Window = NULL;
	Application->IsRunning = true;
	Application->IsMinimized = false;

	GCWindowProperties WindowProperties;
	WindowProperties.Title = "Great City";
	WindowProperties.Width = 1280;
	WindowProperties.Height = 720;
	WindowProperties.EventCallback = GCApplication_OnEvent;

	Application->Window = GCWindow_Create(&WindowProperties);

	GCRenderer_Initialize();
}

void GCApplication_Run(void)
{
	while (Application->IsRunning)
	{
		GCRenderer_Present();

		GCWindow_ProcessEvents(Application->Window);
	}
}

const GCWindow* const GCApplication_GetWindow(void)
{
	return Application->Window;
}

void GCApplication_Destroy(void)
{
	GCRenderer_Terminate();
	GCWindow_Destroy(Application->Window);

	GCMemory_Free(Application);
}

void GCApplication_OnEvent(GCWindow* const Window, GCEvent* const Event)
{
	(void)Window;

	GCEvent_Dispatch(GCEventType_WindowResized, Event, GCApplication_OnWindowResized, NULL);
	GCEvent_Dispatch(GCEventType_WindowClosed, Event, GCApplication_OnWindowClosed, NULL);
}

bool GCApplication_OnWindowResized(GCEvent* const Event, void* CustomData)
{
	(void)CustomData;

	const GCWindowResizedEvent* const EventDetail = (const GCWindowResizedEvent* const)Event->EventDetail;

	if (!EventDetail->Width || !EventDetail->Height)
	{
		Application->IsMinimized = true;
	}

	return true;
}

bool GCApplication_OnWindowClosed(GCEvent* const Event, void* CustomData)
{
	(void)Event;
	(void)CustomData;

	Application->IsRunning = false;

	return true;
}