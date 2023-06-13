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

#include "Renderer/RendererMesh.h"
#include "Renderer/Renderer.h"
#include "Renderer/RendererDevice.h"
#include "Renderer/RendererVertexBuffer.h"
#include "Renderer/RendererIndexBuffer.h"
#include "Renderer/RendererModel.h"
#include "Core/Memory/Allocator.h"
#include "World/Entity.h"

#include <string.h>
#include <stdint.h>

GCRendererMesh* GCRendererMesh_Create(const GCEntity Entity, const GCRendererModel* const Model)
{
	GCRendererMesh* Mesh = (GCRendererMesh*)GCMemory_Allocate(sizeof(GCRendererMesh));
	Mesh->VertexBuffer = NULL;
	Mesh->IndexBuffer = NULL;

	const GCRendererDevice* const Device = GCRenderer_GetDevice();
	const GCRendererCommandList* const CommandList = GCRenderer_GetCommandList();

	for (uint32_t Counter = 0; Counter < Model->VertexCount; Counter++)
	{
		Model->Vertices[Counter].EntityID = (uint64_t)Entity;
	}

	GCRendererVertexBufferDescription VertexBufferDescription = { 0 };
	VertexBufferDescription.Device = Device;
	VertexBufferDescription.CommandList = CommandList;
	VertexBufferDescription.Vertices = Model->Vertices;
	VertexBufferDescription.VertexCount = Model->VertexCount;
	VertexBufferDescription.VertexSize = Model->VertexCount * sizeof(GCRendererVertex);

	Mesh->VertexBuffer = GCRendererVertexBuffer_CreateDynamic(&VertexBufferDescription);

	GCRendererIndexBufferDescription IndexBufferDescription = { 0 };
	IndexBufferDescription.Device = Device;
	IndexBufferDescription.CommandList = CommandList;
	IndexBufferDescription.Indices = Model->Indices;
	IndexBufferDescription.IndexCount = Model->IndexCount;
	IndexBufferDescription.IndexSize = Model->IndexCount * sizeof(uint32_t);

	Mesh->IndexBuffer = GCRendererIndexBuffer_Create(&IndexBufferDescription);

	return Mesh;
}

void GCRendererMesh_ApplyTransform(GCRendererMesh* const Mesh, const GCMatrix4x4* const Transform)
{
	const GCRendererVertex* const OriginalVertices = (const GCRendererVertex* const)GCRendererVertexBuffer_GetVertices(Mesh->VertexBuffer);
	const uint32_t VertexCount = GCRendererVertexBuffer_GetVertexCount(Mesh->VertexBuffer);

	GCRendererVertex* Vertices = (GCRendererVertex*)GCMemory_Allocate(VertexCount * sizeof(GCRendererVertex));
	memcpy(Vertices, OriginalVertices, VertexCount * sizeof(GCRendererVertex));

	for (uint32_t Counter = 0; Counter < VertexCount; Counter++)
	{
		const GCVector3 Position = Vertices[Counter].Position;
		const GCVector4 TransformVector = GCMatrix4x4_MultiplyByVector(Transform, GCVector4_Create(Position.X, Position.Y, Position.Z, 1.0f));
	
		Vertices[Counter].Position = GCVector3_Create(TransformVector.X, TransformVector.Y, TransformVector.Z);
	}

	GCRendererVertexBuffer_SetVertices(Mesh->VertexBuffer, Vertices, VertexCount * sizeof(GCRendererVertex));

	GCMemory_Free(Vertices);
}

void GCRendererMesh_Destroy(GCRendererMesh* Mesh)
{
	GCRendererDevice_WaitIdle(GCRenderer_GetDevice());

	GCRendererIndexBuffer_Destroy(Mesh->IndexBuffer);
	GCRendererVertexBuffer_Destroy(Mesh->VertexBuffer);

	GCMemory_Free(Mesh);
}
