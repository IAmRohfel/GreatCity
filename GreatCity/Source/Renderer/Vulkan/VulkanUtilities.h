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

	void GCVulkanUtilities_CreateBuffer(const GCRendererDevice* const Device, const size_t Size, const VkBufferUsageFlags Usage, const VkMemoryPropertyFlags MemoryProperty, VkBuffer* BufferHandle, VkDeviceMemory* DeviceMemoryHandle);
	void GCVulkanUtilities_CopyBuffer(const GCRendererDevice* const Device, const GCRendererCommandList* const CommandList, const VkBuffer SourceBuffer, const VkBuffer DestinationBuffer, const VkDeviceSize Size);

#ifdef __cplusplus
}
#endif

#endif