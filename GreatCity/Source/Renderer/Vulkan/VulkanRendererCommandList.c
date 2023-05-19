#include "Renderer/Vulkan/VulkanUtilities.h"
#include "Renderer/RendererCommandList.h"
#include "Core/Memory/Allocator.h"
#include "Core/Log.h"
#include "Core/Assert.h"

#include <stdint.h>
#include <stdbool.h>

#include <vulkan/vulkan.h>

typedef struct GCRendererCommandList
{
	const GCRendererDevice* Device;
	const GCRendererSwapChain* SwapChain;
	const GCRendererGraphicsPipeline* GraphicsPipeline;
	const GCRendererFramebuffer* Framebuffer;

	VkCommandPool CommandPoolHandle;
	VkCommandBuffer CommandBufferHandle;
	VkSemaphore ImageAvailableSemaphoreHandle, RenderFinishedSemaphoreHandle;
	VkFence InFlightFenceHandle;
} GCRendererCommandList;

typedef struct GCRendererCommandListRecordData
{
	uint32_t SwapChainImageIndex;
} GCRendererCommandListRecordData;

extern VkDevice GCRendererDevice_GetDeviceHandle(const GCRendererDevice* const Device);
extern uint32_t GCRendererDevice_GetGraphicsFamilyQueueIndex(const GCRendererDevice* const Device);
extern VkQueue GCRendererDevice_GetGraphicsQueueHandle(const GCRendererDevice* const Device);
extern VkQueue GCRendererDevice_GetPresentQueueHandle(const GCRendererDevice* const Device);
extern VkExtent2D GCRendererSwapChain_GetExtent(const GCRendererSwapChain* const SwapChain);
extern VkSwapchainKHR GCRendererSwapChain_GetHandle(const GCRendererSwapChain* const SwapChain);
extern VkRenderPass GCRendererGraphicsPipeline_GetRenderPassHandle(const GCRendererGraphicsPipeline* const GraphicsPipeline);
extern VkPipeline GCRendererGraphicsPipeline_GetPipelineHandle(const GCRendererGraphicsPipeline* const GraphicsPipeline);
extern VkFramebuffer* GCRendererFramebuffer_GetFramebufferHandles(const GCRendererFramebuffer* const Framebuffer);

static void GCRendererCommandList_CreateCommandPool(GCRendererCommandList* const CommandList);
static void GCRendererCommandList_CreateCommandBuffer(GCRendererCommandList* const CommandList);
static void GCRendererCommandList_CreateSemaphores(GCRendererCommandList* const CommandList);
static void GCRendererCommandList_CreateFence(GCRendererCommandList* const CommandList);

GCRendererCommandList* GCRendererCommandList_Create(const GCRendererDevice* const Device, const GCRendererSwapChain* const SwapChain, const GCRendererGraphicsPipeline* const GraphicsPipeline, const GCRendererFramebuffer* const Framebuffer)
{
	GCRendererCommandList* CommandList = (GCRendererCommandList*)GCMemory_Allocate(sizeof(GCRendererCommandList));
	CommandList->Device = Device;
	CommandList->SwapChain = SwapChain;
	CommandList->GraphicsPipeline = GraphicsPipeline;
	CommandList->Framebuffer = Framebuffer;
	CommandList->CommandPoolHandle = VK_NULL_HANDLE;
	CommandList->CommandBufferHandle = VK_NULL_HANDLE;
	CommandList->ImageAvailableSemaphoreHandle = VK_NULL_HANDLE;
	CommandList->RenderFinishedSemaphoreHandle = VK_NULL_HANDLE;
	CommandList->InFlightFenceHandle = VK_NULL_HANDLE;

	GCRendererCommandList_CreateCommandPool(CommandList);
	GCRendererCommandList_CreateCommandBuffer(CommandList);
	GCRendererCommandList_CreateSemaphores(CommandList);
	GCRendererCommandList_CreateFence(CommandList);

	return CommandList;
}

void GCRendererCommandList_BeginRecord(const GCRendererCommandList* const CommandList)
{
	VkCommandBufferBeginInfo CommandBufferBeginInformation = { 0 };
	CommandBufferBeginInformation.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

	GC_VULKAN_VALIDATE(vkBeginCommandBuffer(CommandList->CommandBufferHandle, &CommandBufferBeginInformation), "Failed to begin a Vulkan command buffer");
}

