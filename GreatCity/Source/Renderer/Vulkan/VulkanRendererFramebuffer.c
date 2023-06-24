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

#include "Renderer/Vulkan/VulkanRendererFramebuffer.h"
#include "Core/Assert.h"
#include "Core/Log.h"
#include "Core/Memory/Allocator.h"
#include "Renderer/RendererDevice.h"
#include "Renderer/RendererEnums.h"
#include "Renderer/RendererFramebuffer.h"
#include "Renderer/Vulkan/VulkanRendererCommandList.h"
#include "Renderer/Vulkan/VulkanRendererDevice.h"
#include "Renderer/Vulkan/VulkanRendererGraphicsPipeline.h"
#include "Renderer/Vulkan/VulkanRendererSwapChain.h"
#include "Renderer/Vulkan/VulkanUtilities.h"

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include <vulkan/vulkan.h>
#ifndef VMA_VULKAN_VERSION
#define VMA_VULKAN_VERSION 1001000
#endif
#include <vk_mem_alloc.h>

typedef struct GCRendererFramebuffer
{
    const GCRendererDevice* Device;
    const GCRendererSwapChain* SwapChain;
    const GCRendererGraphicsPipeline* GraphicsPipeline;

    GCRendererFramebufferAttachment* Attachments;
    uint32_t AttachmentCount;
    uint32_t Width, Height;

    VkImageView* SwapChainImageViewHandles;

    VkImage* ColorAttachmentImageHandles;
    VmaAllocation* ColorAttachmentImageAllocationHandles;
    VkImageView* ColorAttachmentImageViewHandles;

    VkImage* ColorResolveAttachmentImageHandles;
    VmaAllocation* ColorResolveAttachmentImageAllocationHandles;
    VkImageView* ColorResolveAttachmentImageViewHandles;

    VkImage* DepthAttachmentImageHandles;
    VmaAllocation* DepthAttachmentImageAllocationHandles;
    VkImageView* DepthAttachmentImageViewHandles;

    VkSampler* ColorAttachmentSampledSamplerHandles;

    VkFramebuffer* SwapChainFramebufferHandles;
    VkFramebuffer AttachmentFramebufferHandle;

    bool HasColorAttachment, HasColorAttachmentSampled, HasColorAttachmentMapped, HasColorResolveAttachment;
    bool HasDepthAtachment;
} GCRendererFramebuffer;

static uint32_t GCRendererFramebuffer_GetColorAttachmentCount(const GCRendererFramebuffer* const Framebuffer);
static uint32_t GCRendererFramebuffer_GetColorAttachmentSampledCount(const GCRendererFramebuffer* const Framebuffer);
static uint32_t GCRendererFramebuffer_GetColorResolveAttachmentCount(const GCRendererFramebuffer* const Framebuffer);
static uint32_t GCRendererFramebuffer_GetDepthAttachmentCount(const GCRendererFramebuffer* const Framebuffer);
static void GCRendererFramebuffer_CreateSwapChainImages(GCRendererFramebuffer* const Framebuffer);
static void GCRendererFramebuffer_CreateAttachments(GCRendererFramebuffer* const Framebuffer);
static void GCRendererFramebuffer_CreateSwapChainFramebuffers(GCRendererFramebuffer* const Framebuffer);
static void GCRendererFramebuffer_CreateAttachmentFramebuffer(GCRendererFramebuffer* const Framebuffer);
static void GCRendererFramebuffer_DestroyObjectsSwapChain(GCRendererFramebuffer* const Framebuffer);
static void GCRendererFramebuffer_DestroyObjectsAttachments(GCRendererFramebuffer* const Framebuffer);

