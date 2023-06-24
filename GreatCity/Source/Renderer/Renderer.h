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

#include "Math/Vector2.h"
#include "Math/Vector3.h"
#include "Math/Vector4.h"
#include "World/Entity.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct GCRendererVertex
    {
        GCVector3 Position;
        GCVector3 Normal;
        GCVector4 Color;
        GCVector2 TextureCoordinate;
        uint32_t TextureIndex;
        uint32_t EntityID;

#ifdef __cplusplus
        bool operator==(const GCRendererVertex& OtherVertex) const noexcept
        {
            return GCVector3_IsEqual(Position, OtherVertex.Position) && GCVector3_IsEqual(Normal, OtherVertex.Normal) &&
                   GCVector4_IsEqual(Color, OtherVertex.Color) &&
                   GCVector2_IsEqual(TextureCoordinate, OtherVertex.TextureCoordinate);
        }
#endif
    } GCRendererVertex;

    typedef struct GCWorldCamera GCWorldCamera;
    typedef struct GCRendererModel GCRendererModel;
    typedef struct GCRendererDevice GCRendererDevice;
    typedef struct GCRendererSwapChain GCRendererSwapChain;
    typedef struct GCRendererCommandList GCRendererCommandList;
    typedef struct GCRendererTexture2D GCRendererTexture2D;
    typedef struct GCRendererGraphicsPipeline GCRendererGraphicsPipeline;
    typedef struct GCRendererFramebuffer GCRendererFramebuffer;

    void GCRenderer_Initialize(void);

    void GCRenderer_BeginWorld(const GCWorldCamera* const WorldCamera);
    void GCRenderer_DrawQuad(const GCVector3 Position, const GCVector2 Size, const GCVector4 Color);
    void GCRenderer_DrawQuadTexture2D(
        const GCVector3 Position, const GCVector2 Size, const GCRendererTexture2D* const Texture2D,
        const GCVector4 TintColor
    );
    void GCRenderer_DrawQuadTransform(const GCMatrix4x4* const Transform, const GCVector4 Color);
    void GCRenderer_DrawQuadTexture2DTransform(
        const GCMatrix4x4* const Transform, const GCRendererTexture2D* const Texture2D, const GCVector4 TintColor
    );
    void GCRenderer_DrawEntity(const GCEntity Entity);
    void GCRenderer_EndWorld(void);

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