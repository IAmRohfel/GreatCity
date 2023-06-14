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

#ifndef GC_RENDERER_VULKAN_VULKAN_RENDERER_COMMAND_LIST_H
#define GC_RENDERER_VULKAN_VULKAN_RENDERER_COMMAND_LIST_H

#include <stdint.h>

#include <vulkan/vulkan.h>

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct GCRendererCommandList GCRendererCommandList;

    uint32_t GCRendererCommandList_GetMaximumFramesInFlight(const GCRendererCommandList *const CommandList);
    VkCommandPool GCRendererCommandList_GetTransientCommandPoolHandle(const GCRendererCommandList *const CommandList);
    VkCommandBuffer GCRendererCommandList_GetCurrentFrameCommandBufferHandle(
        const GCRendererCommandList *const CommandList);

#ifdef __cplusplus
}
#endif

#endif