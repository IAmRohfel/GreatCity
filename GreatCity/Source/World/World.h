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

#ifndef GC_WORLD_WORLD_H
#define GC_WORLD_WORLD_H

#include "World/Entity.h"

#include <stdbool.h>

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct GCWorld GCWorld;
    typedef struct GCWorldCamera GCWorldCamera;
    typedef struct GCEvent GCEvent;

    GCWorld *GCWorld_Create(void);
    GCEntity GCWorld_CreateEntity(GCWorld *const World, const char *const Name);
    bool GCWorld_CheckCollision(const GCWorld *const World, const GCEntity Entity);
    void GCWorld_OnUpdate(GCWorld *const World);
    void GCWorld_OnEvent(GCWorld *const World, GCEvent *const Event);
    GCWorldCamera *GCWorld_GetCamera(const GCWorld *const World);

    GCEntity GCWorld_GetTerrainEntity(const GCWorld *const World);
    void GCWorld_Destroy(GCWorld *World);

#ifdef __cplusplus
}
#endif

#endif