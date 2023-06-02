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

#ifndef GC_SCENE_CAMERA_WORLD_CAMERA_H
#define GC_SCENE_CAMERA_WORLD_CAMERA_H

#include "Math/Matrix4x4.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

	typedef struct GCWorldCamera GCWorldCamera;
	typedef struct GCEvent GCEvent;

	GCWorldCamera* GCWorldCamera_Create(const float FoV, const float AspectRatio, const float Near, const float Far);
	void GCWorldCamera_Update(GCWorldCamera* const WorldCamera);
	void GCWorldCemera_OnEvent(GCWorldCamera* const WorldCamera, GCEvent* const Event);
	void GCWorldCamera_SetSize(GCWorldCamera* const WorldCamera, const uint32_t Width, const uint32_t Height);
	GCMatrix4x4 GCWorldCamera_GetViewProjectionMatrix(const GCWorldCamera* const WorldCamera);

#ifdef __cplusplus
}
#endif

#endif