GCRendererFramebuffer* GCRendererFramebuffer_Create(const GCRendererFramebufferDescription* const Description)
{
    GCRendererFramebuffer* Framebuffer = (GCRendererFramebuffer*)GCMemory_AllocateZero(sizeof(GCRendererFramebuffer));
    Framebuffer->Device = Description->Device;
    Framebuffer->SwapChain = Description->SwapChain;
    Framebuffer->GraphicsPipeline = Description->GraphicsPipeline;
    Framebuffer->AttachmentCount = Description->AttachmentCount;
    Framebuffer->Width = Description->Width;
    Framebuffer->Height = Description->Height;

    GC_ASSERT_WITH_MESSAGE(
        Description->Attachments && (Description->AttachmentCount > 0),
        "Invalid GCRendererFramebufferAttachment array. Either Description->Attachments is null or "
        "Description->AttachmentCount is less than or equal to 0."
    );

    Framebuffer->Attachments = (GCRendererFramebufferAttachment*)GCMemory_AllocateZero(
        Framebuffer->AttachmentCount * sizeof(GCRendererFramebufferAttachment)
    );
    memcpy(
        Framebuffer->Attachments, Description->Attachments,
        Framebuffer->AttachmentCount * sizeof(GCRendererFramebufferAttachment)
    );

    const uint32_t ColorAttachmentCount = GCRendererFramebuffer_GetColorAttachmentCount(Framebuffer);
    const uint32_t ColorAttachmentSampledCount = GCRendererFramebuffer_GetColorAttachmentSampledCount(Framebuffer);
    const uint32_t ColorResolveAttachmentCount = GCRendererFramebuffer_GetColorResolveAttachmentCount(Framebuffer);
    const uint32_t DepthAttachmentCount = GCRendererFramebuffer_GetDepthAttachmentCount(Framebuffer);

    if (ColorAttachmentCount > 0)
    {
        Framebuffer->HasColorAttachment = true;

        Framebuffer->ColorAttachmentImageHandles =
            (VkImage*)GCMemory_AllocateZero(ColorAttachmentCount * sizeof(VkImage));
        Framebuffer->ColorAttachmentImageAllocationHandles =
            (VmaAllocation*)GCMemory_AllocateZero(ColorAttachmentCount * sizeof(VmaAllocation));
        Framebuffer->ColorAttachmentImageViewHandles =
            (VkImageView*)GCMemory_AllocateZero(ColorAttachmentCount * sizeof(VkImageView));
    }

    if (ColorResolveAttachmentCount > 0)
    {
        Framebuffer->HasColorResolveAttachment = true;

        Framebuffer->ColorResolveAttachmentImageHandles =
            (VkImage*)GCMemory_AllocateZero(ColorResolveAttachmentCount * sizeof(VkImage));
        Framebuffer->ColorResolveAttachmentImageAllocationHandles =
            (VmaAllocation*)GCMemory_AllocateZero(ColorResolveAttachmentCount * sizeof(VmaAllocation));
        Framebuffer->ColorResolveAttachmentImageViewHandles =
            (VkImageView*)GCMemory_AllocateZero(ColorResolveAttachmentCount * sizeof(VkImageView));
    }

    if (DepthAttachmentCount > 0)
    {
        Framebuffer->HasDepthAtachment = true;

        Framebuffer->DepthAttachmentImageHandles =
            (VkImage*)GCMemory_AllocateZero(DepthAttachmentCount * sizeof(VkImage));
        Framebuffer->DepthAttachmentImageAllocationHandles =
            (VmaAllocation*)GCMemory_AllocateZero(DepthAttachmentCount * sizeof(VmaAllocation));
        Framebuffer->DepthAttachmentImageViewHandles =
            (VkImageView*)GCMemory_AllocateZero(DepthAttachmentCount * sizeof(VkImageView));
    }

    if (ColorAttachmentSampledCount > 0)
    {
        Framebuffer->HasColorAttachmentSampled = true;

        Framebuffer->ColorAttachmentSampledSamplerHandles =
            (VkSampler*)GCMemory_AllocateZero(ColorAttachmentSampledCount * sizeof(VkSampler));
    }

    GCRendererFramebuffer_CreateSwapChainImages(Framebuffer);
    GCRendererFramebuffer_CreateAttachments(Framebuffer);
    GCRendererFramebuffer_CreateSwapChainFramebuffers(Framebuffer);
    GCRendererFramebuffer_CreateAttachmentFramebuffer(Framebuffer);

    return Framebuffer;
}

void GCRendererFramebuffer_RecreateSwapChainFramebuffer(GCRendererFramebuffer* const Framebuffer)
{
    GCRendererDevice_WaitIdle(Framebuffer->Device);
    GCRendererFramebuffer_DestroyObjectsSwapChain(Framebuffer);
    GCMemory_Free(Framebuffer->SwapChainFramebufferHandles);
    GCMemory_Free(Framebuffer->SwapChainImageViewHandles);

    GCRendererFramebuffer_CreateSwapChainImages(Framebuffer);
    GCRendererFramebuffer_CreateSwapChainFramebuffers(Framebuffer);
}

