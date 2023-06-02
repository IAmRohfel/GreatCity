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

#ifndef GC_RENDERER_VULKAN_VULKAN_UTILITIES_H
#define GC_RENDERER_VULKAN_VULKAN_UTILITIES_H

#include "Core/Log.h"
#include "Core/Assert.h"

#include <stdint.h>
#include <stdbool.h>

#include <vulkan/vulkan.h>

#ifdef __cplusplus
extern "C"
{
#endif

	#ifdef GC_ASSERT_ENABLED
		#ifndef GC_VULKAN_VALIDATE
			#define GC_VULKAN_VALIDATE(Function, ...) if((Function) != VK_SUCCESS) { GC_LOG_FATAL(__VA_ARGS__); GC_ASSERT(false); }
		#endif
	#else
		#ifndef GC_VULKAN_VALIDATE
			#define GC_VULKAN_VALIDATE(Function, ...) (Function)
		#endif
	#endif

	typedef struct GCRendererDevice GCRendererDevice;
	typedef struct GCRendererCommandList GCRendererCommandList;

	void GCVulkanUtilities_CreateBuffer(const GCRendererDevice* const Device, const size_t Size, const VkBufferUsageFlags Usage, const VkMemoryPropertyFlags MemoryProperty, VkBuffer* BufferHandle, VkDeviceMemory* BufferMemoryHandle);
	void GCVulkanUtilities_CopyBuffer(const GCRendererDevice* const Device, const GCRendererCommandList* const CommandList, const VkBuffer SourceBuffer, const VkBuffer DestinationBuffer, const VkDeviceSize Size);
	void GCVulkanUtilities_CreateImage(const GCRendererDevice* const Device, const uint32_t Width, const uint32_t Height, const VkFormat Format, const VkImageTiling Tiling, const VkImageUsageFlags Usage, const VkMemoryPropertyFlags MemoryProperty, VkImage* ImageHandle, VkDeviceMemory* ImageMemoryHandle);
	void GCVulkanUtilities_TransitionImageLayout(const GCRendererDevice* const Device, const GCRendererCommandList* const CommandList, const VkImage ImageHandle, const VkFormat Format, const VkImageLayout OldLayout, const VkImageLayout NewLayout);
	void GCVulkanUtilities_CopyBufferToImage(const GCRendererDevice* const Device, const GCRendererCommandList* const CommandList, const VkBuffer BufferHandle, const VkImage ImageHandle, const uint32_t Width, const uint32_t Height);
	void GCVulkanUtilities_CreateImageView(const GCRendererDevice* const Device, const VkImage ImageHandle, const VkFormat Format, const VkImageAspectFlags ImageAspect, VkImageView* ImageViewHandle);

	VkCommandBuffer GCVulkanUtilities_BeginSingleTimeCommands(const GCRendererDevice* const Device, const GCRendererCommandList* const CommandList);
	void GCVulkanUtilities_EndSingleTimeCommands(const GCRendererDevice* const Device, const GCRendererCommandList* const CommandList, const VkCommandBuffer CommandBufferHandle);

#ifdef __cplusplus
}
#endif

#endif