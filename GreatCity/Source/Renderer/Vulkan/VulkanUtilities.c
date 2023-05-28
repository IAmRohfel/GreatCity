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

void GCVulkanUtilities_CreateBuffer(const GCRendererDevice* const Device, const size_t Size, const VkBufferUsageFlags Usage, const VkMemoryPropertyFlags MemoryProperty, VkBuffer* BufferHandle, VkDeviceMemory* BufferMemoryHandle)
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

	GC_VULKAN_VALIDATE(vkAllocateMemory(DeviceHandle, &MemoryAllocateInformation, NULL, BufferMemoryHandle), "Failed to allocate a Vulkan buffer memory");
	vkBindBufferMemory(DeviceHandle, *BufferHandle, *BufferMemoryHandle, 0);
}

void GCVulkanUtilities_CopyBuffer(const GCRendererDevice* const Device, const GCRendererCommandList* const CommandList, const VkBuffer SourceBuffer, const VkBuffer DestinationBuffer, const VkDeviceSize Size)
{
	const VkCommandBuffer CommandBufferHandle = GCVulkanUtilities_BeginSingleTimeCommands(Device, CommandList);

	VkBufferCopy BufferCopyRegion = { 0 };
	BufferCopyRegion.size = Size;

	vkCmdCopyBuffer(CommandBufferHandle, SourceBuffer, DestinationBuffer, 1, &BufferCopyRegion);

	GCVulkanUtilities_EndSingleTimeCommands(Device, CommandList, CommandBufferHandle);
}

void GCVulkanUtilities_CreateImage(const GCRendererDevice* const Device, const uint32_t Width, const uint32_t Height, const VkFormat Format, const VkImageTiling Tiling, const VkImageUsageFlags Usage, const VkMemoryPropertyFlags MemoryProperty, VkImage* ImageHandle, VkDeviceMemory* ImageMemoryHandle)
{
	const VkDevice DeviceHandle = GCRendererDevice_GetDeviceHandle(Device);

	VkImageCreateInfo ImageInformation = { 0 };
	ImageInformation.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	ImageInformation.imageType = VK_IMAGE_TYPE_2D;
	ImageInformation.format = Format;
	ImageInformation.extent.width = Width;
	ImageInformation.extent.height = Height;
	ImageInformation.extent.depth = 1;
	ImageInformation.mipLevels = 1;
	ImageInformation.arrayLayers = 1;
	ImageInformation.samples = VK_SAMPLE_COUNT_1_BIT;
	ImageInformation.tiling = Tiling;
	ImageInformation.usage = Usage;
	ImageInformation.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	ImageInformation.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

	GC_VULKAN_VALIDATE(vkCreateImage(DeviceHandle, &ImageInformation, NULL, ImageHandle), "Failed to create a Vulkan image");

	VkMemoryRequirements MemoryRequirements = { 0 };
	vkGetImageMemoryRequirements(DeviceHandle, *ImageHandle, &MemoryRequirements);

	VkMemoryAllocateInfo MemoryAllocateInformation = { 0 };
	MemoryAllocateInformation.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	MemoryAllocateInformation.allocationSize = MemoryRequirements.size;
	MemoryAllocateInformation.memoryTypeIndex = GCRendererDevice_GetMemoryTypeIndex(Device, MemoryRequirements.memoryTypeBits, MemoryProperty);

	GC_VULKAN_VALIDATE(vkAllocateMemory(DeviceHandle, &MemoryAllocateInformation, NULL, ImageMemoryHandle), "Failed to allocate a Vulkan image memory");
	vkBindImageMemory(DeviceHandle, *ImageHandle, *ImageMemoryHandle, 0);
}