void GCRendererFramebuffer_RecreateAttachmentFramebuffer(
    GCRendererFramebuffer* const Framebuffer, const uint32_t Width, const uint32_t Height
)
{
    GCRendererDevice_WaitIdle(Framebuffer->Device);
    GCRendererFramebuffer_DestroyObjectsAttachments(Framebuffer);

    Framebuffer->Width = Width;
    Framebuffer->Height = Height;
    GCRendererFramebuffer_CreateAttachments(Framebuffer);
    GCRendererFramebuffer_CreateAttachmentFramebuffer(Framebuffer);
}

void GCRendererFramebuffer_GetSize(
    const GCRendererFramebuffer* const Framebuffer, uint32_t* const Width, uint32_t* const Height
)
{
    *Width = Framebuffer->Width;
    *Height = Framebuffer->Height;
}

int32_t GCRendererFramebuffer_GetPixel(
    const GCRendererFramebuffer* const Framebuffer, const GCRendererCommandList* const CommandList,
    const uint32_t AttachmentIndex, const int32_t X, const int32_t Y
)
{
    VmaAllocator AllocatorHandle = GCRendererDevice_GetAllocatorHandle(Framebuffer->Device);

    VkBuffer ColorAttachmentDataBufferHandle = VK_NULL_HANDLE;
    VmaAllocation ColorAttachmentDataBufferAllocationHandle = VK_NULL_HANDLE;
    VmaAllocationInfo ColorAttachmentDataBufferAllocationInformation = {0};

    GCVulkanUtilities_CreateBuffer(
        Framebuffer->Device, 4, VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        VMA_ALLOCATION_CREATE_MAPPED_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
        VMA_MEMORY_USAGE_AUTO, &ColorAttachmentDataBufferHandle, &ColorAttachmentDataBufferAllocationHandle,
        &ColorAttachmentDataBufferAllocationInformation
    );

    const VkCommandBuffer CommandBufferHandle = GCRendererCommandList_BeginSingleTimeCommands(CommandList);
    GCVulkanUtilities_CopyImageToBuffer(
        CommandBufferHandle, Framebuffer->ColorAttachmentImageHandles[AttachmentIndex],
        VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, ColorAttachmentDataBufferHandle, 1, 1, X, Y
    );
    GCRendererCommandList_EndSingleTimeCommands(CommandList, CommandBufferHandle);

    int32_t Pixel = 0;
    vmaInvalidateAllocation(AllocatorHandle, ColorAttachmentDataBufferAllocationHandle, 0, VK_WHOLE_SIZE);
    Pixel = *(int32_t*)ColorAttachmentDataBufferAllocationInformation.pMappedData;

    vmaDestroyBuffer(AllocatorHandle, ColorAttachmentDataBufferHandle, ColorAttachmentDataBufferAllocationHandle);

    return Pixel;
}

void GCRendererFramebuffer_Destroy(GCRendererFramebuffer* Framebuffer)
{
    GCRendererDevice_WaitIdle(Framebuffer->Device);
    GCRendererFramebuffer_DestroyObjectsAttachments(Framebuffer);
    GCRendererFramebuffer_DestroyObjectsSwapChain(Framebuffer);

    GCMemory_Free(Framebuffer->SwapChainFramebufferHandles);

    if (Framebuffer->HasColorAttachmentSampled)
    {
        GCMemory_Free(Framebuffer->ColorAttachmentSampledSamplerHandles);
    }

    if (Framebuffer->HasDepthAtachment)
    {
        GCMemory_Free(Framebuffer->DepthAttachmentImageViewHandles);
        GCMemory_Free(Framebuffer->DepthAttachmentImageAllocationHandles);
        GCMemory_Free(Framebuffer->DepthAttachmentImageHandles);
    }

    if (Framebuffer->HasColorResolveAttachment)
    {
        GCMemory_Free(Framebuffer->ColorResolveAttachmentImageViewHandles);
        GCMemory_Free(Framebuffer->ColorResolveAttachmentImageAllocationHandles);
        GCMemory_Free(Framebuffer->ColorResolveAttachmentImageHandles);
    }

    if (Framebuffer->HasColorAttachment)
    {
        GCMemory_Free(Framebuffer->ColorAttachmentImageViewHandles);
        GCMemory_Free(Framebuffer->ColorAttachmentImageAllocationHandles);
        GCMemory_Free(Framebuffer->ColorAttachmentImageHandles);
    }

    GCMemory_Free(Framebuffer->SwapChainImageViewHandles);
    GCMemory_Free(Framebuffer->Attachments);
    GCMemory_Free(Framebuffer);
}

