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
#include "Renderer/Vulkan/VulkanRendererDevice.h"
#include "Renderer/Vulkan/VulkanRendererSwapChain.h"
#include "Renderer/Vulkan/VulkanRendererGraphicsPipeline.h"
#include "Renderer/Vulkan/VulkanUtilities.h"
#include "Renderer/RendererFramebuffer.h"
#include "Renderer/RendererDevice.h"
#include "Core/Memory/Allocator.h"
#include "Core/Log.h"
#include "Core/Assert.h"

#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include <vulkan/vulkan.h>

typedef struct GCRendererFramebuffer
{
	const GCRendererDevice* Device;
	const GCRendererSwapChain* SwapChain;
	const GCRendererGraphicsPipeline* GraphicsPipeline;

	GCRendererFramebufferAttachment* Attachments;
	uint32_t AttachmentCount;

	uint32_t Width, Height;

	VkImage* ColorAttachmentImageHandles;
	VkDeviceMemory* ColorAttachmentImageMemoryHandles;
	VkImageView* ColorAttachmentImageViewHandles;

	VkImage* ColorAttachmentMappedImageHandles;
	VkDeviceMemory* ColorAttachmentMappedImageMemoryHandles;
	VkImageView* ColorAttachmentMappedImageViewHandles;
	void** ColorAttachmentMappedImageData;

	VkImage* ColorResolveAttachmentImageHandles;
	VkDeviceMemory* ColorResolveAttachmentImageMemoryHandles;
	VkImageView* ColorResolveAttachmentImageViewHandles;

	VkImage* DepthAttachmentImageHandles;
	VkDeviceMemory* DepthAttachmentImageMemoryHandles;
	VkImageView* DepthAttachmentImageViewHandles;

	VkSampler* ColorAttachmentSampledSamplerHandles;

	VkFramebuffer* SwapChainFramebufferHandles;
	VkFramebuffer AttachmentFramebufferHandle;
} GCRendererFramebuffer;

static uint32_t GCRendererFramebuffer_GetColorAttachmentCount(const GCRendererFramebuffer* const Framebuffer);
static uint32_t GCRendererFramebuffer_GetColorAttachmentSampledCount(const GCRendererFramebuffer* const Framebuffer);
static uint32_t GCRendererFramebuffer_GetColorAttachmentMappedCount(const GCRendererFramebuffer* const Framebuffer);
static uint32_t GCRendererFramebuffer_GetColorResolveAttachmentCount(const GCRendererFramebuffer* const Framebuffer);
static uint32_t GCRendererFramebuffer_GetDepthAttachmentCount(const GCRendererFramebuffer* const Framebuffer);
static void GCRendererFramebuffer_CreateAttachments(GCRendererFramebuffer* const Framebuffer);
static void GCRendererFramebuffer_CreateSwapChainFramebuffers(GCRendererFramebuffer* const Framebuffer);
static void GCRendererFramebuffer_CreateAttachmentFramebuffer(GCRendererFramebuffer* const Framebuffer);
static void GCRendererFramebuffer_DestroyObjectsSwapChain(GCRendererFramebuffer* const Framebuffer);
static void GCRendererFramebuffer_DestroyObjectsAttachments(GCRendererFramebuffer* const Framebuffer);

static VkFormat GCRendererFramebuffer_ToVkFormat(const GCRendererFramebuffer* const Framebuffer, const GCRendererFramebufferAttachmentFormat Format);
static VkSampleCountFlagBits GCRendererFramebuffer_ToVkSampleCountFlagBits(const GCRendererFramebuffer* const Framebuffer, const GCRendererFramebufferAttachmentSampleCount SampleCount);

