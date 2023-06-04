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

#include <stdint.h>
#include <stdbool.h>

#include <vulkan/vulkan.h>

typedef struct GCRendererFramebuffer
{
	const GCRendererDevice* Device;
	const GCRendererSwapChain* SwapChain;
	const GCRendererGraphicsPipeline* GraphicsPipeline;

	VkImage TextureImageHandle, TextureDepthImageHandle;
	VkDeviceMemory TextureImageMemoryHandle, TextureDepthImageMemoryHandle;
	VkImageView TextureImageViewHandle, TextureDepthImageViewHandle;
	VkSampler TextureImageSamplerHandle;

	VkFramebuffer TextureFramebufferHandle;
	VkFramebuffer* SwapChainFramebufferHandles;

	VkExtent2D TextureExtent;
} GCRendererFramebuffer;

static void GCRendererFramebuffer_CreateTexture(GCRendererFramebuffer* const Framebuffer, const bool IsResize);
static void GCRendererFramebuffer_CreateTextureFramebuffer(GCRendererFramebuffer* const Framebuffer);
static void GCRendererFramebuffer_CreateSwapChainFramebuffers(GCRendererFramebuffer* const Framebuffer);
static void GCRendererFramebuffer_DestroyObjectsTexture(GCRendererFramebuffer* const Framebuffer, const bool IsResize);
static void GCRendererFramebuffer_DestroyObjectsSwapChain(GCRendererFramebuffer* const Framebuffer);

GCRendererFramebuffer* GCRendererFramebuffer_Create(const GCRendererDevice* const Device, const GCRendererSwapChain* const SwapChain, const GCRendererGraphicsPipeline* const GraphicsPipeline)
{
	GCRendererFramebuffer* Framebuffer = (GCRendererFramebuffer*)GCMemory_Allocate(sizeof(GCRendererFramebuffer));
	Framebuffer->Device = Device;
	Framebuffer->SwapChain = SwapChain;
	Framebuffer->GraphicsPipeline = GraphicsPipeline;
	Framebuffer->TextureImageHandle = VK_NULL_HANDLE;
	Framebuffer->TextureDepthImageHandle = VK_NULL_HANDLE;
	Framebuffer->TextureImageMemoryHandle = VK_NULL_HANDLE;
	Framebuffer->TextureDepthImageMemoryHandle = VK_NULL_HANDLE;
	Framebuffer->TextureImageViewHandle = VK_NULL_HANDLE;
	Framebuffer->TextureDepthImageViewHandle = VK_NULL_HANDLE;
	Framebuffer->TextureFramebufferHandle = VK_NULL_HANDLE;
	Framebuffer->TextureImageSamplerHandle = VK_NULL_HANDLE;
	Framebuffer->TextureFramebufferHandle = VK_NULL_HANDLE;
	Framebuffer->SwapChainFramebufferHandles = NULL;
	Framebuffer->TextureExtent = GCRendererSwapChain_GetExtent(Framebuffer->SwapChain);

	GCRendererFramebuffer_CreateTexture(Framebuffer, false);
	GCRendererFramebuffer_CreateTextureFramebuffer(Framebuffer);
	GCRendererFramebuffer_CreateSwapChainFramebuffers(Framebuffer);

	return Framebuffer;
}

void GCRendererFramebuffer_RecreateTexture(GCRendererFramebuffer* const Framebuffer, const uint32_t Width, const uint32_t Height)
{
	GCRendererDevice_WaitIdle(Framebuffer->Device);

	GCRendererFramebuffer_DestroyObjectsTexture(Framebuffer, true);

	Framebuffer->TextureExtent = (VkExtent2D){ Width, Height };

	GCRendererFramebuffer_CreateTexture(Framebuffer, true);
	GCRendererFramebuffer_CreateTextureFramebuffer(Framebuffer);
}

void GCRendererFramebuffer_RecreateSwapChain(GCRendererFramebuffer* const Framebuffer)
{
	GCRendererDevice_WaitIdle(Framebuffer->Device);

	GCRendererFramebuffer_DestroyObjectsSwapChain(Framebuffer);

	GCRendererFramebuffer_CreateSwapChainFramebuffers(Framebuffer);
}

void GCRendererFramebuffer_Destroy(GCRendererFramebuffer* Framebuffer)
{
	GCRendererDevice_WaitIdle(Framebuffer->Device);

	GCRendererFramebuffer_DestroyObjectsTexture(Framebuffer, false);
	GCRendererFramebuffer_DestroyObjectsSwapChain(Framebuffer);

	GCMemory_Free(Framebuffer);
}

VkImageView GCRendererFramebuffer_GetTextureImageViewHandle(const GCRendererFramebuffer* const Framebuffer)
{
	return Framebuffer->TextureImageViewHandle;
}

VkSampler GCRendererFramebuffer_GetTextureSamplerHandle(const GCRendererFramebuffer* const Framebuffer)
{
	return Framebuffer->TextureImageSamplerHandle;
}

VkFramebuffer GCRendererFramebuffer_GetTextureFramebufferHandle(const GCRendererFramebuffer* const Framebuffer)
{
	return Framebuffer->TextureFramebufferHandle;
}

