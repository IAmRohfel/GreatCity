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

#ifndef GC_RENDERER_RENDERER_FRAMEBUFFER_H
#define GC_RENDERER_RENDERER_FRAMEBUFFER_H

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

	typedef struct GCRendererFramebuffer GCRendererFramebuffer;
	typedef struct GCRendererDevice GCRendererDevice;
	typedef struct GCRendererSwapChain GCRendererSwapChain;
	typedef struct GCRendererGraphicsPipeline GCRendererGraphicsPipeline;

	GCRendererFramebuffer* GCRendererFramebuffer_Create(const GCRendererDevice* const Device, const GCRendererSwapChain* const SwapChain, const GCRendererGraphicsPipeline* const GraphicsPipeline);
    void GCRendererFramebuffer_RecreateTexture(GCRendererFramebuffer* const Framebuffer, const uint32_t Width, const uint32_t Height);
    void GCRendererFramebuffer_RecreateSwapChain(GCRendererFramebuffer* const Framebuffer);
	void GCRendererFramebuffer_Destroy(GCRendererFramebuffer* Framebuffer);

#ifdef __cplusplus
}
#endif

#endif