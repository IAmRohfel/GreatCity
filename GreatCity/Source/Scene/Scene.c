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

#include "Scene/Scene.h"
#include "Scene/Components/MeshComponent.h"
#include "Core/Memory/Allocator.h"

#include <flecs.h>

typedef struct GCScene
{
	ecs_world_t* World;
} GCScene;

ecs_world_t* GSceneECSWorld = NULL;

ECS_COMPONENT_DECLARE(GCMeshComponent);

GCScene* GCScene_Create(void)
{
	GCScene* Scene = (GCScene*)GCMemory_Allocate(sizeof(GCScene));
	Scene->World = ecs_init();

	GSceneECSWorld = Scene->World;

	ECS_COMPONENT_DEFINE(Scene->World, GCMeshComponent);

	return Scene;
}

GCEntity GCScene_CreateEntity(GCScene* const Scene, const char* const Name)
{
	ecs_entity_t Entity = ecs_new_id(Scene->World);
	ecs_set_name(Scene->World, Entity, Name);

	return (GCEntity)Entity;
}

void GCScene_Destroy(GCScene* Scene)
{
	ecs_fini(Scene->World);

	GCMemory_Free(Scene);
}
