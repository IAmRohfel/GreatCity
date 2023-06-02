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
#include "Renderer/Vulkan/VulkanUtilities.h"
#include "Core/Memory/Allocator.h"
#include "Core/Log.h"
#include "Core/Assert.h"

#include <stdbool.h>

#include <vulkan/vulkan.h>

typedef struct GCRendererFramebuffer
{
	const GCRendererDevice* Device;
	const GCRendererSwapChain* SwapChain;
	const GCRendererGraphicsPipeline* GraphicsPipeline;

	VkFramebuffer* FramebufferHandles;
} GCRendererFramebuffer;

VkFramebuffer* GCRendererFramebuffer_GetFramebufferHandles(const GCRendererFramebuffer* const Framebuffer);

extern VkDevice GCRendererDevice_GetDeviceHandle(const GCRendererDevice* const Device);
extern VkExtent2D GCRendererSwapChain_GetExtent(const GCRendererSwapChain* const SwapChain);
extern VkImageView* GCRendererSwapChain_GetImageViewHandles(const GCRendererSwapChain* const SwapChain);
extern VkImageView GCRendererSwapChain_GetDepthImageViewHandle(const GCRendererSwapChain* const SwapChain);
extern uint32_t GCRendererSwapChain_GetImageCount(const GCRendererSwapChain* const SwapChain);
extern VkRenderPass GCRendererGraphicsPipeline_GetRenderPassHandle(const GCRendererGraphicsPipeline* const GraphicsPipeline);

static void GCRendererFramebuffer_CreateFramebuffers(GCRendererFramebuffer* const Framebuffer);
static void GCRendererFramebuffer_DestroyObjects(GCRendererFramebuffer* const Framebuffer);

GCRendererFramebuffer* GCRendererFramebuffer_Create(const GCRendererDevice* const Device, const GCRendererSwapChain* const SwapChain, const GCRendererGraphicsPipeline* const GraphicsPipeline)
{
	GCRendererFramebuffer* Framebuffer = (GCRendererFramebuffer*)GCMemory_Allocate(sizeof(GCRendererFramebuffer));
	Framebuffer->Device = Device;
	Framebuffer->SwapChain = SwapChain;
	Framebuffer->GraphicsPipeline = GraphicsPipeline;
	Framebuffer->FramebufferHandles = NULL;

	GCRendererFramebuffer_CreateFramebuffers(Framebuffer);

	return Framebuffer;
}

void GCRendererFramebuffer_Recreate(GCRendererFramebuffer* const Framebuffer)
{
	GCRendererFramebuffer_DestroyObjects(Framebuffer);

	GCRendererFramebuffer_CreateFramebuffers(Framebuffer);
}

void GCRendererFramebuffer_Destroy(GCRendererFramebuffer* Framebuffer)
{
	GCRendererFramebuffer_DestroyObjects(Framebuffer);

	GCMemory_Free(Framebuffer);
}

VkFramebuffer* GCRendererFramebuffer_GetFramebufferHandles(const GCRendererFramebuffer* const Framebuffer)
{
	return Framebuffer->FramebufferHandles;
}

void GCRendererFramebuffer_CreateFramebuffers(GCRendererFramebuffer* const Framebuffer)
{
	const uint32_t SwapChainImageCount = GCRendererSwapChain_GetImageCount(Framebuffer->SwapChain);
	Framebuffer->FramebufferHandles = (VkFramebuffer*)GCMemory_Allocate(SwapChainImageCount * sizeof(VkFramebuffer));

	const VkDevice DeviceHandle = GCRendererDevice_GetDeviceHandle(Framebuffer->Device);
	const VkExtent2D SwapChainExtent = GCRendererSwapChain_GetExtent(Framebuffer->SwapChain);
	const VkImageView* const SwapChainImageViewHandles = GCRendererSwapChain_GetImageViewHandles(Framebuffer->SwapChain);
	const VkImageView SwapChainDepthImageViewHandle = GCRendererSwapChain_GetDepthImageViewHandle(Framebuffer->SwapChain);
	const VkRenderPass RenderPassHandle = GCRendererGraphicsPipeline_GetRenderPassHandle(Framebuffer->GraphicsPipeline);

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

		GC_VULKAN_VALIDATE(vkCreateFramebuffer(DeviceHandle, &FramebufferInformation, NULL, &Framebuffer->FramebufferHandles[Counter]), "Failed to create a Vulkan swap chain framebuffer");
	}
}

void GCRendererFramebuffer_DestroyObjects(GCRendererFramebuffer* const Framebuffer)
{
	const VkDevice DeviceHandle = GCRendererDevice_GetDeviceHandle(Framebuffer->Device);
	const uint32_t SwapChainImageCount = GCRendererSwapChain_GetImageCount(Framebuffer->SwapChain);

	for (uint32_t Counter = 0; Counter < SwapChainImageCount; Counter++)
	{
		vkDestroyFramebuffer(DeviceHandle, Framebuffer->FramebufferHandles[Counter], NULL);
	}

	GCMemory_Free(Framebuffer->FramebufferHandles);
}