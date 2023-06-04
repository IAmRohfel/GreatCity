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

#include "Renderer/Vulkan/VulkanUtilities.h"
#include "Renderer/Vulkan/VulkanRendererDevice.h"
#include "Renderer/Vulkan/VulkanRendererCommandList.h"
#include "Renderer/RendererDevice.h"
#include "Core/Log.h"
#include "Core/Assert.h"

#include <stdint.h>
#include <stdbool.h>

#include <vulkan/vulkan.h>

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

void GCVulkanUtilities_CreateImage(const GCRendererDevice* const Device, const uint32_t Width, const uint32_t Height, const uint32_t MipLevels, const VkFormat Format, const VkImageTiling Tiling, const VkImageUsageFlags Usage, const VkMemoryPropertyFlags MemoryProperty, VkImage* ImageHandle, VkDeviceMemory* ImageMemoryHandle)
{
	const VkDevice DeviceHandle = GCRendererDevice_GetDeviceHandle(Device);

	VkImageCreateInfo ImageInformation = { 0 };
	ImageInformation.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	ImageInformation.imageType = VK_IMAGE_TYPE_2D;
	ImageInformation.format = Format;
	ImageInformation.extent.width = Width;
	ImageInformation.extent.height = Height;
	ImageInformation.extent.depth = 1;
	ImageInformation.mipLevels = MipLevels;
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

void GCVulkanUtilities_TransitionImageLayout(const GCRendererDevice* const Device, const GCRendererCommandList* const CommandList, const VkImage ImageHandle, const VkFormat Format, const uint32_t MipLevels, const VkImageLayout OldLayout, const VkImageLayout NewLayout)
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
	ImageMemoryBarrier.subresourceRange.levelCount = MipLevels;
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

void GCVulkanUtilities_CreateImageView(const GCRendererDevice* const Device, const VkImage ImageHandle, const VkFormat Format, const VkImageAspectFlags ImageAspect, const uint32_t MipLevels, VkImageView* ImageViewHandle)
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
	ImageViewInformation.subresourceRange.aspectMask = ImageAspect;
	ImageViewInformation.subresourceRange.baseMipLevel = 0;
	ImageViewInformation.subresourceRange.levelCount = MipLevels;
	ImageViewInformation.subresourceRange.baseArrayLayer = 0;
	ImageViewInformation.subresourceRange.layerCount = 1;

	GC_VULKAN_VALIDATE(vkCreateImageView(DeviceHandle, &ImageViewInformation, NULL, ImageViewHandle), "Failed to create a Vulkan image view");
}

void GCVulkanUtilities_CreateSampler(const GCRendererDevice* const Device, const VkFilter Filter, const VkSamplerAddressMode AddressMode, const uint32_t MipLevels, VkSampler* SamplerHandle)
{
	const VkDevice DeviceHandle = GCRendererDevice_GetDeviceHandle(Device);
	const GCRendererDeviceCapabilities DeviceCapabilities = GCRendererDevice_GetDeviceCapabilities(Device);

	VkSamplerCreateInfo SamplerInformation = { 0 };
	SamplerInformation.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	SamplerInformation.magFilter = Filter;
	SamplerInformation.minFilter = Filter;
	SamplerInformation.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	SamplerInformation.addressModeU = AddressMode;
	SamplerInformation.addressModeV = AddressMode;
	SamplerInformation.addressModeW = AddressMode;
	SamplerInformation.mipLodBias = 0.0f;

	if (DeviceCapabilities.IsAnisotropySupported)
	{
		SamplerInformation.anisotropyEnable = VK_TRUE;
		SamplerInformation.maxAnisotropy = DeviceCapabilities.MaximumAnisotropy;
	}
	else
	{
		SamplerInformation.anisotropyEnable = VK_FALSE;
		SamplerInformation.maxAnisotropy = 1.0f;
	}

	SamplerInformation.compareEnable = VK_FALSE;
	SamplerInformation.compareOp = VK_COMPARE_OP_ALWAYS;
	SamplerInformation.minLod = 0.0f;
	SamplerInformation.maxLod = (float)MipLevels;
	SamplerInformation.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	SamplerInformation.unnormalizedCoordinates = VK_FALSE;

	GC_VULKAN_VALIDATE(vkCreateSampler(DeviceHandle, &SamplerInformation, NULL, SamplerHandle), "Failed to create a Vulkan sampler");
}

