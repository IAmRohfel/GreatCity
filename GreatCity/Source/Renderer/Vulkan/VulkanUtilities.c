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
#include "Core/Assert.h"
#include "Core/Log.h"
#include "Renderer/RendererDevice.h"
#include "Renderer/RendererEnums.h"
#include "Renderer/Vulkan/VulkanRendererCommandList.h"
#include "Renderer/Vulkan/VulkanRendererDevice.h"

#include <stdbool.h>
#include <stdint.h>

#include <vulkan/vulkan.h>
#ifndef VMA_VULKAN_VERSION
#define VMA_VULKAN_VERSION 1001000
#endif
#include <vk_mem_alloc.h>

void GCVulkanUtilities_CreateBuffer(
    const GCRendererDevice* const Device, const size_t Size, const VkBufferUsageFlags Usage,
    const VmaAllocationCreateFlags AllocationFlags, const VmaMemoryUsage MemoryUsage, VkBuffer* BufferHandle,
    VmaAllocation* BufferAllocationHandle, VmaAllocationInfo* AllocationInformation
)
{
    const VmaAllocator AllocatorHandle = GCRendererDevice_GetAllocatorHandle(Device);

    VkBufferCreateInfo BufferInformation = {0};
    BufferInformation.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    BufferInformation.size = Size;
    BufferInformation.usage = Usage;
    BufferInformation.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VmaAllocationCreateInfo BufferAllocationInformation = {0};
    BufferAllocationInformation.flags = AllocationFlags;
    BufferAllocationInformation.usage = MemoryUsage;

    GC_VULKAN_VALIDATE(
        vmaCreateBuffer(
            AllocatorHandle, &BufferInformation, &BufferAllocationInformation, BufferHandle, BufferAllocationHandle,
            AllocationInformation
        ),
        "Failed to create and allocate a buffer."
    );
}

void GCVulkanUtilities_CreateImage(
    const GCRendererDevice* const Device, const uint32_t Width, const uint32_t Height, const uint32_t MipLevels,
    const VkFormat Format, const VkImageTiling Tiling, const VkSampleCountFlagBits SampleCount,
    const VkImageUsageFlags Usage, const VmaAllocationCreateFlags AllocationFlags, const VmaMemoryUsage MemoryUsage,
    VkImage* ImageHandle, VmaAllocation* ImageAllocationHandle, VmaAllocationInfo* AllocationInformation
)
{
    const VmaAllocator AllocatorHandle = GCRendererDevice_GetAllocatorHandle(Device);

    VkImageCreateInfo ImageInformation = {0};
    ImageInformation.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    ImageInformation.imageType = VK_IMAGE_TYPE_2D;
    ImageInformation.format = Format;
    ImageInformation.extent.width = Width;
    ImageInformation.extent.height = Height;
    ImageInformation.extent.depth = 1;
    ImageInformation.mipLevels = MipLevels;
    ImageInformation.arrayLayers = 1;
    ImageInformation.samples = SampleCount;
    ImageInformation.tiling = Tiling;
    ImageInformation.usage = Usage;
    ImageInformation.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    ImageInformation.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    VmaAllocationCreateInfo ImageAllocationInformation = {0};
    ImageAllocationInformation.flags = AllocationFlags;
    ImageAllocationInformation.usage = MemoryUsage;

    GC_VULKAN_VALIDATE(
        vmaCreateImage(
            AllocatorHandle, &ImageInformation, &ImageAllocationInformation, ImageHandle, ImageAllocationHandle,
            AllocationInformation
        ),
        "Failed to create and allocate an image."
    );
}

void GCVulkanUtilities_CreateImageView(
    const GCRendererDevice* const Device, const VkImage ImageHandle, const VkFormat Format,
    const VkImageAspectFlags ImageAspect, const uint32_t MipLevels, VkImageView* ImageViewHandle
)
{
    const VkDevice DeviceHandle = GCRendererDevice_GetDeviceHandle(Device);

    VkImageViewCreateInfo ImageViewInformation = {0};
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

    GC_VULKAN_VALIDATE(
        vkCreateImageView(DeviceHandle, &ImageViewInformation, NULL, ImageViewHandle), "Failed to create an image view."
    );
}

