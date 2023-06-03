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

#include "Renderer/RendererFramebuffer.h"
#include "Renderer/RendererDevice.h"
#include "Renderer/Vulkan/VulkanUtilities.h"
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

VkImageView GCRendererFramebuffer_GetTextureImageViewHandle(const GCRendererFramebuffer* const Framebuffer);
VkSampler GCRendererFramebuffer_GetTextureSamplerHandle(const GCRendererFramebuffer* const Framebuffer);
VkFramebuffer GCRendererFramebuffer_GetTextureFramebufferHandle(const GCRendererFramebuffer* const Framebuffer);
VkFramebuffer* GCRendererFramebuffer_GetSwapChainFramebufferHandles(const GCRendererFramebuffer* const Framebuffer);
VkExtent2D GCRendererFramebuffer_GetTextureExtent(const GCRendererFramebuffer* const Framebuffer);

extern VkDevice GCRendererDevice_GetDeviceHandle(const GCRendererDevice* const Device);
extern VkFormat GCRendererSwapChain_GetFormat(const GCRendererSwapChain* const SwapChain);
extern VkExtent2D GCRendererSwapChain_GetExtent(const GCRendererSwapChain* const SwapChain);
extern VkImageView* GCRendererSwapChain_GetImageViewHandles(const GCRendererSwapChain* const SwapChain);
extern VkImageView GCRendererSwapChain_GetDepthImageViewHandle(const GCRendererSwapChain* const SwapChain);
extern uint32_t GCRendererSwapChain_GetImageCount(const GCRendererSwapChain* const SwapChain);
extern VkFormat GCRendererSwapChain_GetDepthFormat(const GCRendererSwapChain* const SwapChain);
extern VkRenderPass GCRendererGraphicsPipeline_GetTextureRenderPassHandle(const GCRendererGraphicsPipeline* const GraphicsPipeline);
extern VkRenderPass GCRendererGraphicsPipeline_GetSwapChainRenderPassHandle(const GCRendererGraphicsPipeline* const GraphicsPipeline);

static void GCRendererFramebuffer_CreateTextureImages(GCRendererFramebuffer* const Framebuffer);
static void GCRendererFramebuffer_CreateTextureFramebuffer(GCRendererFramebuffer* const Framebuffer);
static void GCRendererFramebuffer_CreateSwapChainFramebuffers(GCRendererFramebuffer* const Framebuffer);
static void GCRendererFramebuffer_DestroyObjectsTexture(GCRendererFramebuffer* const Framebuffer);
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

	GCRendererFramebuffer_CreateTextureImages(Framebuffer);
	GCRendererFramebuffer_CreateTextureFramebuffer(Framebuffer);
	GCRendererFramebuffer_CreateSwapChainFramebuffers(Framebuffer);

	return Framebuffer;
}