GCRendererFramebuffer* GCRendererFramebuffer_Create(const GCRendererDevice* const Device, const GCRendererSwapChain* const SwapChain, const GCRendererGraphicsPipeline* const GraphicsPipeline, const uint32_t Width, const uint32_t Height, const GCRendererFramebufferAttachment* const Attachments, const uint32_t AttachmentCount)
{
	GCRendererFramebuffer* Framebuffer = (GCRendererFramebuffer*)GCMemory_Allocate(sizeof(GCRendererFramebuffer));
	Framebuffer->Device = Device;
	Framebuffer->SwapChain = SwapChain;
	Framebuffer->GraphicsPipeline = GraphicsPipeline;
	Framebuffer->Attachments = NULL;
	Framebuffer->AttachmentCount = AttachmentCount;
	Framebuffer->Width = Width;
	Framebuffer->Height = Height;
	Framebuffer->ColorAttachmentImageHandles = NULL;
	Framebuffer->ColorAttachmentImageMemoryHandles = NULL;
	Framebuffer->ColorAttachmentImageViewHandles = NULL;

	Framebuffer->ColorAttachmentMappedImageHandles = NULL;
	Framebuffer->ColorAttachmentMappedImageMemoryHandles = NULL;
	Framebuffer->ColorAttachmentMappedImageViewHandles = NULL;
	Framebuffer->ColorAttachmentMappedImageData = NULL;

	Framebuffer->ColorResolveAttachmentImageHandles = NULL;
	Framebuffer->ColorResolveAttachmentImageMemoryHandles = NULL;
	Framebuffer->ColorResolveAttachmentImageViewHandles = NULL;

	Framebuffer->DepthAttachmentImageHandles = NULL;
	Framebuffer->DepthAttachmentImageMemoryHandles = NULL;
	Framebuffer->DepthAttachmentImageViewHandles = NULL;
	
	Framebuffer->ColorAttachmentSampledSamplerHandles = NULL;
	
	Framebuffer->SwapChainFramebufferHandles = NULL;
	Framebuffer->AttachmentFramebufferHandle = VK_NULL_HANDLE;

	Framebuffer->Attachments = (GCRendererFramebufferAttachment*)GCMemory_Allocate(Framebuffer->AttachmentCount * sizeof(GCRendererFramebufferAttachment));
	memcpy(Framebuffer->Attachments, Attachments, Framebuffer->AttachmentCount * sizeof(GCRendererFramebufferAttachment));

	const uint32_t ColorAttachmentCount = GCRendererFramebuffer_GetColorAttachmentCount(Framebuffer);
	const uint32_t ColorAttachmentMappedCount = GCRendererFramebuffer_GetColorAttachmentMappedCount(Framebuffer);
	const uint32_t ColorAttachmentSampledCount = GCRendererFramebuffer_GetColorAttachmentSampledCount(Framebuffer);
	const uint32_t ColorResolveAttachmentCount = GCRendererFramebuffer_GetColorResolveAttachmentCount(Framebuffer);
	const uint32_t DepthAttachmentCount = GCRendererFramebuffer_GetDepthAttachmentCount(Framebuffer);

	Framebuffer->ColorAttachmentImageHandles = (VkImage*)GCMemory_Allocate(ColorAttachmentCount * sizeof(VkImage));
	Framebuffer->ColorAttachmentImageMemoryHandles = (VkDeviceMemory*)GCMemory_Allocate(ColorAttachmentCount * sizeof(VkDeviceMemory));
	Framebuffer->ColorAttachmentImageViewHandles = (VkImageView*)GCMemory_Allocate(ColorAttachmentCount * sizeof(VkImageView));

	Framebuffer->ColorAttachmentMappedImageHandles = (VkImage*)GCMemory_Allocate(ColorAttachmentMappedCount * sizeof(VkImage));
	Framebuffer->ColorAttachmentMappedImageMemoryHandles = (VkDeviceMemory*)GCMemory_Allocate(ColorAttachmentMappedCount * sizeof(VkDeviceMemory));
	Framebuffer->ColorAttachmentMappedImageViewHandles = (VkImageView*)GCMemory_Allocate(ColorAttachmentMappedCount * sizeof(VkImageView));
	Framebuffer->ColorAttachmentMappedImageData = (void**)GCMemory_Allocate(GCRendererFramebuffer_GetColorAttachmentMappedCount(Framebuffer) * sizeof(void*));

	Framebuffer->ColorResolveAttachmentImageHandles = (VkImage*)GCMemory_Allocate(ColorResolveAttachmentCount * sizeof(VkImage));
	Framebuffer->ColorResolveAttachmentImageMemoryHandles = (VkDeviceMemory*)GCMemory_Allocate(ColorResolveAttachmentCount * sizeof(VkDeviceMemory));
	Framebuffer->ColorResolveAttachmentImageViewHandles = (VkImageView*)GCMemory_Allocate(ColorResolveAttachmentCount * sizeof(VkImageView));

	Framebuffer->DepthAttachmentImageHandles = (VkImage*)GCMemory_Allocate(DepthAttachmentCount * sizeof(VkImage));
	Framebuffer->DepthAttachmentImageMemoryHandles = (VkDeviceMemory*)GCMemory_Allocate(DepthAttachmentCount * sizeof(VkDeviceMemory));
	Framebuffer->DepthAttachmentImageViewHandles = (VkImageView*)GCMemory_Allocate(DepthAttachmentCount * sizeof(VkImageView));

	Framebuffer->ColorAttachmentSampledSamplerHandles = (VkSampler*)GCMemory_Allocate(ColorAttachmentSampledCount * sizeof(VkSampler));

	GCRendererFramebuffer_CreateAttachments(Framebuffer);
	GCRendererFramebuffer_CreateSwapChainFramebuffers(Framebuffer);
	GCRendererFramebuffer_CreateAttachmentFramebuffer(Framebuffer);

	return Framebuffer;
}