VkImage GCRendererFramebuffer_GetColorAttachmentImageHandle(
    const GCRendererFramebuffer* const Framebuffer, const uint32_t AttachmentIndex
)
{
    return Framebuffer->ColorAttachmentImageHandles[AttachmentIndex];
}

VkImageView GCRendererFramebuffer_GetColorAttachmentImageViewHandle(
    const GCRendererFramebuffer* const Framebuffer, const uint32_t AttachmentIndex
)
{
    return Framebuffer->ColorAttachmentImageViewHandles[AttachmentIndex];
}

VkSampler GCRendererFramebuffer_GetColorAttachmentSampledSamplerHandle(
    const GCRendererFramebuffer* const Framebuffer, const uint32_t AttachmentIndex
)
{
    return Framebuffer->ColorAttachmentSampledSamplerHandles[AttachmentIndex];
}

VkFramebuffer* GCRendererFramebuffer_GetSwapChainFramebufferHandles(const GCRendererFramebuffer* const Framebuffer)
{
    return Framebuffer->SwapChainFramebufferHandles;
}

VkFramebuffer GCRendererFramebuffer_GetAttachmentFramebufferHandle(const GCRendererFramebuffer* const Framebuffer)
{
    return Framebuffer->AttachmentFramebufferHandle;
}

VkExtent2D GCRendererFramebuffer_GetFramebufferSize(const GCRendererFramebuffer* const Framebuffer)
{
    VkExtent2D Extent = {0};
    Extent.width = Framebuffer->Width;
    Extent.height = Framebuffer->Height;

    return Extent;
}

uint32_t GCRendererFramebuffer_GetColorAttachmentCount(const GCRendererFramebuffer* const Framebuffer)
{
    uint32_t Count = 0;
    for (uint32_t Counter = 0; Counter < Framebuffer->AttachmentCount; Counter++)
    {
        if (Framebuffer->Attachments[Counter].Type == GCRendererAttachmentType_Color)
        {
            Count++;
        }
    }

    return Count;
}

uint32_t GCRendererFramebuffer_GetColorAttachmentSampledCount(const GCRendererFramebuffer* const Framebuffer)
{
    uint32_t Count = 0;
    for (uint32_t Counter = 0; Counter < Framebuffer->AttachmentCount; Counter++)
    {
        if (Framebuffer->Attachments[Counter].Type == GCRendererAttachmentType_Color)
        {
            if (Framebuffer->Attachments[Counter].Flags == GCRendererFramebufferAttachmentFlags_Sampled)
            {
                Count++;
            }
        }
    }

    return Count;
}

uint32_t GCRendererFramebuffer_GetColorResolveAttachmentCount(const GCRendererFramebuffer* const Framebuffer)
{
    uint32_t Count = 0;
    for (uint32_t Counter = 0; Counter < Framebuffer->AttachmentCount; Counter++)
    {
        if (Framebuffer->Attachments[Counter].Type == GCRendererAttachmentType_Color)
        {
            if (Framebuffer->Attachments[Counter].SampleCount > GCRendererSampleCount_1)
            {
                Count++;
            }
        }
    }

    return Count;
}

uint32_t GCRendererFramebuffer_GetDepthAttachmentCount(const GCRendererFramebuffer* const Framebuffer)
{
    uint32_t Count = 0;
    for (uint32_t Counter = 0; Counter < Framebuffer->AttachmentCount; Counter++)
    {
        if (Framebuffer->Attachments[Counter].Type == GCRendererAttachmentType_DepthStencil)
        {
            Count++;
        }
    }

    return Count;
}

void GCRendererFramebuffer_CreateSwapChainImages(GCRendererFramebuffer* const Framebuffer)
{
    const VkImage* const SwapChainImageHandles = GCRendererSwapChain_GetImageHandles(Framebuffer->SwapChain);
    const uint32_t SwapChainImageCount = GCRendererSwapChain_GetImageCount(Framebuffer->SwapChain);
    const VkFormat SwapChainFormat = GCRendererSwapChain_GetFormat(Framebuffer->SwapChain);

    Framebuffer->SwapChainImageViewHandles =
        (VkImageView*)GCMemory_AllocateZero(SwapChainImageCount * sizeof(VkImageView));

    for (uint32_t Counter = 0; Counter < SwapChainImageCount; Counter++)
    {
        GCVulkanUtilities_CreateImageView(
            Framebuffer->Device, SwapChainImageHandles[Counter], SwapChainFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1,
            &Framebuffer->SwapChainImageViewHandles[Counter]
        );
    }
}

