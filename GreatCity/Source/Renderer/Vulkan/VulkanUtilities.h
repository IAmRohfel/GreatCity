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

	typedef enum GCRendererAttachmentFormat GCRendererAttachmentFormat;
	typedef enum GCRendererAttachmentSampleCount GCRendererAttachmentSampleCount;

	void GCVulkanUtilities_CreateBuffer(const GCRendererDevice* const Device, const size_t Size, const VkBufferUsageFlags Usage, const VkMemoryPropertyFlags MemoryProperty, VkBuffer* BufferHandle, VkDeviceMemory* BufferMemoryHandle);
	void GCVulkanUtilities_CreateImage(const GCRendererDevice* const Device, const uint32_t Width, const uint32_t Height, const uint32_t MipLevels, const VkFormat Format, const VkImageTiling Tiling, const VkSampleCountFlagBits SampleCount, const VkImageUsageFlags Usage, const VkMemoryPropertyFlags MemoryProperty, VkImage* ImageHandle, VkDeviceMemory* ImageMemoryHandle);
	void GCVulkanUtilities_CreateImageView(const GCRendererDevice* const Device, const VkImage ImageHandle, const VkFormat Format, const VkImageAspectFlags ImageAspect, const uint32_t MipLevels, VkImageView* ImageViewHandle);
	void GCVulkanUtilities_CreateSampler(const GCRendererDevice* const Device, const VkFilter Filter, const VkSamplerAddressMode AddressMode, const uint32_t MipLevels, VkSampler* SamplerHandle);
	void GCVulkanUtilities_TransitionImageLayout(const VkCommandBuffer CommandBufferHandle, const VkImage ImageHandle, const uint32_t MipLevels, const VkImageLayout OldLayout, const VkImageLayout NewLayout);
	void GCVulkanUtilities_GenerateMipmap(const GCRendererDevice* const Device, const VkCommandBuffer CommandBufferHandle, const VkImage ImageHandle, const uint32_t Width, const uint32_t Height, const uint32_t MipLevels, const VkFormat Format);
	void GCVulkanUtilities_CopyImage(const VkCommandBuffer CommandBufferHandle, const VkImage SourceImageHandle, const VkImageLayout SourceImageLayout, const VkImage DestinationImageHandle, const VkImageLayout DestinationImageLayout, const uint32_t Width, const uint32_t Height);
	void GCVulkanUtilities_CopyImageToBuffer(const VkCommandBuffer CommandBufferHandle, const VkImage SourceImageHandle, const VkImageLayout SourceImageLayout, const VkBuffer DestinationBufferHandle, const uint32_t Width, const uint32_t Height, const int32_t X, const int32_t Y);
	void GCVulkanUtilities_CopyBuffer(const VkCommandBuffer CommandBufferHandle, const VkBuffer SourceBufferHandle, const VkBuffer DestinationBufferHandle, const VkDeviceSize Size);
	void GCVulkanUtilities_CopyBufferToImage(const VkCommandBuffer CommandBufferHandle, const VkBuffer SourceBufferHandle, const VkImage DestinationImageHandle, const uint32_t Width, const uint32_t Height);

	VkCommandBuffer GCVulkanUtilities_BeginSingleTimeCommands(const GCRendererDevice* const Device, const GCRendererCommandList* const CommandList);
	void GCVulkanUtilities_EndSingleTimeCommands(const GCRendererDevice* const Device, const GCRendererCommandList* const CommandList, const VkCommandBuffer CommandBufferHandle);

	VkSampleCountFlagBits GCVulkanUtilities_GetMaximumUsableSampleCount(const GCRendererDevice* const Device);

	VkFormat GCVulkanUtilities_ToVkFormat(const GCRendererDevice* const Device, const GCRendererAttachmentFormat Format);
	VkSampleCountFlagBits GCVulkanUtilities_ToVkSampleCountFlagBits(const GCRendererDevice* const Device, const GCRendererAttachmentSampleCount SampleCount);

#ifdef __cplusplus
}
#endif

#endif