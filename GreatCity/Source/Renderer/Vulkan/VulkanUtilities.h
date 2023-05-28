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

	#ifndef GC_VULKAN_VALIDATE
		#define GC_VULKAN_VALIDATE(Function, ...) if((Function) != VK_SUCCESS) { GC_LOG_FATAL(__VA_ARGS__); GC_ASSERT(false); }
	#endif

	typedef struct GCRendererDevice GCRendererDevice;
	typedef struct GCRendererCommandList GCRendererCommandList;

	void GCVulkanUtilities_CreateBuffer(const GCRendererDevice* const Device, const size_t Size, const VkBufferUsageFlags Usage, const VkMemoryPropertyFlags MemoryProperty, VkBuffer* BufferHandle, VkDeviceMemory* BufferMemoryHandle);
	void GCVulkanUtilities_CopyBuffer(const GCRendererDevice* const Device, const GCRendererCommandList* const CommandList, const VkBuffer SourceBuffer, const VkBuffer DestinationBuffer, const VkDeviceSize Size);
	void GCVulkanUtilities_CreateImage(const GCRendererDevice* const Device, const uint32_t Width, const uint32_t Height, const VkFormat Format, const VkImageTiling Tiling, const VkImageUsageFlags Usage, const VkMemoryPropertyFlags MemoryProperty, VkImage* ImageHandle, VkDeviceMemory* ImageMemoryHandle);
	void GCVulkanUtilities_TransitionImageLayout(const GCRendererDevice* const Device, const GCRendererCommandList* const CommandList, const VkImage ImageHandle, const VkFormat Format, const VkImageLayout OldLayout, const VkImageLayout NewLayout);
	void GCVulkanUtilities_CopyBufferToImage(const GCRendererDevice* const Device, const GCRendererCommandList* const CommandList, const VkBuffer BufferHandle, const VkImage ImageHandle, const uint32_t Width, const uint32_t Height);
	void GCVulkanUtilities_CreateImageView(const GCRendererDevice* const Device, const VkImage ImageHandle, const VkFormat Format, VkImageView* ImageViewHandle);

	VkCommandBuffer GCVulkanUtilities_BeginSingleTimeCommands(const GCRendererDevice* const Device, const GCRendererCommandList* const CommandList);
	void GCVulkanUtilities_EndSingleTimeCommands(const GCRendererDevice* const Device, const GCRendererCommandList* const CommandList, const VkCommandBuffer CommandBufferHandle);

#ifdef __cplusplus
}
#endif

#endif