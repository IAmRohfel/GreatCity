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

#include "Scene/Entity.h"
#include "Scene/Components/MeshComponent.h"
#include "Renderer/RendererModel.h"

#include <flecs.h>

extern ecs_world_t* GSceneECSWorld;
extern ECS_COMPONENT_DECLARE(GCMeshComponent);

GCMeshComponent* GCEntity_AddMeshComponent(const GCEntity Entity)
{
	ecs_add(GSceneECSWorld, (ecs_entity_t)Entity, GCMeshComponent);

	return ecs_get_mut(GSceneECSWorld, (ecs_entity_t)Entity, GCMeshComponent);
}

GCMeshComponent* GCEntity_GetMeshComponent(const GCEntity Entity)
{
	return ecs_get_mut(GSceneECSWorld, (ecs_entity_t)Entity, GCMeshComponent);
}

void GCEntity_RemoveMeshComponent(const GCEntity Entity)
{
	GCMeshComponent* MeshComponent = ecs_get_mut(GSceneECSWorld, (ecs_entity_t)Entity, GCMeshComponent);

	GCRendererModel_Destroy(MeshComponent->Model);
	ecs_remove(GSceneECSWorld, (ecs_entity_t)Entity, GCMeshComponent);
}