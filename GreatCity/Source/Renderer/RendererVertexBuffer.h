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

#ifndef GC_RENDERER_RENDERER_VERTEX_BUFFER_H
#define GC_RENDERER_RENDERER_VERTEX_BUFFER_H

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct GCRendererVertexBuffer GCRendererVertexBuffer;
    typedef struct GCRendererDevice GCRendererDevice;
    typedef struct GCRendererCommandList GCRendererCommandList;

    typedef struct GCRendererVertexBufferDescription
    {
        const GCRendererDevice *Device;
        const GCRendererCommandList *CommandList;

        void *Vertices;
        uint32_t VertexCount;
        size_t VertexSize;
    } GCRendererVertexBufferDescription;

    GCRendererVertexBuffer *GCRendererVertexBuffer_Create(const GCRendererVertexBufferDescription *const Description);
    GCRendererVertexBuffer *GCRendererVertexBuffer_CreateDynamic(
        const GCRendererVertexBufferDescription *const Description);
    void GCRendererVertexBuffer_SetVertices(GCRendererVertexBuffer *const VertexBuffer, const void *const Vertices,
                                            const size_t VertexSize);
    void *GCRendererVertexBuffer_GetVertices(const GCRendererVertexBuffer *const VertexBuffer);
    uint32_t GCRendererVertexBuffer_GetVertexCount(const GCRendererVertexBuffer *const VertexBuffer);
    void GCRendererVertexBuffer_Destroy(GCRendererVertexBuffer *VertexBuffer);

#ifdef __cplusplus
}
#endif

#endif