void GCRendererFramebuffer_RecreateSwapChainFramebuffer(GCRendererFramebuffer* const Framebuffer)
{
	GCRendererDevice_WaitIdle(Framebuffer->Device);
	GCRendererFramebuffer_DestroyObjectsSwapChain(Framebuffer);

	GCRendererFramebuffer_CreateSwapChainFramebuffers(Framebuffer);
}

void GCRendererFramebuffer_RecreateAttachmentFramebuffer(GCRendererFramebuffer* const Framebuffer, const uint32_t Width, const uint32_t Height)
{
	GCRendererDevice_WaitIdle(Framebuffer->Device);
	GCRendererFramebuffer_DestroyObjectsAttachments(Framebuffer);

	Framebuffer->Width = Width;
	Framebuffer->Height = Height;
	GCRendererFramebuffer_CreateAttachments(Framebuffer);
	GCRendererFramebuffer_CreateAttachmentFramebuffer(Framebuffer);
}

int32_t GCRendererFramebuffer_GetPixel(const GCRendererFramebuffer* const Framebuffer, const GCRendererCommandList* const CommandList, const uint32_t ColorAttachmentIndex, const uint32_t ColorAttachmentMappedIndex)
{
	const VkCommandBuffer CommandBufferHandle = GCVulkanUtilities_BeginSingleTimeCommands(Framebuffer->Device, CommandList);

	GCVulkanUtilities_TransitionImageLayout(CommandBufferHandle, Framebuffer->ColorAttachmentImageHandles[ColorAttachmentIndex], 1, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
	GCVulkanUtilities_TransitionImageLayout(CommandBufferHandle, Framebuffer->ColorAttachmentMappedImageHandles[ColorAttachmentMappedIndex], 1, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	GCVulkanUtilities_CopyImage(CommandBufferHandle, Framebuffer->ColorAttachmentImageHandles[ColorAttachmentIndex], VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, Framebuffer->ColorAttachmentMappedImageHandles[ColorAttachmentMappedIndex], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, Framebuffer->Width, Framebuffer->Height);
	GCVulkanUtilities_TransitionImageLayout(CommandBufferHandle, Framebuffer->ColorAttachmentImageHandles[ColorAttachmentIndex], 1, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

	GCVulkanUtilities_EndSingleTimeCommands(Framebuffer->Device, CommandList, CommandBufferHandle);

	int32_t Pixel = 0;

	Pixel = *(int32_t*)Framebuffer->ColorAttachmentMappedImageData[0];

	return Pixel;
}

void GCRendererFramebuffer_Destroy(GCRendererFramebuffer* Framebuffer)
{
	GCRendererDevice_WaitIdle(Framebuffer->Device);

	GCRendererFramebuffer_DestroyObjectsAttachments(Framebuffer);
	GCRendererFramebuffer_DestroyObjectsSwapChain(Framebuffer);

	GCMemory_Free(Framebuffer->SwapChainFramebufferHandles);

	GCMemory_Free(Framebuffer->ColorAttachmentSampledSamplerHandles);

	GCMemory_Free(Framebuffer->DepthAttachmentImageViewHandles);
	GCMemory_Free(Framebuffer->DepthAttachmentImageMemoryHandles);
	GCMemory_Free(Framebuffer->DepthAttachmentImageHandles);

	GCMemory_Free(Framebuffer->ColorResolveAttachmentImageViewHandles);
	GCMemory_Free(Framebuffer->ColorResolveAttachmentImageMemoryHandles);
	GCMemory_Free(Framebuffer->ColorResolveAttachmentImageHandles);

	GCMemory_Free(Framebuffer->ColorAttachmentMappedImageData);
	GCMemory_Free(Framebuffer->ColorAttachmentMappedImageViewHandles);
	GCMemory_Free(Framebuffer->ColorAttachmentMappedImageMemoryHandles);
	GCMemory_Free(Framebuffer->ColorAttachmentMappedImageHandles);

	GCMemory_Free(Framebuffer->ColorAttachmentImageViewHandles);
	GCMemory_Free(Framebuffer->ColorAttachmentImageMemoryHandles);
	GCMemory_Free(Framebuffer->ColorAttachmentImageHandles);

	GCMemory_Free(Framebuffer->Attachments);
	GCMemory_Free(Framebuffer);
}

VkImageView GCRendererFramebuffer_GetColorAttachmentImageViewHandle(const GCRendererFramebuffer* const Framebuffer, const uint32_t AttachmentIndex)
{
	return Framebuffer->ColorAttachmentImageViewHandles[AttachmentIndex];
}

VkSampler GCRendererFramebuffer_GetColorAttachmentSampledSamplerHandle(const GCRendererFramebuffer* const Framebuffer, const uint32_t AttachmentIndex)
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
	VkExtent2D Extent = { 0 };
	Extent.width = Framebuffer->Width;
	Extent.height = Framebuffer->Height;

	return Extent;
}

uint32_t GCRendererFramebuffer_GetColorAttachmentCount(const GCRendererFramebuffer* const Framebuffer)
{
	uint32_t Count = 0;
	for (uint32_t Counter = 0; Counter < Framebuffer->AttachmentCount; Counter++)
	{
		if (Framebuffer->Attachments[Counter].Type == GCRendererFramebufferAttachmentType_Color)
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
		if (Framebuffer->Attachments[Counter].Type == GCRendererFramebufferAttachmentType_Color)
		{
			if (Framebuffer->Attachments[Counter].Flags == GCRendererFramebufferAttachmentFlags_Sampled)
			{
				Count++;
			}
		}
	}

	return Count;
}

uint32_t GCRendererFramebuffer_GetColorAttachmentMappedCount(const GCRendererFramebuffer* const Framebuffer)
{
	uint32_t Count = 0;
	for (uint32_t Counter = 0; Counter < Framebuffer->AttachmentCount; Counter++)
	{
		if (Framebuffer->Attachments[Counter].Type == GCRendererFramebufferAttachmentType_Color)
		{
			if (Framebuffer->Attachments[Counter].Flags == GCRendererFramebufferAttachmentFlags_Mapped)
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
		if (Framebuffer->Attachments[Counter].Type == GCRendererFramebufferAttachmentType_Color)
		{
			if (Framebuffer->Attachments[Counter].SampleCount > GCRendererFramebufferAttachmentSampleCount_1)
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
		if (Framebuffer->Attachments[Counter].Type == GCRendererFramebufferAttachmentType_DepthStencil)
		{
			Count++;
		}
	}

	return Count;
}

void GCRendererFramebuffer_CreateAttachments(GCRendererFramebuffer* const Framebuffer)
{
	uint32_t ColorAttachmentIndex = 0, ColorAttachmentSampledIndex = 0, ColorAttachmentMappedIndex = 0, ColorResolveAttachmentIndex = 0;
	uint32_t DepthAttachmentIndex = 0;

	for (uint32_t Counter = 0; Counter < Framebuffer->AttachmentCount; Counter++)
	{
		const GCRendererFramebufferAttachment Attachment = Framebuffer->Attachments[Counter];
		const VkFormat AttachmentFormat = GCRendererFramebuffer_ToVkFormat(Framebuffer, Attachment.Format);
		VkSampleCountFlagBits AttachmentSampleCount = GCRendererFramebuffer_ToVkSampleCountFlagBits(Framebuffer, Attachment.SampleCount);

		VkImage* AttachmentImageHandle = NULL;
		VkDeviceMemory* AttachmentImageMemoryHandle = NULL;
		VkImageView* AttachmentImageViewHandle = NULL;

		VkImageUsageFlagBits AttachmentImageUsage = 0;
		VkMemoryPropertyFlagBits AttachmentMemoryProperty = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
		VkImageAspectFlagBits AttachmentImageAspect = VK_IMAGE_ASPECT_NONE;

		if (Attachment.Type == GCRendererFramebufferAttachmentType_Color)
		{
			AttachmentImageHandle = &Framebuffer->ColorAttachmentImageHandles[ColorAttachmentIndex];
			AttachmentImageMemoryHandle = &Framebuffer->ColorAttachmentImageMemoryHandles[ColorAttachmentIndex];
			AttachmentImageViewHandle = &Framebuffer->ColorAttachmentImageViewHandles[ColorAttachmentIndex];

			AttachmentImageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
			AttachmentImageAspect = VK_IMAGE_ASPECT_COLOR_BIT;

			if (Attachment.Flags == GCRendererFramebufferAttachmentFlags_Sampled)
			{
				GCVulkanUtilities_CreateSampler(Framebuffer->Device, VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_REPEAT, 1, &Framebuffer->ColorAttachmentSampledSamplerHandles[ColorAttachmentSampledIndex]);

				ColorAttachmentSampledIndex++;
			}

			if (Attachment.Flags == GCRendererFramebufferAttachmentFlags_Mapped)
			{
				AttachmentImageUsage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
			}

			if (Attachment.SampleCount > GCRendererFramebufferAttachmentSampleCount_1)
			{
				GCVulkanUtilities_CreateImage(Framebuffer->Device, Framebuffer->Width, Framebuffer->Height, 1, AttachmentFormat, VK_IMAGE_TILING_OPTIMAL, AttachmentSampleCount, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &Framebuffer->ColorResolveAttachmentImageHandles[ColorResolveAttachmentIndex], &Framebuffer->ColorResolveAttachmentImageMemoryHandles[ColorResolveAttachmentIndex]);
				GCVulkanUtilities_CreateImageView(Framebuffer->Device, Framebuffer->ColorResolveAttachmentImageHandles[Counter], AttachmentFormat, AttachmentImageAspect, 1, &Framebuffer->ColorResolveAttachmentImageViewHandles[ColorResolveAttachmentIndex]);

				ColorResolveAttachmentIndex++;
			}

			ColorAttachmentIndex++;
		}
		else if(Attachment.Type == GCRendererFramebufferAttachmentType_DepthStencil)
		{
			AttachmentImageHandle = &Framebuffer->DepthAttachmentImageHandles[DepthAttachmentIndex];
			AttachmentImageMemoryHandle = &Framebuffer->DepthAttachmentImageMemoryHandles[DepthAttachmentIndex];
			AttachmentImageViewHandle = &Framebuffer->DepthAttachmentImageViewHandles[DepthAttachmentIndex];

			AttachmentImageUsage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
			AttachmentImageAspect = VK_IMAGE_ASPECT_DEPTH_BIT;

			DepthAttachmentIndex++;
		}

		GCVulkanUtilities_CreateImage(Framebuffer->Device, Framebuffer->Width, Framebuffer->Height, 1, AttachmentFormat, VK_IMAGE_TILING_OPTIMAL, Attachment.Type == GCRendererFramebufferAttachmentType_Color ? VK_SAMPLE_COUNT_1_BIT : AttachmentSampleCount, AttachmentImageUsage, AttachmentMemoryProperty, AttachmentImageHandle, AttachmentImageMemoryHandle);
		GCVulkanUtilities_CreateImageView(Framebuffer->Device, *AttachmentImageHandle, AttachmentFormat, AttachmentImageAspect, 1, AttachmentImageViewHandle);

		if (Attachment.Flags == GCRendererFramebufferAttachmentFlags_Mapped)
		{
			GCVulkanUtilities_CreateImage(Framebuffer->Device, Framebuffer->Width, Framebuffer->Height, 1, AttachmentFormat, VK_IMAGE_TILING_OPTIMAL, Attachment.Type == GCRendererFramebufferAttachmentType_Color ? VK_SAMPLE_COUNT_1_BIT : AttachmentSampleCount, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &Framebuffer->ColorAttachmentMappedImageHandles[ColorAttachmentMappedIndex], &Framebuffer->ColorAttachmentMappedImageMemoryHandles[ColorAttachmentMappedIndex]);
			GCVulkanUtilities_CreateImageView(Framebuffer->Device, Framebuffer->ColorAttachmentMappedImageHandles[ColorAttachmentMappedIndex], AttachmentFormat, AttachmentImageAspect, 1, &Framebuffer->ColorAttachmentMappedImageViewHandles[ColorAttachmentMappedIndex]);

			vkMapMemory(GCRendererDevice_GetDeviceHandle(Framebuffer->Device), Framebuffer->ColorAttachmentMappedImageMemoryHandles[ColorAttachmentMappedIndex], 0, VK_WHOLE_SIZE, 0, &Framebuffer->ColorAttachmentMappedImageData[ColorAttachmentMappedIndex]);

			ColorAttachmentMappedIndex++;
		}
	}
}

void GCRendererFramebuffer_CreateSwapChainFramebuffers(GCRendererFramebuffer* const Framebuffer)
{
	const uint32_t SwapChainImageCount = GCRendererSwapChain_GetImageCount(Framebuffer->SwapChain);
	Framebuffer->SwapChainFramebufferHandles = (VkFramebuffer*)GCMemory_Allocate(SwapChainImageCount * sizeof(VkFramebuffer));

	const VkDevice DeviceHandle = GCRendererDevice_GetDeviceHandle(Framebuffer->Device);
	const VkImageView* const SwapChainImageViewHandles = GCRendererSwapChain_GetImageViewHandles(Framebuffer->SwapChain);
	const VkRenderPass RenderPassHandle = GCRendererGraphicsPipeline_GetSwapChainRenderPassHandle(Framebuffer->GraphicsPipeline);

	const VkExtent2D SwapChainExtent = GCRendererSwapChain_GetExtent(Framebuffer->SwapChain);

	for (uint32_t Counter = 0; Counter < SwapChainImageCount; Counter++)
	{
		const VkImageView AttachmentHandles[1] =
		{
			SwapChainImageViewHandles[Counter]
		};

		VkFramebufferCreateInfo FramebufferInformation = { 0 };
		FramebufferInformation.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		FramebufferInformation.renderPass = RenderPassHandle;
		FramebufferInformation.attachmentCount = 1;
		FramebufferInformation.pAttachments = AttachmentHandles;
		FramebufferInformation.width = SwapChainExtent.width;
		FramebufferInformation.height = SwapChainExtent.height;
		FramebufferInformation.layers = 1;

		GC_VULKAN_VALIDATE(vkCreateFramebuffer(DeviceHandle, &FramebufferInformation, NULL, &Framebuffer->SwapChainFramebufferHandles[Counter]), "Failed to create a Vulkan swap chain framebuffer");
	}
}

void GCRendererFramebuffer_CreateAttachmentFramebuffer(GCRendererFramebuffer* const Framebuffer)
{
	const VkDevice DeviceHandle = GCRendererDevice_GetDeviceHandle(Framebuffer->Device);
	const VkRenderPass RenderPassHandle = GCRendererGraphicsPipeline_GetTextureRenderPassHandle(Framebuffer->GraphicsPipeline);

	VkImageView* AttachmentHandles = (VkImageView*)GCMemory_Allocate((Framebuffer->AttachmentCount + GCRendererFramebuffer_GetColorResolveAttachmentCount(Framebuffer)) * sizeof(VkImageView));
	uint32_t AttachmentCounter = 0;

	for (uint32_t Counter = 0; Counter < GCRendererFramebuffer_GetColorResolveAttachmentCount(Framebuffer); Counter++)
	{
		AttachmentHandles[AttachmentCounter] = Framebuffer->ColorResolveAttachmentImageViewHandles[Counter];
		AttachmentCounter++;
	}

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

	VkFramebufferCreateInfo FramebufferInformation = { 0 };
	FramebufferInformation.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	FramebufferInformation.renderPass = RenderPassHandle;
	FramebufferInformation.attachmentCount = AttachmentCounter;
	FramebufferInformation.pAttachments = AttachmentHandles;
	FramebufferInformation.width = Framebuffer->Width;
	FramebufferInformation.height = Framebuffer->Height;
	FramebufferInformation.layers = 1;

	GC_VULKAN_VALIDATE(vkCreateFramebuffer(DeviceHandle, &FramebufferInformation, NULL, &Framebuffer->AttachmentFramebufferHandle), "Failed to create a Vulkan attachment framebuffer.");

	GCMemory_Free(AttachmentHandles);
}

void GCRendererFramebuffer_DestroyObjectsAttachments(GCRendererFramebuffer* const Framebuffer)
{
	const VkDevice DeviceHandle = GCRendererDevice_GetDeviceHandle(Framebuffer->Device);

	vkDestroyFramebuffer(DeviceHandle, Framebuffer->AttachmentFramebufferHandle, NULL);

	for (uint32_t Counter = 0; Counter < GCRendererFramebuffer_GetColorAttachmentSampledCount(Framebuffer); Counter++)
	{
		vkDestroySampler(DeviceHandle, Framebuffer->ColorAttachmentSampledSamplerHandles[Counter], NULL);
	}

	for (uint32_t Counter = 0; Counter < GCRendererFramebuffer_GetDepthAttachmentCount(Framebuffer); Counter++)
	{
		vkDestroyImageView(DeviceHandle, Framebuffer->DepthAttachmentImageViewHandles[Counter], NULL);
		vkFreeMemory(DeviceHandle, Framebuffer->DepthAttachmentImageMemoryHandles[Counter], NULL);
		vkDestroyImage(DeviceHandle, Framebuffer->DepthAttachmentImageHandles[Counter], NULL);
	}

	for (uint32_t Counter = 0; Counter < GCRendererFramebuffer_GetColorResolveAttachmentCount(Framebuffer); Counter++)
	{
		vkDestroyImageView(DeviceHandle, Framebuffer->ColorResolveAttachmentImageViewHandles[Counter], NULL);
		vkFreeMemory(DeviceHandle, Framebuffer->ColorResolveAttachmentImageMemoryHandles[Counter], NULL);
		vkDestroyImage(DeviceHandle, Framebuffer->ColorResolveAttachmentImageHandles[Counter], NULL);
	}

	for (uint32_t Counter = 0; Counter < GCRendererFramebuffer_GetColorAttachmentMappedCount(Framebuffer); Counter++)
	{
		vkDestroyImageView(DeviceHandle, Framebuffer->ColorAttachmentMappedImageViewHandles[Counter], NULL);
		vkFreeMemory(DeviceHandle, Framebuffer->ColorAttachmentMappedImageMemoryHandles[Counter], NULL);
		vkDestroyImage(DeviceHandle, Framebuffer->ColorAttachmentMappedImageHandles[Counter], NULL);
	}

	for (uint32_t Counter = 0; Counter < GCRendererFramebuffer_GetColorAttachmentCount(Framebuffer); Counter++)
	{
		vkDestroyImageView(DeviceHandle, Framebuffer->ColorAttachmentImageViewHandles[Counter], NULL);
		vkFreeMemory(DeviceHandle, Framebuffer->ColorAttachmentImageMemoryHandles[Counter], NULL);
		vkDestroyImage(DeviceHandle, Framebuffer->ColorAttachmentImageHandles[Counter], NULL);
	}
}

void GCRendererFramebuffer_DestroyObjectsSwapChain(GCRendererFramebuffer* const Framebuffer)
{
	const VkDevice DeviceHandle = GCRendererDevice_GetDeviceHandle(Framebuffer->Device);
	const uint32_t SwapChainImageCount = GCRendererSwapChain_GetImageCount(Framebuffer->SwapChain);

	for (uint32_t Counter = 0; Counter < SwapChainImageCount; Counter++)
	{
		vkDestroyFramebuffer(DeviceHandle, Framebuffer->SwapChainFramebufferHandles[Counter], NULL);
	}
}

VkFormat GCRendererFramebuffer_ToVkFormat(const GCRendererFramebuffer* const Framebuffer, const GCRendererFramebufferAttachmentFormat Format)
{
	(void)Framebuffer;

	switch (Format)
	{
		case GCRendererFramebufferAttachmentFormat_SRGB:
		{
			return VK_FORMAT_B8G8R8A8_SRGB;

			break;
		}
		case GCRendererFramebufferAttachmentFormat_Integer:
		{
			return VK_FORMAT_R32_SINT;

			break;
		}
		case GCRendererFramebufferAttachmentFormat_D32:
		{
			return VK_FORMAT_D32_SFLOAT;

			break;
		}
	}

	GC_ASSERT_WITH_MESSAGE(false, "'%d': Invalid GCRendererFramebufferAttachmentFormat");
	return VK_FORMAT_UNDEFINED;
}

VkSampleCountFlagBits GCRendererFramebuffer_ToVkSampleCountFlagBits(const GCRendererFramebuffer* const Framebuffer, const GCRendererFramebufferAttachmentSampleCount SampleCount)
{
	switch (SampleCount)
	{
		case GCRendererFramebufferAttachmentSampleCount_1:
		{
			return VK_SAMPLE_COUNT_1_BIT;

			break;
		}
		case GCRendererFramebufferAttachmentSampleCount_2:
		{
			return VK_SAMPLE_COUNT_2_BIT;

			break;
		}
		case GCRendererFramebufferAttachmentSampleCount_4:
		{
			return VK_SAMPLE_COUNT_4_BIT;

			break;
		}
		case GCRendererFramebufferAttachmentSampleCount_8:
		{
			return VK_SAMPLE_COUNT_8_BIT;

			break;
		}
		case GCRendererFramebufferAttachmentSampleCount_MaximumUsable:
		{
			return GCRendererSwapChain_GetMaximumUsableSampleCount(Framebuffer->SwapChain);

			break;
		}
	}

	GC_ASSERT_WITH_MESSAGE(false, "'%d': Invalid GCRendererFramebufferAttachmentSampleCount");
	return (VkSampleCountFlagBits)-1;
}