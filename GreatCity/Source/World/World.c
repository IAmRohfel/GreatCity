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
#include "Core/Memory/Allocator.h"
#include "Renderer/Renderer.h"
#include "Renderer/RendererModel.h"
#include "World/Camera/WorldCamera.h"
#include "World/Components.h"

#include <stdbool.h>

#include <flecs.h>

typedef struct GCWorld
{
    GCWorldCamera* WorldCamera;
    GCEntity TerrainEntity;

    ecs_world_t* World;
} GCWorld;

ecs_world_t* GWorldECSWorld = NULL;

ECS_COMPONENT_DECLARE(GCTransformComponent);
ECS_COMPONENT_DECLARE(GCMeshComponent);

GCWorld* GCWorld_Create(void)
{
    GCWorld* World = (GCWorld*)GCMemory_Allocate(sizeof(GCWorld));
    World->WorldCamera = GCWorldCamera_Create(30.0f, 1280.0f / 720.0f, 0.1f, 1000.0f);
    World->World = ecs_init();

    GWorldECSWorld = World->World;

    ECS_COMPONENT_DEFINE(World->World, GCTransformComponent);
    ECS_COMPONENT_DEFINE(World->World, GCMeshComponent);

    {
        World->TerrainEntity = GCWorld_CreateEntity(World, "Basic Terrain");
        GCRendererModel* BasicTerrainModel =
            GCRendererModel_CreateFromFile("Assets/Models/Terrains/BasicTerrain.obj", "Assets/Models/Terrains");

        GCTransformComponent* const TransformComponent = GCEntity_GetTransformComponent(World->TerrainEntity);
        TransformComponent->Scale = GCVector3_Create(50.0f, 50.0f, 50.0f);

        GCEntity_AddMeshComponent(World->TerrainEntity, BasicTerrainModel);
        GCRendererModel_Destroy(BasicTerrainModel);
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

bool GCWorld_CheckCollision(const GCWorld* const World, const GCEntity Entity)
{
    const GCTransformComponent* const TransformComponent = GCEntity_GetTransformComponent(Entity);

    ecs_filter_desc_t FilterDescription = {0};
    FilterDescription.terms->id = ecs_id(GCTransformComponent);

    ecs_filter_t* Filter = ecs_filter_init(World->World, &FilterDescription);
    ecs_iter_t FilterIterator = ecs_filter_iter(World->World, Filter);

    bool IsColliding = false;

    while (ecs_filter_next(&FilterIterator))
    {
        for (int32_t Counter = 0; Counter < FilterIterator.count; Counter++)
        {
            if (FilterIterator.entities[Counter] != Entity)
            {
                const GCTransformComponent* const OtherTransformComponent =
                    GCEntity_GetTransformComponent(FilterIterator.entities[Counter]);

                if (TransformComponent->Translation.X < OtherTransformComponent->Translation.X + 1.0f &&
                    TransformComponent->Translation.X + 1.0f > OtherTransformComponent->Translation.X &&
                    TransformComponent->Translation.Y < OtherTransformComponent->Translation.Y + 1.0f &&
                    TransformComponent->Translation.Y + 1.0f > OtherTransformComponent->Translation.Y &&
                    TransformComponent->Translation.Z < OtherTransformComponent->Translation.Z + 1.0f &&
                    TransformComponent->Translation.Z + 1.0f > OtherTransformComponent->Translation.Z)
                {
                    IsColliding = true;
                }
            }
        }
    }

    ecs_iter_fini(&FilterIterator);
    ecs_filter_fini(Filter);

    return IsColliding;
}

void GCWorld_OnUpdate(GCWorld* const World)
{
    GCRenderer_BeginWorld(World->WorldCamera);
    {
        ecs_filter_desc_t FilterDescriptions[2] = {0};
        FilterDescriptions[0].terms->id = ecs_id(GCTransformComponent);
        FilterDescriptions[1].terms->id = ecs_id(GCMeshComponent);

        ecs_filter_t* Filter = ecs_filter_init(World->World, FilterDescriptions);
        ecs_iter_t FilterIterator = ecs_filter_iter(World->World, Filter);

        while (ecs_filter_next(&FilterIterator))
        {
            for (int32_t Counter = 0; Counter < FilterIterator.count; Counter++)
            {
                GCRenderer_RenderEntity(FilterIterator.entities[Counter]);
            }
        }

        ecs_iter_fini(&FilterIterator);
        ecs_filter_fini(Filter);
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

GCEntity GCWorld_GetTerrainEntity(const GCWorld* const World)
{
    return World->TerrainEntity;
}

void GCWorld_Destroy(GCWorld* World)
{
    ecs_filter_desc_t FilterDescription = {0};
    FilterDescription.terms->id = ecs_id(GCMeshComponent);

    ecs_filter_t* Filter = ecs_filter_init(World->World, &FilterDescription);
    ecs_iter_t FilterIterator = ecs_filter_iter(World->World, Filter);

    while (ecs_filter_next(&FilterIterator))
    {
        for (int32_t Counter = 0; Counter < FilterIterator.count; Counter++)
        {
            GCEntity_RemoveMeshComponent(FilterIterator.entities[Counter]);
        }
    }

    ecs_iter_fini(&FilterIterator);
    ecs_filter_fini(Filter);
    ecs_fini(World->World);

    GCMemory_Free(World->WorldCamera);
    GCMemory_Free(World);
}
