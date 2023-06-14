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
#include "ApplicationCore/GenericPlatform/Window.h"
#include "ApplicationCore/Event/Event.h"
#include "ApplicationCore/Event/ApplicationEvent.h"
#include "Core/Memory/Allocator.h"
#include "Renderer/Renderer.h"
#include "Renderer/RendererModel.h"
#include "Renderer/RendererMesh.h"
#include "World/World.h"
#include "UI/UI.h"
#include "Math/Matrix4x4.h"
#include "Math/Utilities.h"

#include <stdbool.h>

typedef struct GCApplication
{
	GCWindow* Window;
	GCWorld* World;

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
	Application->World = NULL;
	Application->IsRunning = true;
	Application->IsMinimized = false;

	GCWindowProperties WindowProperties;
	WindowProperties.Title = "Great City";
	WindowProperties.Width = 1280;
	WindowProperties.Height = 720;
	WindowProperties.EventCallback = GCApplication_OnEvent;

	Application->Window = GCWindow_Create(&WindowProperties);

	GCRenderer_PreInitialize();
	GCRenderer_Initialize();
	GCUI_Initialize();

	Application->World = GCWorld_Create();
}

void GCApplication_Run(void)
{
	while (Application->IsRunning)
	{
		GCWorld_OnUpdate(Application->World);

		GCUI_Render();
		GCRenderer_Present();
		GCUI_OnUpdate();

		GCWindow_ProcessEvents(Application->Window);
	}
}

GCWindow* const GCApplication_GetWindow(void)
{
	return Application->Window;
}

GCWorld* GCApplication_GetWorld(void)
{
	return Application->World;
}

void GCApplication_Destroy(void)
{
	GCWorld_Destroy(Application->World);

	GCUI_Terminate();
	GCRenderer_Terminate();
	GCWindow_Destroy(Application->Window);

	GCMemory_Free(Application);
}

void GCApplication_OnEvent(GCWindow* const Window, GCEvent* const Event)
{
	(void)Window;

	GCEvent_Dispatch(GCEventType_WindowResized, Event, GCApplication_OnWindowResized, NULL);
	GCEvent_Dispatch(GCEventType_WindowClosed, Event, GCApplication_OnWindowClosed, NULL);

	if (Application->World)
	{
		GCWorld_OnEvent(Application->World, Event);
	}

	GCUI_OnEvent(Event);
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

	return true;
}

bool GCApplication_OnWindowClosed(GCEvent* const Event, void* CustomData)
{
	(void)Event;
	(void)CustomData;

	Application->IsRunning = false;

	return true;
}