void GCRendererFramebuffer_CreateAttachments(GCRendererFramebuffer* const Framebuffer)
{
    uint32_t ColorAttachmentIndex = 0, ColorAttachmentSampledIndex = 0, ColorResolveAttachmentIndex = 0;
    uint32_t DepthAttachmentIndex = 0;

    for (uint32_t Counter = 0; Counter < Framebuffer->AttachmentCount; Counter++)
    {
        const GCRendererFramebufferAttachment Attachment = Framebuffer->Attachments[Counter];
        const VkFormat AttachmentFormat = GCVulkanUtilities_ToVkFormat(Framebuffer->Device, Attachment.Format);
        VkSampleCountFlagBits AttachmentSampleCount =
            GCVulkanUtilities_ToVkSampleCountFlagBits(Framebuffer->Device, Attachment.SampleCount);

        VkImage* AttachmentImageHandle = NULL;
        VmaAllocation* AttachmentImageAllocationHandle = NULL;
        VkImageView* AttachmentImageViewHandle = NULL;

        VkImageUsageFlagBits AttachmentImageUsage = 0;
        VkImageTiling AttachmentImageTiling = VK_IMAGE_TILING_OPTIMAL;
        VkImageAspectFlagBits AttachmentImageAspect = VK_IMAGE_ASPECT_NONE;

        if (Attachment.Type == GCRendererAttachmentType_Color)
        {
            AttachmentImageHandle = &Framebuffer->ColorAttachmentImageHandles[ColorAttachmentIndex];
            AttachmentImageAllocationHandle = &Framebuffer->ColorAttachmentImageAllocationHandles[ColorAttachmentIndex];
            AttachmentImageViewHandle = &Framebuffer->ColorAttachmentImageViewHandles[ColorAttachmentIndex];

            AttachmentImageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
            AttachmentImageAspect = VK_IMAGE_ASPECT_COLOR_BIT;

            if (Attachment.Flags == GCRendererFramebufferAttachmentFlags_Sampled)
            {
                GCVulkanUtilities_CreateSampler(
                    Framebuffer->Device, VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_REPEAT, 1,
                    &Framebuffer->ColorAttachmentSampledSamplerHandles[ColorAttachmentSampledIndex]
                );

                ColorAttachmentSampledIndex++;
            }

            if (Attachment.Format == GCRendererFormat_UnsignedInteger)
            {
                AttachmentImageUsage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
                AttachmentImageTiling = VK_IMAGE_TILING_LINEAR;
            }

            if (Attachment.SampleCount > GCRendererSampleCount_1)
            {
                GCVulkanUtilities_CreateImage(
                    Framebuffer->Device, Framebuffer->Width, Framebuffer->Height, 1, AttachmentFormat,
                    VK_IMAGE_TILING_OPTIMAL, AttachmentSampleCount,
                    VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT,
                    VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT, VMA_MEMORY_USAGE_AUTO,
                    &Framebuffer->ColorResolveAttachmentImageHandles[ColorResolveAttachmentIndex],
                    &Framebuffer->ColorResolveAttachmentImageAllocationHandles[ColorResolveAttachmentIndex], NULL
                );
                GCVulkanUtilities_CreateImageView(
                    Framebuffer->Device, Framebuffer->ColorResolveAttachmentImageHandles[Counter], AttachmentFormat,
                    AttachmentImageAspect, 1,
                    &Framebuffer->ColorResolveAttachmentImageViewHandles[ColorResolveAttachmentIndex]
                );

                ColorResolveAttachmentIndex++;
            }

            ColorAttachmentIndex++;
        }
        else if (Attachment.Type == GCRendererAttachmentType_DepthStencil)
        {
            AttachmentImageHandle = &Framebuffer->DepthAttachmentImageHandles[DepthAttachmentIndex];
            AttachmentImageAllocationHandle = &Framebuffer->DepthAttachmentImageAllocationHandles[DepthAttachmentIndex];
            AttachmentImageViewHandle = &Framebuffer->DepthAttachmentImageViewHandles[DepthAttachmentIndex];

            AttachmentImageUsage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
            AttachmentImageAspect = VK_IMAGE_ASPECT_DEPTH_BIT;

            DepthAttachmentIndex++;
        }

        GCVulkanUtilities_CreateImage(
            Framebuffer->Device, Framebuffer->Width, Framebuffer->Height, 1, AttachmentFormat, AttachmentImageTiling,
            Attachment.Type == GCRendererAttachmentType_Color ? VK_SAMPLE_COUNT_1_BIT : AttachmentSampleCount,
            AttachmentImageUsage, VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT, VMA_MEMORY_USAGE_AUTO,
            AttachmentImageHandle, AttachmentImageAllocationHandle, NULL
        );
        GCVulkanUtilities_CreateImageView(
            Framebuffer->Device, *AttachmentImageHandle, AttachmentFormat, AttachmentImageAspect, 1,
            AttachmentImageViewHandle
        );
    }
}