VkFramebuffer* GCRendererFramebuffer_GetSwapChainFramebufferHandles(const GCRendererFramebuffer* const Framebuffer)
{
	return Framebuffer->SwapChainFramebufferHandles;
}

VkExtent2D GCRendererFramebuffer_GetTextureExtent(const GCRendererFramebuffer* const Framebuffer)
{
	return Framebuffer->TextureExtent;
}

void GCRendererFramebuffer_CreateTexture(GCRendererFramebuffer* const Framebuffer, const bool IsResize)
{
	const VkFormat SwapChainFormat = GCRendererSwapChain_GetFormat(Framebuffer->SwapChain);
	const VkFormat SwapChainDepthFormat = GCRendererSwapChain_GetDepthFormat(Framebuffer->SwapChain);

	GCVulkanUtilities_CreateImage(Framebuffer->Device, Framebuffer->TextureExtent.width, Framebuffer->TextureExtent.height, 1, SwapChainFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &Framebuffer->TextureImageHandle, &Framebuffer->TextureImageMemoryHandle);
	GCVulkanUtilities_CreateImageView(Framebuffer->Device, Framebuffer->TextureImageHandle, SwapChainFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1, &Framebuffer->TextureImageViewHandle);

	if (!IsResize)
	{
		GCVulkanUtilities_CreateSampler(Framebuffer->Device, VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_REPEAT, 0, &Framebuffer->TextureImageSamplerHandle);
	}

	GCVulkanUtilities_CreateImage(Framebuffer->Device, Framebuffer->TextureExtent.width, Framebuffer->TextureExtent.height, 1, SwapChainDepthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &Framebuffer->TextureDepthImageHandle, &Framebuffer->TextureDepthImageMemoryHandle);
	GCVulkanUtilities_CreateImageView(Framebuffer->Device, Framebuffer->TextureDepthImageHandle, SwapChainDepthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, 1, &Framebuffer->TextureDepthImageViewHandle);
}

void GCRendererFramebuffer_CreateTextureFramebuffer(GCRendererFramebuffer* const Framebuffer)
{
	const VkDevice DeviceHandle = GCRendererDevice_GetDeviceHandle(Framebuffer->Device);
	const VkRenderPass RenderPassHandle = GCRendererGraphicsPipeline_GetTextureRenderPassHandle(Framebuffer->GraphicsPipeline);
	const VkImageView AttachmentHandles[2] =
	{
		Framebuffer->TextureImageViewHandle,
		Framebuffer->TextureDepthImageViewHandle
	};

	VkFramebufferCreateInfo FramebufferInformation = { 0 };
	FramebufferInformation.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	FramebufferInformation.renderPass = RenderPassHandle;
	FramebufferInformation.attachmentCount = 2;
	FramebufferInformation.pAttachments = AttachmentHandles;
	FramebufferInformation.width = Framebuffer->TextureExtent.width;
	FramebufferInformation.height = Framebuffer->TextureExtent.height;
	FramebufferInformation.layers = 1;

	GC_VULKAN_VALIDATE(vkCreateFramebuffer(DeviceHandle, &FramebufferInformation, NULL, &Framebuffer->TextureFramebufferHandle), "Failed to create a Vulkan texture framebuffer");
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
			SwapChainImageViewHandles[Counter],
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

void GCRendererFramebuffer_DestroyObjectsTexture(GCRendererFramebuffer* const Framebuffer, const bool IsResize)
{
	const VkDevice DeviceHandle = GCRendererDevice_GetDeviceHandle(Framebuffer->Device);

	vkDestroyFramebuffer(DeviceHandle, Framebuffer->TextureFramebufferHandle, NULL);

	vkDestroyImageView(DeviceHandle, Framebuffer->TextureDepthImageViewHandle, NULL);
	vkFreeMemory(DeviceHandle, Framebuffer->TextureDepthImageMemoryHandle, NULL);
	vkDestroyImage(DeviceHandle, Framebuffer->TextureDepthImageHandle, NULL);

	if (!IsResize)
	{
		vkDestroySampler(DeviceHandle, Framebuffer->TextureImageSamplerHandle, NULL);
	}

	vkDestroyImageView(DeviceHandle, Framebuffer->TextureImageViewHandle, NULL);
	vkFreeMemory(DeviceHandle, Framebuffer->TextureImageMemoryHandle, NULL);
	vkDestroyImage(DeviceHandle, Framebuffer->TextureImageHandle, NULL);
}

void GCRendererFramebuffer_DestroyObjectsSwapChain(GCRendererFramebuffer* const Framebuffer)
{
	const VkDevice DeviceHandle = GCRendererDevice_GetDeviceHandle(Framebuffer->Device);
	const uint32_t SwapChainImageCount = GCRendererSwapChain_GetImageCount(Framebuffer->SwapChain);

	for (uint32_t Counter = 0; Counter < SwapChainImageCount; Counter++)
	{
		vkDestroyFramebuffer(DeviceHandle, Framebuffer->SwapChainFramebufferHandles[Counter], NULL);
	}

	GCMemory_Free(Framebuffer->SwapChainFramebufferHandles);
}