void GCVulkanUtilities_CreateSampler(
    const GCRendererDevice* const Device, const VkFilter Filter, const VkSamplerAddressMode AddressMode,
    const uint32_t MipLevels, VkSampler* SamplerHandle
)
{
    const VkDevice DeviceHandle = GCRendererDevice_GetDeviceHandle(Device);
    const GCRendererDeviceCapabilities DeviceCapabilities = GCRendererDevice_GetDeviceCapabilities(Device);

    VkSamplerCreateInfo SamplerInformation = {0};
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

    GC_VULKAN_VALIDATE(
        vkCreateSampler(DeviceHandle, &SamplerInformation, NULL, SamplerHandle), "Failed to create a sampler."
    );
}

void GCVulkanUtilities_TransitionImageLayout(
    const VkCommandBuffer CommandBufferHandle, const VkImage ImageHandle, const uint32_t MipLevels,
    const VkImageLayout OldLayout, const VkImageLayout NewLayout
)
{
    VkImageMemoryBarrier ImageMemoryBarrier = {0};
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

    switch (OldLayout)
    {
    case VK_IMAGE_LAYOUT_UNDEFINED: {
        ImageMemoryBarrier.srcAccessMask = 0;
        SourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;

        break;
    }
    case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL: {
        ImageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        SourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;

        break;
    }
    case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL: {
        ImageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        SourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;

        break;
    }
    case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL: {
        ImageMemoryBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        SourceStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

        break;
    }
    default: {
        GC_ASSERT_WITH_MESSAGE(false, "Unsupported image layout transition.");

        break;
    }
    }

    switch (NewLayout)
    {
    case VK_IMAGE_LAYOUT_UNDEFINED: {
        ImageMemoryBarrier.dstAccessMask = 0;
        DestinationStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;

        break;
    }
    case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL: {
        ImageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        DestinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;

        break;
    }
    case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL: {
        ImageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        DestinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;

        break;
    }
    case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL: {
        ImageMemoryBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        DestinationStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

        break;
    }
    case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL: {
        ImageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        DestinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;

        break;
    }
    default: {
        GC_ASSERT_WITH_MESSAGE(false, "Unsupported image layout transition.");

        break;
    }
    }

    vkCmdPipelineBarrier(
        CommandBufferHandle, SourceStage, DestinationStage, 0, 0, NULL, 0, NULL, 1, &ImageMemoryBarrier
    );
}

void GCVulkanUtilities_GenerateMipmap(
    const GCRendererDevice* const Device, const VkCommandBuffer CommandBufferHandle, const VkImage ImageHandle,
    const uint32_t Width, const uint32_t Height, const uint32_t MipLevels, const VkFormat Format
)
{
    VkFormatProperties FormatProperties = {0};
    vkGetPhysicalDeviceFormatProperties(GCRendererDevice_GetPhysicalDeviceHandle(Device), Format, &FormatProperties);

    if (!(FormatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT))
    {
        GC_ASSERT_WITH_MESSAGE(false, "Image format does not support linear blitting.");
    }

    VkImageMemoryBarrier ImageMemoryBarrier = {0};
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

        vkCmdPipelineBarrier(
            CommandBufferHandle, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, NULL, 0, NULL, 1,
            &ImageMemoryBarrier
        );

        VkImageBlit ImageBlit = {0};
        ImageBlit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        ImageBlit.srcSubresource.mipLevel = Counter - 1;
        ImageBlit.srcSubresource.baseArrayLayer = 0;
        ImageBlit.srcSubresource.layerCount = 1;
        ImageBlit.srcOffsets[0] = (VkOffset3D){0, 0, 0};
        ImageBlit.srcOffsets[1] = (VkOffset3D){MipWidth, MipHeight, 1};
        ImageBlit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        ImageBlit.dstSubresource.mipLevel = Counter;
        ImageBlit.dstSubresource.baseArrayLayer = 0;
        ImageBlit.dstSubresource.layerCount = 1;
        ImageBlit.dstOffsets[0] = (VkOffset3D){0, 0, 0};
        ImageBlit.dstOffsets[1] = (VkOffset3D){MipWidth > 1 ? MipWidth / 2 : 1, MipHeight > 1 ? MipHeight / 2 : 1, 1};

        vkCmdBlitImage(
            CommandBufferHandle, ImageHandle, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, ImageHandle,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &ImageBlit, VK_FILTER_LINEAR
        );

        ImageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        ImageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        ImageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        ImageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        vkCmdPipelineBarrier(
            CommandBufferHandle, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, NULL, 0,
            NULL, 1, &ImageMemoryBarrier
        );

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

    vkCmdPipelineBarrier(
        CommandBufferHandle, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, NULL, 0, NULL,
        1, &ImageMemoryBarrier
    );
}

