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
    typedef struct GCRendererCommandList GCRendererCommandList;
	typedef struct GCRendererGraphicsPipeline GCRendererGraphicsPipeline;

    typedef enum GCRendererFramebufferAttachmentType
    {
        GCRendererFramebufferAttachmentType_Color,
        GCRendererFramebufferAttachmentType_DepthStencil
    } GCRendererFramebufferAttachmentType;

    typedef enum GCRendererFramebufferAttachmentFlags
    {
        GCRendererFramebufferAttachmentFlags_None,
        GCRendererFramebufferAttachmentFlags_Sampled,
        GCRendererFramebufferAttachmentFlags_Mapped
    } GCRendererFramebufferAttachmentFlags;

    typedef enum GCRendererFramebufferAttachmentFormat
    {
        GCRendererFramebufferAttachmentFormat_SRGB,
        GCRendererFramebufferAttachmentFormat_Integer,
        GCRendererFramebufferAttachmentFormat_D32
    } GCRendererFramebufferAttachmentFormat;

    typedef enum GCRendererFramebufferAttachmentSampleCount
    {
        GCRendererFramebufferAttachmentSampleCount_1,
        GCRendererFramebufferAttachmentSampleCount_2,
        GCRendererFramebufferAttachmentSampleCount_4,
        GCRendererFramebufferAttachmentSampleCount_8,

        GCRendererFramebufferAttachmentSampleCount_MaximumUsable
    } GCRendererFramebufferAttachmentSampleCount;

    typedef struct GCRendererFramebufferAttachment
    {
        GCRendererFramebufferAttachmentType Type;
        GCRendererFramebufferAttachmentFlags Flags;
        GCRendererFramebufferAttachmentFormat Format;
        GCRendererFramebufferAttachmentSampleCount SampleCount;
    } GCRendererFramebufferAttachment;

    typedef struct GCRendererFramebufferDescription
    {
        const GCRendererDevice* Device;
        const GCRendererSwapChain* SwapChain;
        const GCRendererGraphicsPipeline* GraphicsPipeline;
        uint32_t Width, Height;
        const GCRendererFramebufferAttachment* Attachments;
        uint32_t AttachmentCount;
    } GCRendererFramebufferDescription;

	GCRendererFramebuffer* GCRendererFramebuffer_Create(const GCRendererFramebufferDescription* const Description);
    void GCRendererFramebuffer_RecreateSwapChainFramebuffer(GCRendererFramebuffer* const Framebuffer);
    void GCRendererFramebuffer_RecreateAttachmentFramebuffer(GCRendererFramebuffer* const Framebuffer, const uint32_t Width, const uint32_t Height);
    int32_t GCRendererFramebuffer_GetPixel(const GCRendererFramebuffer* const Framebuffer, const GCRendererCommandList* const CommandList, const uint32_t ColorAttachmentIndex, const uint32_t ColorAttachmentMappedIndex);
	void GCRendererFramebuffer_Destroy(GCRendererFramebuffer* Framebuffer);

#ifdef __cplusplus
}
#endif

#endif