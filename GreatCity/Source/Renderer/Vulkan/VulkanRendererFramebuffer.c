#include "Renderer/Vulkan/VulkanUtilities.h"
#include "Renderer/RendererFramebuffer.h"
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
extern uint32_t GCRendererSwapChain_GetImageCount(const GCRendererSwapChain* const SwapChain);
extern VkRenderPass GCRendererGraphicsPipeline_GetRenderPassHandle(const GCRendererGraphicsPipeline* const GraphicsPipeline);

static void GCRendererFramebuffer_CreateFramebuffers(GCRendererFramebuffer* const Framebuffer);

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

void GCRendererFramebuffer_Destroy(GCRendererFramebuffer* Framebuffer)
{
	const VkDevice DeviceHandle = GCRendererDevice_GetDeviceHandle(Framebuffer->Device);
	const uint32_t SwapChainImageCount = GCRendererSwapChain_GetImageCount(Framebuffer->SwapChain);

	for (uint32_t Counter = 0; Counter < SwapChainImageCount; Counter++)
	{
		vkDestroyFramebuffer(DeviceHandle, Framebuffer->FramebufferHandles[Counter], NULL);
	}

	GCMemory_Free(Framebuffer->FramebufferHandles);
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
	const VkRenderPass RenderPassHandle = GCRendererGraphicsPipeline_GetRenderPassHandle(Framebuffer->GraphicsPipeline);

	for (uint32_t Counter = 0; Counter < SwapChainImageCount; Counter++)
	{
		const VkImageView AttachmentHandle[1] =
		{
			SwapChainImageViewHandles[Counter]
		};

		VkFramebufferCreateInfo FramebufferInformation = { 0 };
		FramebufferInformation.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		FramebufferInformation.renderPass = RenderPassHandle;
		FramebufferInformation.attachmentCount = 1;
		FramebufferInformation.pAttachments = AttachmentHandle;
		FramebufferInformation.width = SwapChainExtent.width;
		FramebufferInformation.height = SwapChainExtent.height;
		FramebufferInformation.layers = 1;

		GC_VULKAN_VALIDATE(vkCreateFramebuffer(DeviceHandle, &FramebufferInformation, NULL, &Framebuffer->FramebufferHandles[Counter]), "Failed to create a Vulkan swap chain framebuffer");
	}
}