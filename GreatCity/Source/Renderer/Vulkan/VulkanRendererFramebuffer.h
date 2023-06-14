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

#ifndef GC_RENDERER_VULKAN_VULKAN_RENDERER_FRAMEBUFFER_H
#define GC_RENDERER_VULKAN_VULKAN_RENDERER_FRAMEBUFFER_H

#include <vulkan/vulkan.h>

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct GCRendererFramebuffer GCRendererFramebuffer;

    VkImage GCRendererFramebuffer_GetColorAttachmentImageHandle(const GCRendererFramebuffer *const Framebuffer,
                                                                const uint32_t AttachmentIndex);
    VkImageView GCRendererFramebuffer_GetColorAttachmentImageViewHandle(const GCRendererFramebuffer *const Framebuffer,
                                                                        const uint32_t AttachmentIndex);
    VkSampler GCRendererFramebuffer_GetColorAttachmentSampledSamplerHandle(
        const GCRendererFramebuffer *const Framebuffer, const uint32_t AttachmentIndex);
    VkFramebuffer *GCRendererFramebuffer_GetSwapChainFramebufferHandles(const GCRendererFramebuffer *const Framebuffer);
    VkFramebuffer GCRendererFramebuffer_GetAttachmentFramebufferHandle(const GCRendererFramebuffer *const Framebuffer);
    VkExtent2D GCRendererFramebuffer_GetFramebufferSize(const GCRendererFramebuffer *const Framebuffer);

#ifdef __cplusplus
}
#endif

#endif