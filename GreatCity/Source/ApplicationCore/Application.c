#include "ApplicationCore/Application.h"
#include "ApplicationCore/GenericPlatform/Window.h"
#include "ApplicationCore/Event/Event.h"
#include "ApplicationCore/Event/ApplicationEvent.h"
#include "Core/Memory/Allocator.h"
#include "Renderer/Renderer.h"
#include "Scene/Camera/WorldCamera.h"

#include <stdbool.h>

typedef struct GCApplication
{
	GCWindow* Window;
	GCWorldCamera* WorldCamera;

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
	Application->WorldCamera = NULL;
	Application->IsRunning = true;
	Application->IsMinimized = false;

	GCWindowProperties WindowProperties;
	WindowProperties.Title = "Great City";
	WindowProperties.Width = 1280;
	WindowProperties.Height = 720;
	WindowProperties.EventCallback = GCApplication_OnEvent;

	Application->Window = GCWindow_Create(&WindowProperties);

	Application->WorldCamera = GCWorldCamera_Create(30.0f, 1280.0f / 720.0f, 0.1f, 1000.0f);

	GCRenderer_Initialize(Application->WorldCamera);
}

void GCApplication_Run(void)
{
	while (Application->IsRunning)
	{
		GCWorldCamera_Update(Application->WorldCamera);

		GCRenderer_Begin();
		GCRenderer_Present();
		GCRenderer_End();

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

	GCMemory_Free(Application->WorldCamera);
	GCMemory_Free(Application);
}

void GCApplication_OnEvent(GCWindow* const Window, GCEvent* const Event)
{
	(void)Window;

	GCEvent_Dispatch(GCEventType_WindowResized, Event, GCApplication_OnWindowResized, NULL);
	GCEvent_Dispatch(GCEventType_WindowClosed, Event, GCApplication_OnWindowClosed, NULL);

	GCWorldCemera_OnEvent(Application->WorldCamera, Event);
}

bool GCApplication_OnWindowResized(GCEvent* const Event, void* CustomData)
{
	(void)CustomData;

	const GCWindowResizedEvent* const EventDetail = (const GCWindowResizedEvent* const)Event->EventDetail;

	if (!EventDetail->Width || !EventDetail->Height)
	{
		Application->IsMinimized = true;

		return true;
	}

	GCRenderer_Resize();

	if (Application->WorldCamera)
	{
		GCWorldCamera_SetSize(Application->WorldCamera, EventDetail->Width, EventDetail->Height);
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