void GCVulkanUtilities_GenerateMipmap(const GCRendererDevice* const Device, const GCRendererCommandList* const CommandList, const VkImage ImageHandle, const uint32_t Width, const uint32_t Height, const uint32_t MipLevels, const VkFormat Format)
{
	VkFormatProperties FormatProperties = { 0 };
	vkGetPhysicalDeviceFormatProperties(GCRendererDevice_GetPhysicalDeviceHandle(Device), Format, &FormatProperties);

	if (!(FormatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT))
	{
		GC_ASSERT_WITH_MESSAGE(false, "Image format does not support linear blitting");
	}

	const VkCommandBuffer CommandBufferHandle = GCVulkanUtilities_BeginSingleTimeCommands(Device, CommandList);

	VkImageMemoryBarrier ImageMemoryBarrier = { 0 };
	ImageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	ImageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	ImageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	ImageMemoryBarrier.image = ImageHandle;
	ImageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	ImageMemoryBarrier.subresourceRange.levelCount = 1;
	ImageMemoryBarrier.subresourceRange.baseArrayLayer = 0;
	ImageMemoryBarrier.subresourceRange.layerCount = 1;

	int32_t MipWidth = Width, MipHeight = Height;

	for (uint32_t Counter = 1; Counter < MipLevels; Counter++)
	{
		ImageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		ImageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		ImageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		ImageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		ImageMemoryBarrier.subresourceRange.baseMipLevel = Counter - 1;

		vkCmdPipelineBarrier(CommandBufferHandle, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, NULL, 0, NULL, 1, &ImageMemoryBarrier);

		VkImageBlit ImageBlit = { 0 };
		ImageBlit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		ImageBlit.srcSubresource.mipLevel = Counter - 1;
		ImageBlit.srcSubresource.baseArrayLayer = 0;
		ImageBlit.srcSubresource.layerCount = 1;
		ImageBlit.srcOffsets[0] = (VkOffset3D){ 0, 0, 0 };
		ImageBlit.srcOffsets[1] = (VkOffset3D){ MipWidth, MipHeight, 1 };
		ImageBlit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		ImageBlit.dstSubresource.mipLevel = Counter;
		ImageBlit.dstSubresource.baseArrayLayer = 0;
		ImageBlit.dstSubresource.layerCount = 1;
		ImageBlit.dstOffsets[0] = (VkOffset3D){ 0, 0, 0 };
		ImageBlit.dstOffsets[1] = (VkOffset3D){ MipWidth > 1 ? MipWidth / 2 : 1, MipHeight > 1 ? MipHeight / 2 : 1, 1 };

		vkCmdBlitImage(CommandBufferHandle, ImageHandle, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, ImageHandle, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &ImageBlit, VK_FILTER_LINEAR);

		ImageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		ImageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		ImageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		ImageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		vkCmdPipelineBarrier(CommandBufferHandle, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, NULL, 0, NULL, 1, &ImageMemoryBarrier);

		if (MipWidth > 1)
		{
			MipWidth /= 2;
		}

		if (MipHeight > 1)
		{
			MipHeight /= 2;
		}
	}

	ImageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	ImageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	ImageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	ImageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	ImageMemoryBarrier.subresourceRange.baseMipLevel = MipLevels - 1;

	vkCmdPipelineBarrier(CommandBufferHandle, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, NULL, 0, NULL, 1, &ImageMemoryBarrier);

	GCVulkanUtilities_EndSingleTimeCommands(Device, CommandList, CommandBufferHandle);
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