void GCVulkanUtilities_CopyImage(
    const VkCommandBuffer CommandBufferHandle, const VkImage SourceImageHandle, const VkImageLayout SourceImageLayout,
    const VkImage DestinationImageHandle, const VkImageLayout DestinationImageLayout, const uint32_t Width,
    const uint32_t Height
)
{
    VkImageCopy ImageCopyRegion = {0};
    ImageCopyRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    ImageCopyRegion.srcSubresource.mipLevel = 0;
    ImageCopyRegion.srcSubresource.baseArrayLayer = 0;
    ImageCopyRegion.srcSubresource.layerCount = 1;
    ImageCopyRegion.srcOffset = (VkOffset3D){0};
    ImageCopyRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    ImageCopyRegion.dstSubresource.mipLevel = 0;
    ImageCopyRegion.dstSubresource.baseArrayLayer = 0;
    ImageCopyRegion.dstSubresource.layerCount = 1;
    ImageCopyRegion.dstOffset = (VkOffset3D){0};
    ImageCopyRegion.extent.width = Width;
    ImageCopyRegion.extent.height = Height;
    ImageCopyRegion.extent.depth = 1;

    vkCmdCopyImage(
        CommandBufferHandle, SourceImageHandle, SourceImageLayout, DestinationImageHandle, DestinationImageLayout, 1,
        &ImageCopyRegion
    );
}

void GCVulkanUtilities_CopyImageToBuffer(
    const VkCommandBuffer CommandBufferHandle, const VkImage SourceImageHandle, const VkImageLayout SourceImageLayout,
    const VkBuffer DestinationBufferHandle, const uint32_t Width, const uint32_t Height, const int32_t X,
    const int32_t Y
)
{
    VkBufferImageCopy BufferImageCopyRegion = {0};
    BufferImageCopyRegion.bufferOffset = 0;
    BufferImageCopyRegion.bufferRowLength = 0;
    BufferImageCopyRegion.bufferImageHeight = 0;
    BufferImageCopyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    BufferImageCopyRegion.imageSubresource.mipLevel = 0;
    BufferImageCopyRegion.imageSubresource.baseArrayLayer = 0;
    BufferImageCopyRegion.imageSubresource.layerCount = 1;
    BufferImageCopyRegion.imageOffset = (VkOffset3D){X, Y, 0};
    BufferImageCopyRegion.imageExtent = (VkExtent3D){Width, Height, 1};

    vkCmdCopyImageToBuffer(
        CommandBufferHandle, SourceImageHandle, SourceImageLayout, DestinationBufferHandle, 1, &BufferImageCopyRegion
    );
}

void GCVulkanUtilities_CopyBuffer(
    const VkCommandBuffer CommandBufferHandle, const VkBuffer SourceBufferHandle,
    const VkBuffer DestinationBufferHandle, const VkDeviceSize Size
)
{
    VkBufferCopy BufferCopyRegion = {0};
    BufferCopyRegion.size = Size;

    vkCmdCopyBuffer(CommandBufferHandle, SourceBufferHandle, DestinationBufferHandle, 1, &BufferCopyRegion);
}