void GCRendererCommandList_BeginRenderPass(const GCRendererCommandList* const CommandList, const GCRendererCommandListRecordData* RecordData, const float* const ClearColor)
{
	VkRenderPassBeginInfo RenderPassBeginInformation = { 0 };
	RenderPassBeginInformation.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	RenderPassBeginInformation.renderPass = GCRendererGraphicsPipeline_GetRenderPassHandle(CommandList->GraphicsPipeline);
	RenderPassBeginInformation.framebuffer = GCRendererFramebuffer_GetFramebufferHandles(CommandList->Framebuffer)[RecordData->SwapChainImageIndex];
	RenderPassBeginInformation.renderArea.offset = (VkOffset2D){ 0, 0 };
	RenderPassBeginInformation.renderArea.extent = GCRendererSwapChain_GetExtent(CommandList->SwapChain);

	VkClearValue ClearValue = *(VkClearValue*)&ClearColor;

	RenderPassBeginInformation.clearValueCount = 1;
	RenderPassBeginInformation.pClearValues = &ClearValue;

	vkCmdBeginRenderPass(CommandList->CommandBufferHandle, &RenderPassBeginInformation, VK_SUBPASS_CONTENTS_INLINE);
}

void GCRendererCommandList_BindGraphicsPipeline(const GCRendererCommandList* const CommandList)
{
	vkCmdBindPipeline(CommandList->CommandBufferHandle, VK_PIPELINE_BIND_POINT_GRAPHICS, GCRendererGraphicsPipeline_GetPipelineHandle(CommandList->GraphicsPipeline));
}

void GCRendererCommandList_SetViewport(const GCRendererCommandList* const CommandList)
{
	const VkExtent2D SwapChainExtent = GCRendererSwapChain_GetExtent(CommandList->SwapChain);

	VkViewport Viewport = { 0 };
	Viewport.x = 0.0f;
	Viewport.y = 0.0f;
	Viewport.width = (float)SwapChainExtent.width;
	Viewport.height = (float)SwapChainExtent.height;
	Viewport.minDepth = 0.0f;
	Viewport.maxDepth = 1.0f;

	vkCmdSetViewport(CommandList->CommandBufferHandle, 0, 1, &Viewport);

	VkRect2D Scissor = { 0 };
	Scissor.offset = (VkOffset2D){ 0, 0 };
	Scissor.extent = SwapChainExtent;

	vkCmdSetScissor(CommandList->CommandBufferHandle, 0, 1, &Scissor);
}

void GCRendererCommandList_Draw(const GCRendererCommandList* const CommandList, const uint32_t VertexCount, const uint32_t FirstVertex)
{
	vkCmdDraw(CommandList->CommandBufferHandle, VertexCount, 1, FirstVertex, 0);
}

void GCRendererCommandList_EndRenderPass(const GCRendererCommandList* const CommandList)
{
	vkCmdEndRenderPass(CommandList->CommandBufferHandle);
}

void GCRendererCommandList_EndRecord(const GCRendererCommandList* const CommandList)
{
	GC_VULKAN_VALIDATE(vkEndCommandBuffer(CommandList->CommandBufferHandle), "Failed to end a Vulkan command buffer");
}

void GCRendererCommandList_SubmitAndPresent(const GCRendererCommandList* const CommandList, const GCRendererCommandListRecordFunction RecordFunction)
{
	const VkDevice DeviceHandle = GCRendererDevice_GetDeviceHandle(CommandList->Device);

	vkWaitForFences(DeviceHandle, 1, &CommandList->InFlightFenceHandle, VK_TRUE, UINT64_MAX);
	vkResetFences(DeviceHandle, 1, &CommandList->InFlightFenceHandle);

	uint32_t SwapChainImageIndex = 0;
	vkAcquireNextImageKHR(DeviceHandle, GCRendererSwapChain_GetHandle(CommandList->SwapChain), UINT64_MAX, CommandList->ImageAvailableSemaphoreHandle, VK_NULL_HANDLE, &SwapChainImageIndex);

	vkResetCommandBuffer(CommandList->CommandBufferHandle, 0);

	GCRendererCommandListRecordData RecordData = { 0 };
	RecordData.SwapChainImageIndex = SwapChainImageIndex;

	if (RecordFunction)
	{
		RecordFunction(&RecordData);
	}

	VkSubmitInfo SubmitInformation = { 0 };
	SubmitInformation.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	const VkSemaphore WaitSemaphoreHandle[1] = { CommandList->ImageAvailableSemaphoreHandle };
	const VkPipelineStageFlags PipelineWaitStage[1] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	
	SubmitInformation.waitSemaphoreCount = 1;
	SubmitInformation.pWaitSemaphores = WaitSemaphoreHandle;
	SubmitInformation.pWaitDstStageMask = PipelineWaitStage;
	SubmitInformation.commandBufferCount = 1;
	SubmitInformation.pCommandBuffers = &CommandList->CommandBufferHandle;

	const VkSemaphore SignalSemaphoreHandle[1] = { CommandList->RenderFinishedSemaphoreHandle };

	SubmitInformation.signalSemaphoreCount = 1;
	SubmitInformation.pSignalSemaphores = SignalSemaphoreHandle;

	GC_VULKAN_VALIDATE(vkQueueSubmit(GCRendererDevice_GetGraphicsQueueHandle(CommandList->Device), 1, &SubmitInformation, CommandList->InFlightFenceHandle), "Failed to submit a Vulkan queue");

	VkPresentInfoKHR PresentInformation = { 0 };
	PresentInformation.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	PresentInformation.waitSemaphoreCount = 1;
	PresentInformation.pWaitSemaphores = SignalSemaphoreHandle;

	const VkSwapchainKHR SwapChainHandle[1] = { GCRendererSwapChain_GetHandle(CommandList->SwapChain) };

	PresentInformation.swapchainCount = 1;
	PresentInformation.pSwapchains = SwapChainHandle;
	PresentInformation.pImageIndices = &SwapChainImageIndex;

	vkQueuePresentKHR(GCRendererDevice_GetPresentQueueHandle(CommandList->Device), &PresentInformation);
}

