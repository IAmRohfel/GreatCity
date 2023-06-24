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

#ifndef GC_RENDERER_VULKAN_VULKAN_RENDERER_DEVICE_H
#define GC_RENDERER_VULKAN_VULKAN_RENDERER_DEVICE_H

#include <stdint.h>

#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct GCRendererDevice GCRendererDevice;

    VkInstance GCRendererDevice_GetInstanceHandle(const GCRendererDevice* const Device);
    VkSurfaceKHR GCRendererDevice_GetSurfaceHandle(const GCRendererDevice* const Device);
    VkPhysicalDevice GCRendererDevice_GetPhysicalDeviceHandle(const GCRendererDevice* const Device);
    VkDevice GCRendererDevice_GetDeviceHandle(const GCRendererDevice* const Device);
    uint32_t GCRendererDevice_GetGraphicsFamilyQueueIndex(const GCRendererDevice* const Device);
    uint32_t GCRendererDevice_GetPresentFamilyQueueIndex(const GCRendererDevice* const Device);
    VkQueue GCRendererDevice_GetGraphicsQueueHandle(const GCRendererDevice* const Device);
    VkQueue GCRendererDevice_GetPresentQueueHandle(const GCRendererDevice* const Device);
    VmaAllocator GCRendererDevice_GetAllocatorHandle(const GCRendererDevice* const Device);
    uint32_t GCRendererDevice_GetMemoryTypeIndex(
        const GCRendererDevice* const Device, const uint32_t TypeFilter, const VkMemoryPropertyFlags PropertyFlags
    );

#ifdef __cplusplus
}
#endif

#endif