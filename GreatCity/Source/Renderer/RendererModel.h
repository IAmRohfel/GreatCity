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

#ifndef GC_RENDERER_RENDERER_MODEL_H
#define GC_RENDERER_RENDERER_MODEL_H

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct GCRendererVertexBuffer GCRendererVertexBuffer;
    typedef struct GCRendererIndexBuffer GCRendererIndexBuffer;

    typedef struct GCRendererVertex GCRendererVertex;

    typedef struct GCRendererModel
    {
        GCRendererVertex* Vertices;
        uint32_t VertexCount;

        uint32_t* Indices;
        uint32_t IndexCount;
    } GCRendererModel;

    GCRendererModel* GCRendererModel_CreateFromFile(const char* const ModelPath, const char* const MaterialPath);
    GCRendererModel* GCRendererModel_CreateFromFiles(
        const char* const* const ModelPaths, const char* const* const MaterialPaths, const uint32_t ModelCount
    );
    void GCRendererModel_Destroy(GCRendererModel* Model);

#ifdef __cplusplus
}
#endif

#endif