void GCVulkanUtilities_TransitionImageLayout(const GCRendererDevice* const Device, const GCRendererCommandList* const CommandList, const VkImage ImageHandle, const VkFormat Format, const VkImageLayout OldLayout, const VkImageLayout NewLayout)
{
	(void)Format;

	const VkCommandBuffer CommandBufferHandle = GCVulkanUtilities_BeginSingleTimeCommands(Device, CommandList);

	VkImageMemoryBarrier ImageMemoryBarrier = { 0 };
	ImageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	ImageMemoryBarrier.oldLayout = OldLayout;
	ImageMemoryBarrier.newLayout = NewLayout;
	ImageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	ImageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	ImageMemoryBarrier.image = ImageHandle;
	ImageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	ImageMemoryBarrier.subresourceRange.baseMipLevel = 0;
	ImageMemoryBarrier.subresourceRange.levelCount = 1;
	ImageMemoryBarrier.subresourceRange.baseArrayLayer = 0;
	ImageMemoryBarrier.subresourceRange.layerCount = 1;

	VkPipelineStageFlags SourceStage = VK_PIPELINE_STAGE_NONE, DestinationStage = VK_PIPELINE_STAGE_NONE;

	if (OldLayout == VK_IMAGE_LAYOUT_UNDEFINED && NewLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
	{
		ImageMemoryBarrier.srcAccessMask = 0;
		ImageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		SourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		DestinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	else if (OldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && NewLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
	{
		ImageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		ImageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		SourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		DestinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}
	else
	{
		GC_ASSERT_WITH_MESSAGE(false, "Unsupported Vulkan image layout transition");
	}

	vkCmdPipelineBarrier(CommandBufferHandle, SourceStage, DestinationStage, 0, 0, NULL, 0, NULL, 1, &ImageMemoryBarrier);

	GCVulkanUtilities_EndSingleTimeCommands(Device, CommandList, CommandBufferHandle);
}

void GCVulkanUtilities_CopyBufferToImage(const GCRendererDevice* const Device, const GCRendererCommandList* const CommandList, const VkBuffer BufferHandle, const VkImage ImageHandle, const uint32_t Width, const uint32_t Height)
{
	const VkCommandBuffer CommandBufferHandle = GCVulkanUtilities_BeginSingleTimeCommands(Device, CommandList);

	VkBufferImageCopy BufferImageCopyRegion = { 0 };
	BufferImageCopyRegion.bufferOffset = 0;
	BufferImageCopyRegion.bufferRowLength = 0;
	BufferImageCopyRegion.bufferImageHeight = 0;
	BufferImageCopyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	BufferImageCopyRegion.imageSubresource.mipLevel = 0;
	BufferImageCopyRegion.imageSubresource.baseArrayLayer = 0;
	BufferImageCopyRegion.imageSubresource.layerCount = 1;
	BufferImageCopyRegion.imageOffset = (VkOffset3D){ 0 };
	BufferImageCopyRegion.imageExtent.width = Width;
	BufferImageCopyRegion.imageExtent.height = Height;
	BufferImageCopyRegion.imageExtent.depth = 1;

	vkCmdCopyBufferToImage(CommandBufferHandle, BufferHandle, ImageHandle, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &BufferImageCopyRegion);

	GCVulkanUtilities_EndSingleTimeCommands(Device, CommandList, CommandBufferHandle);
}

void GCVulkanUtilities_CreateImageView(const GCRendererDevice* const Device, const VkImage ImageHandle, const VkFormat Format, VkImageView* ImageViewHandle)
{
	const VkDevice DeviceHandle = GCRendererDevice_GetDeviceHandle(Device);

	VkImageViewCreateInfo ImageViewInformation = { 0 };
	ImageViewInformation.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	ImageViewInformation.image = ImageHandle;
	ImageViewInformation.viewType = VK_IMAGE_VIEW_TYPE_2D;
	ImageViewInformation.format = Format;
	ImageViewInformation.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
	ImageViewInformation.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	ImageViewInformation.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	ImageViewInformation.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
	ImageViewInformation.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	ImageViewInformation.subresourceRange.baseMipLevel = 0;
	ImageViewInformation.subresourceRange.levelCount = 1;
	ImageViewInformation.subresourceRange.baseArrayLayer = 0;
	ImageViewInformation.subresourceRange.layerCount = 1;

	GC_VULKAN_VALIDATE(vkCreateImageView(DeviceHandle, &ImageViewInformation, NULL, ImageViewHandle), "Failed to create a Vulkan image view");
}

VkCommandBuffer GCVulkanUtilities_BeginSingleTimeCommands(const GCRendererDevice* const Device, const GCRendererCommandList* const CommandList)
{
	const VkDevice DeviceHandle = GCRendererDevice_GetDeviceHandle(Device);
	const VkCommandPool TransientCommandPoolHandle = GCRendererCommandList_GetTransientCommandPoolHandle(CommandList);

	VkCommandBufferAllocateInfo CommandBufferAllocateInformation = { 0 };
	CommandBufferAllocateInformation.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	CommandBufferAllocateInformation.commandPool = TransientCommandPoolHandle;
	CommandBufferAllocateInformation.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	CommandBufferAllocateInformation.commandBufferCount = 1;

	VkCommandBuffer CommandBufferHandle = VK_NULL_HANDLE;
	GC_VULKAN_VALIDATE(vkAllocateCommandBuffers(DeviceHandle, &CommandBufferAllocateInformation, &CommandBufferHandle), "Failed to allocate a Vulkan command buffer");

	VkCommandBufferBeginInfo CommandBufferBeginInformation = { 0 };
	CommandBufferBeginInformation.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	CommandBufferBeginInformation.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	GC_VULKAN_VALIDATE(vkBeginCommandBuffer(CommandBufferHandle, &CommandBufferBeginInformation), "Failed to begin a Vulkan command buffer");

	return CommandBufferHandle;
}

void GCVulkanUtilities_EndSingleTimeCommands(const GCRendererDevice* const Device, const GCRendererCommandList* const CommandList, const VkCommandBuffer CommandBufferHandle)
{
	const VkDevice DeviceHandle = GCRendererDevice_GetDeviceHandle(Device);
	const VkCommandPool TransientCommandPoolHandle = GCRendererCommandList_GetTransientCommandPoolHandle(CommandList);
	const VkQueue GraphicsQueueHandle = GCRendererDevice_GetGraphicsQueueHandle(Device);

	GC_VULKAN_VALIDATE(vkEndCommandBuffer(CommandBufferHandle), "Failed to end a Vulkan command buffer");

	VkSubmitInfo SubmitInformation = { 0 };
	SubmitInformation.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	SubmitInformation.commandBufferCount = 1;
	SubmitInformation.pCommandBuffers = &CommandBufferHandle;

	vkQueueSubmit(GraphicsQueueHandle, 1, &SubmitInformation, VK_NULL_HANDLE);
	vkQueueWaitIdle(GraphicsQueueHandle);

	vkFreeCommandBuffers(DeviceHandle, TransientCommandPoolHandle, 1, &CommandBufferHandle);
}