void GCRendererFramebuffer_CreateSwapChainFramebuffers(GCRendererFramebuffer* const Framebuffer)
{
    const uint32_t SwapChainImageCount = GCRendererSwapChain_GetImageCount(Framebuffer->SwapChain);
    Framebuffer->SwapChainFramebufferHandles =
        (VkFramebuffer*)GCMemory_AllocateZero(SwapChainImageCount * sizeof(VkFramebuffer));

    const VkDevice DeviceHandle = GCRendererDevice_GetDeviceHandle(Framebuffer->Device);
    const VkRenderPass RenderPassHandle =
        GCRendererGraphicsPipeline_GetSwapChainRenderPassHandle(Framebuffer->GraphicsPipeline);

    const VkExtent2D SwapChainExtent = GCRendererSwapChain_GetExtent(Framebuffer->SwapChain);

    for (uint32_t Counter = 0; Counter < SwapChainImageCount; Counter++)
    {
        const VkImageView AttachmentHandles[1] = {Framebuffer->SwapChainImageViewHandles[Counter]};

        VkFramebufferCreateInfo FramebufferInformation = {0};
        FramebufferInformation.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        FramebufferInformation.renderPass = RenderPassHandle;
        FramebufferInformation.attachmentCount = 1;
        FramebufferInformation.pAttachments = AttachmentHandles;
        FramebufferInformation.width = SwapChainExtent.width;
        FramebufferInformation.height = SwapChainExtent.height;
        FramebufferInformation.layers = 1;

        GC_VULKAN_VALIDATE(
            vkCreateFramebuffer(
                DeviceHandle, &FramebufferInformation, NULL, &Framebuffer->SwapChainFramebufferHandles[Counter]
            ),
            "Failed to create a swap chain framebuffer."
        );
    }
}

void GCRendererFramebuffer_CreateAttachmentFramebuffer(GCRendererFramebuffer* const Framebuffer)
{
    const VkDevice DeviceHandle = GCRendererDevice_GetDeviceHandle(Framebuffer->Device);
    const VkRenderPass RenderPassHandle =
        GCRendererGraphicsPipeline_GetAttachmentRenderPassHandle(Framebuffer->GraphicsPipeline);

    VkImageView* AttachmentHandles = (VkImageView*)GCMemory_AllocateZero(
        (Framebuffer->AttachmentCount + GCRendererFramebuffer_GetColorResolveAttachmentCount(Framebuffer)) *
        sizeof(VkImageView)
    );
    uint32_t AttachmentCounter = 0;

    for (uint32_t Counter = 0; Counter < GCRendererFramebuffer_GetColorResolveAttachmentCount(Framebuffer); Counter++)
    {
        AttachmentHandles[AttachmentCounter] = Framebuffer->ColorResolveAttachmentImageViewHandles[Counter];
        AttachmentCounter++;
    }

    if (AttachmentCounter)
    {
        for (uint32_t Counter = 0; Counter < GCRendererFramebuffer_GetDepthAttachmentCount(Framebuffer); Counter++)
        {
            AttachmentHandles[AttachmentCounter] = Framebuffer->DepthAttachmentImageViewHandles[Counter];
            AttachmentCounter++;
        }

        for (uint32_t Counter = 0; Counter < GCRendererFramebuffer_GetColorAttachmentCount(Framebuffer); Counter++)
        {
            AttachmentHandles[AttachmentCounter] = Framebuffer->ColorAttachmentImageViewHandles[Counter];
            AttachmentCounter++;
        }
    }
    else
    {
        for (uint32_t Counter = 0; Counter < GCRendererFramebuffer_GetColorAttachmentCount(Framebuffer); Counter++)
        {
            AttachmentHandles[AttachmentCounter] = Framebuffer->ColorAttachmentImageViewHandles[Counter];
            AttachmentCounter++;
        }

        for (uint32_t Counter = 0; Counter < GCRendererFramebuffer_GetDepthAttachmentCount(Framebuffer); Counter++)
        {
            AttachmentHandles[AttachmentCounter] = Framebuffer->DepthAttachmentImageViewHandles[Counter];
            AttachmentCounter++;
        }
    }

    VkFramebufferCreateInfo FramebufferInformation = {0};
    FramebufferInformation.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    FramebufferInformation.renderPass = RenderPassHandle;
    FramebufferInformation.attachmentCount = AttachmentCounter;
    FramebufferInformation.pAttachments = AttachmentHandles;
    FramebufferInformation.width = Framebuffer->Width;
    FramebufferInformation.height = Framebuffer->Height;
    FramebufferInformation.layers = 1;

    GC_VULKAN_VALIDATE(
        vkCreateFramebuffer(DeviceHandle, &FramebufferInformation, NULL, &Framebuffer->AttachmentFramebufferHandle),
        "Failed to create an attachment framebuffer."
    );

    GCMemory_Free(AttachmentHandles);
}

