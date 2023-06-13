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
#include "Scene/Scene.h"
#include "Scene/Camera/WorldCamera.h"
#include "ImGui/ImGuiManager.h"
#include "Math/Matrix4x4.h"
#include "Math/Utilities.h"

#include <stdbool.h>

typedef struct GCApplication
{
	GCWindow* Window;
	GCScene* Scene;
	GCWorldCamera* WorldCamera;

	bool IsRunning;
	bool IsMinimized;
} GCApplication;

static GCApplication* Application = NULL;
static GCEntity BasicTerrainEntity = 0, SmallOfficeEntity = 0;

static void GCApplication_OnEvent(GCWindow* const Window, GCEvent* const Event);
static bool GCApplication_OnWindowResized(GCEvent* const Event, void* CustomData);
static bool GCApplication_OnWindowClosed(GCEvent* const Event, void* CustomData);

void GCApplication_Create(void)
{
	Application = (GCApplication*)GCMemory_Allocate(sizeof(GCApplication));
	Application->Window = NULL;
	Application->Scene = NULL;
	Application->WorldCamera = NULL;
	Application->IsRunning = true;
	Application->IsMinimized = false;

	GCWindowProperties WindowProperties;
	WindowProperties.Title = "Great City";
	WindowProperties.Width = 1280;
	WindowProperties.Height = 720;
	WindowProperties.EventCallback = GCApplication_OnEvent;

	Application->Window = GCWindow_Create(&WindowProperties);
	Application->Scene = GCScene_Create();
	Application->WorldCamera = GCWorldCamera_Create(30.0f, 1280.0f / 720.0f, 0.1f, 1000.0f);

	GCRenderer_Initialize(Application->WorldCamera);
	GCImGuiManager_Initialize();

	{
		BasicTerrainEntity = GCScene_CreateEntity(Application->Scene, "Basic Terrain");
		GCMeshComponent* MeshComponent = GCEntity_AddMeshComponent(BasicTerrainEntity);
		MeshComponent->Mesh = GCRendererMesh_Create(GCRendererModel_CreateFromFile("Assets/Models/Terrains/BasicTerrain.obj", "Assets/Models/Terrains"));
	}

	{
		SmallOfficeEntity = GCScene_CreateEntity(Application->Scene, "Small Office");
		GCTransformComponent* TransformComponent = GCEntity_GetTransformComponent(SmallOfficeEntity);
		TransformComponent->Position = GCVector3_Create(0.0f, 2.0f, 0.0f);
		TransformComponent->Scale = GCVector3_Create(2.0f, 2.0f, 2.0f);

		GCMeshComponent* MeshComponent = GCEntity_AddMeshComponent(SmallOfficeEntity);
		MeshComponent->Mesh = GCRendererMesh_Create(GCRendererModel_CreateFromFile("Assets/Models/Buildings/Offices/SmallOffice.obj", "Assets/Models/Buildings/Offices"));
	}
}

void GCApplication_Run(void)
{
	while (Application->IsRunning)
	{
		GCWorldCamera_Update(Application->WorldCamera);

		GCRenderer_BeginScene();
		GCImGuiManager_BeginFrame();
		{
			GCRenderer_RenderEntity(BasicTerrainEntity);
			GCRenderer_RenderEntity(SmallOfficeEntity);
		}
		GCImGuiManager_EndFrame();
		GCRenderer_EndScene();
		
		GCRenderer_BeginImGui();
		{
			GCImGuiManager_Render();
		}
		GCRenderer_EndImGui();
		GCRenderer_Present();

		GCImGuiManager_Update();

		GCWindow_ProcessEvents(Application->Window);
	}
}

GCWindow* const GCApplication_GetWindow(void)
{
	return Application->Window;
}

GCWorldCamera* const GCApplication_GetWorldCamera(void)
{
	return Application->WorldCamera;
}

void GCApplication_Destroy(void)
{
	GCEntity_RemoveMeshComponent(SmallOfficeEntity);
	GCEntity_RemoveMeshComponent(BasicTerrainEntity);
	GCScene_Destroy(Application->Scene);

	GCImGuiManager_Terminate();
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

	return true;
}

bool GCApplication_OnWindowClosed(GCEvent* const Event, void* CustomData)
{
	(void)Event;
	(void)CustomData;

	Application->IsRunning = false;

	return true;
}