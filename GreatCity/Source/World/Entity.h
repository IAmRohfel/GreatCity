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

#ifndef GC_WORLD_ENTITY_H
#define GC_WORLD_ENTITY_H

#include "World/Components.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

    typedef uint64_t GCEntity;

    typedef struct GCRendererModel GCRendererModel;

    GCTransformComponent* GCEntity_AddTransformComponent(const GCEntity Entity);
    GCTransformComponent* GCEntity_GetTransformComponent(const GCEntity Entity);
    GCMeshComponent* GCEntity_AddMeshComponent(const GCEntity Entity, const GCRendererModel* const Model);
    GCMeshComponent* GCEntity_GetMeshComponent(const GCEntity Entity);
    void GCEntity_RemoveMeshComponent(const GCEntity Entity);

#ifdef __cplusplus
}
#endif

#endif