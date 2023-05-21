#include "Renderer/Vulkan/VulkanUtilities.h"
#include "Core/Log.h"
#include "Core/Assert.h"

#include <stdint.h>
#include <stdbool.h>

#include <vulkan/vulkan.h>

extern VkDevice GCRendererDevice_GetDeviceHandle(const GCRendererDevice* const Device);
extern VkQueue GCRendererDevice_GetGraphicsQueueHandle(const GCRendererDevice* const Device);
extern uint32_t GCRendererDevice_GetMemoryTypeIndex(const GCRendererDevice* const Device, const uint32_t TypeFilter, const VkMemoryPropertyFlags PropertyFlags);
extern VkCommandPool GCRendererCommandList_GetTransientCommandPoolHandle(const GCRendererCommandList* const CommandList);

void GCVulkanUtilities_CreateBuffer(const GCRendererDevice* const Device, const size_t Size, const VkBufferUsageFlags Usage, const VkMemoryPropertyFlags MemoryProperty, VkBuffer* BufferHandle, VkDeviceMemory* DeviceMemoryHandle)
{
	const VkDevice DeviceHandle = GCRendererDevice_GetDeviceHandle(Device);

	VkBufferCreateInfo BufferInformation = { 0 };
	BufferInformation.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	BufferInformation.size = Size;
	BufferInformation.usage = Usage;
	BufferInformation.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	GC_VULKAN_VALIDATE(vkCreateBuffer(DeviceHandle, &BufferInformation, NULL, BufferHandle), "Failed to create a Vulkan buffer");

	VkMemoryRequirements MemoryRequirements = { 0 };
	vkGetBufferMemoryRequirements(DeviceHandle, *BufferHandle, &MemoryRequirements);

	VkMemoryAllocateInfo MemoryAllocateInformation = { 0 };
	MemoryAllocateInformation.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	MemoryAllocateInformation.allocationSize = MemoryRequirements.size;
	MemoryAllocateInformation.memoryTypeIndex = GCRendererDevice_GetMemoryTypeIndex(Device, MemoryRequirements.memoryTypeBits, MemoryProperty);

	GC_VULKAN_VALIDATE(vkAllocateMemory(DeviceHandle, &MemoryAllocateInformation, NULL, DeviceMemoryHandle), "Failed to allocate a Vulkan buffer memory");
	vkBindBufferMemory(DeviceHandle, *BufferHandle, *DeviceMemoryHandle, 0);
}

void GCVulkanUtilities_CopyBuffer(const GCRendererDevice* const Device, const GCRendererCommandList* const CommandList, const VkBuffer SourceBuffer, const VkBuffer DestinationBuffer, const VkDeviceSize Size)
{
	const VkDevice DeviceHandle = GCRendererDevice_GetDeviceHandle(Device);
	const VkCommandPool TransientCommandPoolHandle = GCRendererCommandList_GetTransientCommandPoolHandle(CommandList);
	const VkQueue GraphicsQueueHandle = GCRendererDevice_GetGraphicsQueueHandle(Device);

	VkCommandBufferAllocateInfo CommandBufferAllocateInformation = { 0 };
	CommandBufferAllocateInformation.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	CommandBufferAllocateInformation.commandPool = TransientCommandPoolHandle;
	CommandBufferAllocateInformation.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	CommandBufferAllocateInformation.commandBufferCount = 1;

	VkCommandBuffer CommandBufferHandle;
	GC_VULKAN_VALIDATE(vkAllocateCommandBuffers(DeviceHandle, &CommandBufferAllocateInformation, &CommandBufferHandle), "Failed to allocate a Vulkan command buffer");

	VkCommandBufferBeginInfo CommandBufferBeginInformation = { 0 };
	CommandBufferBeginInformation.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	CommandBufferBeginInformation.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	GC_VULKAN_VALIDATE(vkBeginCommandBuffer(CommandBufferHandle, &CommandBufferBeginInformation), "Failed to begin a Vulkan command buffer");

	VkBufferCopy BufferCopyRegion = { 0 };
	BufferCopyRegion.size = Size;

	vkCmdCopyBuffer(CommandBufferHandle, SourceBuffer, DestinationBuffer, 1, &BufferCopyRegion);

	GC_VULKAN_VALIDATE(vkEndCommandBuffer(CommandBufferHandle), "Failed to end a Vulkan command buffer");

	VkSubmitInfo SubmitInformation = { 0 };
	SubmitInformation.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	SubmitInformation.commandBufferCount = 1;
	SubmitInformation.pCommandBuffers = &CommandBufferHandle;

	vkQueueSubmit(GraphicsQueueHandle, 1, &SubmitInformation, VK_NULL_HANDLE);
	vkQueueWaitIdle(GraphicsQueueHandle);

	vkFreeCommandBuffers(DeviceHandle, TransientCommandPoolHandle, 1, &CommandBufferHandle);
}
