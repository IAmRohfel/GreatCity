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

#ifndef GC_RENDERER_RENDERER_TEXTURE_2D_H
#define GC_RENDERER_RENDERER_TEXTURE_2D_H

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct GCRendererTexture2D GCRendererTexture2D;
    typedef struct GCRendererDevice GCRendererDevice;
    typedef struct GCRendererCommandList GCRendererCommandList;

    typedef struct GCRendererTexture2DDescription
    {
        const GCRendererDevice *Device;
        const GCRendererCommandList *CommandList;

        const char *TexturePath;
    } GCRendererTexture2DDescription;

    GCRendererTexture2D *GCRendererTexture2D_Create(const GCRendererTexture2DDescription *const Description);
    void GCRendererTexture2D_Destroy(GCRendererTexture2D *Texture2D);

#ifdef __cplusplus
}
#endif

#endif