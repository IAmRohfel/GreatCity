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

#include "World/World.h"
#include "World/Camera/WorldCamera.h"
#include "World/Components.h"
#include "Core/Memory/Allocator.h"
#include "Renderer/Renderer.h"
#include "Renderer/RendererModel.h"

#include <flecs.h>

typedef struct GCWorld
{
	GCWorldCamera* WorldCamera;

	ecs_world_t* World;
} GCWorld;

ecs_world_t* GWorldECSWorld = NULL;

ECS_COMPONENT_DECLARE(GCTransformComponent);
ECS_COMPONENT_DECLARE(GCMeshComponent);

static GCEntity BasicTerrainEntity = 0, SmallOfficeEntity = 0;

GCWorld* GCWorld_Create(void)
{
	GCWorld* World = (GCWorld*)GCMemory_Allocate(sizeof(GCWorld));
	World->WorldCamera = GCWorldCamera_Create(30.0f, 1280.0f / 720.0f, 0.1f, 1000.0f);
	World->World = ecs_init();

	GWorldECSWorld = World->World;

	ECS_COMPONENT_DEFINE(World->World, GCTransformComponent);
	ECS_COMPONENT_DEFINE(World->World, GCMeshComponent);

	{
		BasicTerrainEntity = GCWorld_CreateEntity(World, "Basic Terrain");
		GCRendererModel* BasicTerrainModel = GCRendererModel_CreateFromFile("Assets/Models/Terrains/BasicTerrain.obj", "Assets/Models/Terrains");
		GCEntity_AddMeshComponent(BasicTerrainEntity, BasicTerrainModel);

		GCRendererModel_Destroy(BasicTerrainModel);
	}

	{
		SmallOfficeEntity = GCWorld_CreateEntity(World, "Small Office");
		GCRendererModel* SmallOfficeModel = GCRendererModel_CreateFromFile("Assets/Models/Buildings/Offices/SmallOffice.obj", "Assets/Models/Buildings/Offices");

		GCTransformComponent* TransformComponent = GCEntity_GetTransformComponent(SmallOfficeEntity);
		TransformComponent->Translation = GCVector3_Create(0.0f, 2.0f, 0.0f);
		TransformComponent->Scale = GCVector3_Create(2.0f, 2.0f, 2.0f);
		GCEntity_AddMeshComponent(SmallOfficeEntity, SmallOfficeModel);

		GCRendererModel_Destroy(SmallOfficeModel);
	}

	return World;
}

GCEntity GCWorld_CreateEntity(GCWorld* const World, const char* const Name)
{
	ecs_entity_t Entity = ecs_new_id(World->World);
	ecs_set_name(World->World, Entity, Name);

	GCEntity_AddTransformComponent((GCEntity)Entity);

	return (GCEntity)Entity;
}

void GCWorld_OnUpdate(GCWorld* const World)
{
	GCRenderer_BeginWorld(World->WorldCamera);
	{
		GCRenderer_RenderEntity(BasicTerrainEntity);
		GCRenderer_RenderEntity(SmallOfficeEntity);
	}
	GCRenderer_EndWorld();
}

void GCWorld_OnEvent(GCWorld* const World, GCEvent* const Event)
{
	(void)World;

	GCWorldCemera_OnEvent(World->WorldCamera, Event);
}

GCWorldCamera* GCWorld_GetCamera(const GCWorld* const World)
{
	return World->WorldCamera;
}

void GCWorld_Destroy(GCWorld* World)
{
	GCEntity_RemoveMeshComponent(SmallOfficeEntity);
	GCEntity_RemoveMeshComponent(BasicTerrainEntity);

	ecs_fini(World->World);

	GCMemory_Free(World->WorldCamera);
	GCMemory_Free(World);
}