void GCRendererCommandList_Destroy(GCRendererCommandList* CommandList)
{
	const VkDevice DeviceHandle = GCRendererDevice_GetDeviceHandle(CommandList->Device);

	vkDestroyFence(DeviceHandle, CommandList->InFlightFenceHandle, NULL);
	vkDestroySemaphore(DeviceHandle, CommandList->RenderFinishedSemaphoreHandle, NULL);
	vkDestroySemaphore(DeviceHandle, CommandList->ImageAvailableSemaphoreHandle, NULL);
	vkDestroyCommandPool(DeviceHandle, CommandList->CommandPoolHandle, NULL);

	GCMemory_Free(CommandList);
}

void GCRendererCommandList_CreateCommandPool(GCRendererCommandList* const CommandList)
{
	const VkDevice DeviceHandle = GCRendererDevice_GetDeviceHandle(CommandList->Device);

	VkCommandPoolCreateInfo CommandPoolInformation = { 0 };
	CommandPoolInformation.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	CommandPoolInformation.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	CommandPoolInformation.queueFamilyIndex = GCRendererDevice_GetGraphicsFamilyQueueIndex(CommandList->Device);

	GC_VULKAN_VALIDATE(vkCreateCommandPool(DeviceHandle, &CommandPoolInformation, NULL, &CommandList->CommandPoolHandle), "Failed to create a Vulkan command pool");
}

void GCRendererCommandList_CreateCommandBuffer(GCRendererCommandList* const CommandList)
{
	const VkDevice DeviceHandle = GCRendererDevice_GetDeviceHandle(CommandList->Device);

	VkCommandBufferAllocateInfo CommandBufferInformation = { 0 };
	CommandBufferInformation.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	CommandBufferInformation.commandPool = CommandList->CommandPoolHandle;
	CommandBufferInformation.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	CommandBufferInformation.commandBufferCount = 1;

	GC_VULKAN_VALIDATE(vkAllocateCommandBuffers(DeviceHandle, &CommandBufferInformation, &CommandList->CommandBufferHandle), "Failed to allocate a Vulkan command buffer");
}

void GCRendererCommandList_CreateSemaphores(GCRendererCommandList* const CommandList)
{
	VkSemaphoreCreateInfo SemaphoreInformation = { 0 };
	SemaphoreInformation.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	const VkDevice DeviceHandle = GCRendererDevice_GetDeviceHandle(CommandList->Device);

	GC_VULKAN_VALIDATE(vkCreateSemaphore(DeviceHandle, &SemaphoreInformation, NULL, &CommandList->ImageAvailableSemaphoreHandle), "Failed to create a Vulkan semaphore");
	GC_VULKAN_VALIDATE(vkCreateSemaphore(DeviceHandle, &SemaphoreInformation, NULL, &CommandList->RenderFinishedSemaphoreHandle), "Failed to create a Vulkan semaphore");
}

void GCRendererCommandList_CreateFence(GCRendererCommandList* const CommandList)
{
	VkFenceCreateInfo FenceInformation = { 0 };
	FenceInformation.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	FenceInformation.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	const VkDevice DeviceHandle = GCRendererDevice_GetDeviceHandle(CommandList->Device);

	GC_VULKAN_VALIDATE(vkCreateFence(DeviceHandle, &FenceInformation, NULL, &CommandList->InFlightFenceHandle), "Failed to create a Vulkan fence");
}