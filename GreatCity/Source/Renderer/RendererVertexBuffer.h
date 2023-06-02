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

	GCRendererVertexBuffer* GCRendererVertexBuffer_Create(const GCRendererDevice* const Device, const GCRendererCommandList* const CommandList, void* const Vertices, const size_t VertexSize);
	GCRendererVertexBuffer* GCRendererVertexBuffer_CreateDynamic(const GCRendererDevice* const Device, const GCRendererCommandList* const CommandList, const size_t VertexSize);
	void GCRendererVertexBuffer_SetVertices(GCRendererVertexBuffer* const VertexBuffer, const void* const Vertices, const size_t VertexSize);
	void GCRendererVertexBuffer_Destroy(GCRendererVertexBuffer* VertexBuffer);

#ifdef __cplusplus
}
#endif

#endif