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

#ifndef GC_WORLD_COMPONENTS_H
#define GC_WORLD_COMPONENTS_H

#include "Math/Vector3.h"
#include "Math/Matrix4x4.h"

#ifdef __cplusplus
extern "C"
{
#endif

	typedef struct GCRendererMesh GCRendererMesh;

	typedef struct GCTransformComponent
	{
		GCVector3 Translation;
		GCVector3 Rotation;
		GCVector3 Scale;
	} GCTransformComponent;

	typedef struct GCMeshComponent
	{
		GCRendererMesh* Mesh;
	} GCMeshComponent;

	GCMatrix4x4 GCTransformComponent_GetTransform(const GCTransformComponent* const TransformComponent);

#ifdef __cplusplus
}
#endif

#endif