void GCRendererFramebuffer_RecreateTexture(GCRendererFramebuffer* const Framebuffer, const uint32_t Width, const uint32_t Height)
{
	GCRendererDevice_WaitIdle(Framebuffer->Device);

	GCRendererFramebuffer_DestroyObjectsTexture(Framebuffer);

	Framebuffer->TextureExtent = (VkExtent2D){ Width, Height };

	GCRendererFramebuffer_CreateTextureImages(Framebuffer);
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
	GCRendererFramebuffer_DestroyObjectsTexture(Framebuffer);
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

void GCRendererFramebuffer_CreateTextureImages(GCRendererFramebuffer* const Framebuffer)
{
	const VkDevice DeviceHandle = GCRendererDevice_GetDeviceHandle(Framebuffer->Device);
	const VkFormat SwapChainFormat = GCRendererSwapChain_GetFormat(Framebuffer->SwapChain);
	const VkFormat SwapChainDepthFormat = GCRendererSwapChain_GetDepthFormat(Framebuffer->SwapChain);
	const GCRendererDeviceCapabilities DeviceCapabilities = GCRendererDevice_GetDeviceCapabilities(Framebuffer->Device);

	GCVulkanUtilities_CreateImage(Framebuffer->Device, Framebuffer->TextureExtent.width, Framebuffer->TextureExtent.height, SwapChainFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &Framebuffer->TextureImageHandle, &Framebuffer->TextureImageMemoryHandle);
	GCVulkanUtilities_CreateImageView(Framebuffer->Device, Framebuffer->TextureImageHandle, SwapChainFormat, VK_IMAGE_ASPECT_COLOR_BIT, &Framebuffer->TextureImageViewHandle);

	GCVulkanUtilities_CreateImage(Framebuffer->Device, Framebuffer->TextureExtent.width, Framebuffer->TextureExtent.height, SwapChainDepthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &Framebuffer->TextureDepthImageHandle, &Framebuffer->TextureDepthImageMemoryHandle);
	GCVulkanUtilities_CreateImageView(Framebuffer->Device, Framebuffer->TextureDepthImageHandle, SwapChainDepthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, &Framebuffer->TextureDepthImageViewHandle);

	VkSamplerCreateInfo SamplerInformation = { 0 };
	SamplerInformation.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	SamplerInformation.magFilter = VK_FILTER_LINEAR;
	SamplerInformation.minFilter = VK_FILTER_LINEAR;
	SamplerInformation.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	SamplerInformation.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	SamplerInformation.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	SamplerInformation.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
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
	SamplerInformation.maxLod = 0.0f;
	SamplerInformation.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	SamplerInformation.unnormalizedCoordinates = VK_FALSE;

	GC_VULKAN_VALIDATE(vkCreateSampler(DeviceHandle, &SamplerInformation, NULL, &Framebuffer->TextureImageSamplerHandle), "Failed to create a Vulkan texture sampler");
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
	const VkImageView SwapChainDepthImageViewHandle = GCRendererSwapChain_GetDepthImageViewHandle(Framebuffer->SwapChain);
	const VkRenderPass RenderPassHandle = GCRendererGraphicsPipeline_GetSwapChainRenderPassHandle(Framebuffer->GraphicsPipeline);

	const VkExtent2D SwapChainExtent = GCRendererSwapChain_GetExtent(Framebuffer->SwapChain);

	for (uint32_t Counter = 0; Counter < SwapChainImageCount; Counter++)
	{
		const VkImageView AttachmentHandles[2] =
		{
			SwapChainImageViewHandles[Counter],
			SwapChainDepthImageViewHandle
		};

		VkFramebufferCreateInfo FramebufferInformation = { 0 };
		FramebufferInformation.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		FramebufferInformation.renderPass = RenderPassHandle;
		FramebufferInformation.attachmentCount = 2;
		FramebufferInformation.pAttachments = AttachmentHandles;
		FramebufferInformation.width = SwapChainExtent.width;
		FramebufferInformation.height = SwapChainExtent.height;
		FramebufferInformation.layers = 1;

		GC_VULKAN_VALIDATE(vkCreateFramebuffer(DeviceHandle, &FramebufferInformation, NULL, &Framebuffer->SwapChainFramebufferHandles[Counter]), "Failed to create a Vulkan swap chain framebuffer");
	}
}

void GCRendererFramebuffer_DestroyObjectsTexture(GCRendererFramebuffer* const Framebuffer)
{
	const VkDevice DeviceHandle = GCRendererDevice_GetDeviceHandle(Framebuffer->Device);

	vkDestroyFramebuffer(DeviceHandle, Framebuffer->TextureFramebufferHandle, NULL);

	vkDestroySampler(DeviceHandle, Framebuffer->TextureImageSamplerHandle, NULL);

	vkDestroyImageView(DeviceHandle, Framebuffer->TextureDepthImageViewHandle, NULL);
	vkFreeMemory(DeviceHandle, Framebuffer->TextureDepthImageMemoryHandle, NULL);
	vkDestroyImage(DeviceHandle, Framebuffer->TextureDepthImageHandle, NULL);

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