void GCVulkanUtilities_CopyBufferToImage(
    const VkCommandBuffer CommandBufferHandle, const VkBuffer SourceBufferHandle, const VkImage DestinationImageHandle,
    const uint32_t Width, const uint32_t Height
)
{
    VkBufferImageCopy BufferImageCopyRegion = {0};
    BufferImageCopyRegion.bufferOffset = 0;
    BufferImageCopyRegion.bufferRowLength = 0;
    BufferImageCopyRegion.bufferImageHeight = 0;
    BufferImageCopyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    BufferImageCopyRegion.imageSubresource.mipLevel = 0;
    BufferImageCopyRegion.imageSubresource.baseArrayLayer = 0;
    BufferImageCopyRegion.imageSubresource.layerCount = 1;
    BufferImageCopyRegion.imageOffset = (VkOffset3D){0};
    BufferImageCopyRegion.imageExtent.width = Width;
    BufferImageCopyRegion.imageExtent.height = Height;
    BufferImageCopyRegion.imageExtent.depth = 1;

    vkCmdCopyBufferToImage(
        CommandBufferHandle, SourceBufferHandle, DestinationImageHandle, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1,
        &BufferImageCopyRegion
    );
}

VkSampleCountFlagBits GCVulkanUtilities_GetMaximumUsableSampleCount(const GCRendererDevice* const Device)
{
    VkPhysicalDeviceProperties PhysicalDeviceProperties = {0};
    vkGetPhysicalDeviceProperties(GCRendererDevice_GetPhysicalDeviceHandle(Device), &PhysicalDeviceProperties);

    const VkSampleCountFlags SampleCount = PhysicalDeviceProperties.limits.framebufferColorSampleCounts &
                                           PhysicalDeviceProperties.limits.framebufferDepthSampleCounts;

    if (SampleCount & VK_SAMPLE_COUNT_64_BIT)
    {
        return VK_SAMPLE_COUNT_64_BIT;
    }
    else if (SampleCount & VK_SAMPLE_COUNT_32_BIT)
    {
        return VK_SAMPLE_COUNT_32_BIT;
    }
    else if (SampleCount & VK_SAMPLE_COUNT_16_BIT)
    {
        return VK_SAMPLE_COUNT_16_BIT;
    }
    else if (SampleCount & VK_SAMPLE_COUNT_8_BIT)
    {
        return VK_SAMPLE_COUNT_8_BIT;
    }
    else if (SampleCount & VK_SAMPLE_COUNT_4_BIT)
    {
        return VK_SAMPLE_COUNT_4_BIT;
    }
    else if (SampleCount & VK_SAMPLE_COUNT_2_BIT)
    {
        return VK_SAMPLE_COUNT_2_BIT;
    }

    return VK_SAMPLE_COUNT_1_BIT;
}

VkFormat GCVulkanUtilities_ToVkFormat(const GCRendererDevice* const Device, const GCRendererFormat Format)
{
    (void)Device;

    switch (Format)
    {
    case GCRendererFormat_SRGB: {
        return VK_FORMAT_B8G8R8A8_SRGB;

        break;
    }
    case GCRendererFormat_RGBA: {
        return VK_FORMAT_R8G8B8A8_SRGB;

        break;
    }
    case GCRendererFormat_UnsignedInteger: {
        return VK_FORMAT_R32_UINT;

        break;
    }
    case GCRendererFormat_Byte: {
        return VK_FORMAT_R8_UNORM;

        break;
    }
    case GCRendererFormat_D32: {
        return VK_FORMAT_D32_SFLOAT;

        break;
    }
    }

    GC_ASSERT_WITH_MESSAGE(false, "'%d': Invalid GCRendererFormat.");
    return VK_FORMAT_UNDEFINED;
}

VkSampleCountFlagBits GCVulkanUtilities_ToVkSampleCountFlagBits(
    const GCRendererDevice* const Device, const GCRendererSampleCount SampleCount
)
{
    switch (SampleCount)
    {
    case GCRendererSampleCount_1: {
        return VK_SAMPLE_COUNT_1_BIT;

        break;
    }
    case GCRendererSampleCount_2: {
        return VK_SAMPLE_COUNT_2_BIT;

        break;
    }
    case GCRendererSampleCount_4: {
        return VK_SAMPLE_COUNT_4_BIT;

        break;
    }
    case GCRendererSampleCount_8: {
        return VK_SAMPLE_COUNT_8_BIT;

        break;
    }
    case GCRendererSampleCount_MaximumUsable: {
        return GCVulkanUtilities_GetMaximumUsableSampleCount(Device);

        break;
    }
    }

    GC_ASSERT_WITH_MESSAGE(false, "'%d': Invalid GCRendererSampleCount.");
    return (VkSampleCountFlagBits)-1;
}