void GCRendererFramebuffer_DestroyObjectsAttachments(GCRendererFramebuffer* const Framebuffer)
{
    const VkDevice DeviceHandle = GCRendererDevice_GetDeviceHandle(Framebuffer->Device);
    const VmaAllocator AllocatorHandle = GCRendererDevice_GetAllocatorHandle(Framebuffer->Device);

    vkDestroyFramebuffer(DeviceHandle, Framebuffer->AttachmentFramebufferHandle, NULL);

    for (uint32_t Counter = 0; Counter < GCRendererFramebuffer_GetColorAttachmentSampledCount(Framebuffer); Counter++)
    {
        vkDestroySampler(DeviceHandle, Framebuffer->ColorAttachmentSampledSamplerHandles[Counter], NULL);
    }

    for (uint32_t Counter = 0; Counter < GCRendererFramebuffer_GetDepthAttachmentCount(Framebuffer); Counter++)
    {
        vkDestroyImageView(DeviceHandle, Framebuffer->DepthAttachmentImageViewHandles[Counter], NULL);
        vmaDestroyImage(
            AllocatorHandle, Framebuffer->DepthAttachmentImageHandles[Counter],
            Framebuffer->DepthAttachmentImageAllocationHandles[Counter]
        );
    }

    for (uint32_t Counter = 0; Counter < GCRendererFramebuffer_GetColorResolveAttachmentCount(Framebuffer); Counter++)
    {
        vkDestroyImageView(DeviceHandle, Framebuffer->ColorResolveAttachmentImageViewHandles[Counter], NULL);
        vmaDestroyImage(
            AllocatorHandle, Framebuffer->ColorResolveAttachmentImageHandles[Counter],
            Framebuffer->ColorResolveAttachmentImageAllocationHandles[Counter]
        );
    }

    for (uint32_t Counter = 0; Counter < GCRendererFramebuffer_GetColorAttachmentCount(Framebuffer); Counter++)
    {
        vkDestroyImageView(DeviceHandle, Framebuffer->ColorAttachmentImageViewHandles[Counter], NULL);
        vmaDestroyImage(
            AllocatorHandle, Framebuffer->ColorAttachmentImageHandles[Counter],
            Framebuffer->ColorAttachmentImageAllocationHandles[Counter]
        );
    }
}

void GCRendererFramebuffer_DestroyObjectsSwapChain(GCRendererFramebuffer* const Framebuffer)
{
    const VkDevice DeviceHandle = GCRendererDevice_GetDeviceHandle(Framebuffer->Device);
    const uint32_t SwapChainImageCount = GCRendererSwapChain_GetImageCount(Framebuffer->SwapChain);

    for (uint32_t Counter = 0; Counter < SwapChainImageCount; Counter++)
    {
        vkDestroyFramebuffer(DeviceHandle, Framebuffer->SwapChainFramebufferHandles[Counter], NULL);
        vkDestroyImageView(DeviceHandle, Framebuffer->SwapChainImageViewHandles[Counter], NULL);
    }
}