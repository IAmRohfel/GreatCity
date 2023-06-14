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

#ifndef GC_RENDERER_RENDERER_MESH_H
#define GC_RENDERER_RENDERER_MESH_H

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

	typedef struct GCRendererVertexBuffer GCRendererVertexBuffer;
	typedef struct GCRendererIndexBuffer GCRendererIndexBuffer;
	typedef struct GCRendererModel GCRendererModel;

	typedef struct GCMatrix4x4 GCMatrix4x4;

	typedef struct GCRendererMesh
	{
		GCRendererVertexBuffer* VertexBuffer;
		GCRendererIndexBuffer* IndexBuffer;
	} GCRendererMesh;

	GCRendererMesh* GCRendererMesh_Create(const uint64_t EntityID, const GCRendererModel* const Model);
	void GCRendererMesh_Destroy(GCRendererMesh* Mesh);

#ifdef __cplusplus
}
#endif

#endif