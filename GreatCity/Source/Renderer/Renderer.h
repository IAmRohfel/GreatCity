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

#ifndef GC_RENDERER_RENDERER_H
#define GC_RENDERER_RENDERER_H

#include "Scene/Entity.h"
#include "Math/Vector2.h"
#include "Math/Vector3.h"
#include "Math/Vector4.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

	typedef struct GCRendererVertex
	{
		GCVector3 Position;
		GCVector4 Color;
		GCVector2 TextureCoordinate;
		int32_t EntityID;

#ifdef __cplusplus
		bool operator==(const GCRendererVertex& OtherVertex) const noexcept
		{
			return GCVector3_IsEqual(Position, OtherVertex.Position) && GCVector4_IsEqual(Color, OtherVertex.Color) && GCVector2_IsEqual(TextureCoordinate, OtherVertex.TextureCoordinate);
		}
#endif
	} GCRendererVertex;

	typedef struct GCWorldCamera GCWorldCamera;
	typedef struct GCRendererModel GCRendererModel;
	typedef struct GCRendererDevice GCRendererDevice;
	typedef struct GCRendererSwapChain GCRendererSwapChain;
	typedef struct GCRendererCommandList GCRendererCommandList;
	typedef struct GCRendererGraphicsPipeline GCRendererGraphicsPipeline;
	typedef struct GCRendererFramebuffer GCRendererFramebuffer;

	void GCRenderer_Initialize(const GCWorldCamera* const WorldCamera);
	void GCRenderer_BeginScene(void);
	void GCRenderer_RenderEntity(const GCEntity Entity);
	void GCRenderer_EndScene(void);
	void GCRenderer_BeginImGui(void);
	void GCRenderer_EndImGui(void);
	void GCRenderer_Present(void);
	void GCRenderer_Resize(void);
	void GCRenderer_Terminate(void);

	GCRendererDevice* const GCRenderer_GetDevice(void);
	GCRendererSwapChain* const GCRenderer_GetSwapChain(void);
	GCRendererCommandList* const GCRenderer_GetCommandList(void);
	GCRendererGraphicsPipeline* const GCRenderer_GetGraphicsPipeline(void);
	GCRendererFramebuffer* const GCRenderer_GetFramebuffer(void);

#ifdef __cplusplus
}
#endif

#endif