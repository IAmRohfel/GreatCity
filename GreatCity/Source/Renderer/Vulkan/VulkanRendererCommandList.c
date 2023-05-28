#include "Renderer/RendererCommandList.h"
#include "Renderer/Vulkan/VulkanUtilities.h"
#include "Core/Memory/Allocator.h"
#include "Core/Log.h"
#include "Core/Assert.h"

#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include <vulkan/vulkan.h>

typedef struct GCRendererCommandList
{
	const GCRendererDevice* Device;

	VkCommandPool CommandPoolHandle, TransientCommandPoolHandle;
	VkCommandBuffer* CommandBufferHandles;
	VkSemaphore* ImageAvailableSemaphoreHandles;
	VkSemaphore* RenderFinishedSemaphoreHandles;
	VkFence* InFlightFenceHandles;

	GCRendererCommandListResizeCallbackFunction ResizeCallbackFunction;
	bool IsResized;
	uint32_t MaximumFramesInFlight;
	uint32_t CurrentFrame;
} GCRendererCommandList;

typedef struct GCRendererCommandListRecordData
{
	uint32_t SwapChainImageIndex;
} GCRendererCommandListRecordData;

uint32_t GCRendererCommandList_GetMaximumFramesInFlight(const GCRendererCommandList* const CommandList);
VkCommandPool GCRendererCommandList_GetTransientCommandPoolHandle(const GCRendererCommandList* const CommandList);

extern VkDevice GCRendererDevice_GetDeviceHandle(const GCRendererDevice* const Device);
extern uint32_t GCRendererDevice_GetGraphicsFamilyQueueIndex(const GCRendererDevice* const Device);
extern VkQueue GCRendererDevice_GetGraphicsQueueHandle(const GCRendererDevice* const Device);
extern VkQueue GCRendererDevice_GetPresentQueueHandle(const GCRendererDevice* const Device);
extern VkExtent2D GCRendererSwapChain_GetExtent(const GCRendererSwapChain* const SwapChain);
extern VkSwapchainKHR GCRendererSwapChain_GetHandle(const GCRendererSwapChain* const SwapChain);
extern VkBuffer GCRendererVertexBuffer_GetHandle(const GCRendererVertexBuffer* const VertexBuffer);
extern VkBuffer GCRendererIndexBuffer_GetHandle(const GCRendererIndexBuffer* const IndexBuffer);
extern VkDescriptorSet* GCRendererGraphicsPipeline_GetDescriptorSetHandles(const GCRendererGraphicsPipeline* const GraphicsPipeline);
extern void** GCRendererUniformBuffer_GetData(const GCRendererUniformBuffer* const UniformBuffer);
extern VkRenderPass GCRendererGraphicsPipeline_GetRenderPassHandle(const GCRendererGraphicsPipeline* const GraphicsPipeline);
extern VkPipelineLayout GCRendererGraphicsPipeline_GetPipelineLayoutHandle(const GCRendererGraphicsPipeline* const GraphicsPipeline);
extern VkPipeline GCRendererGraphicsPipeline_GetPipelineHandle(const GCRendererGraphicsPipeline* const GraphicsPipeline);
extern VkFramebuffer* GCRendererFramebuffer_GetFramebufferHandles(const GCRendererFramebuffer* const Framebuffer);

static void GCRendererCommandList_CreateCommandPool(GCRendererCommandList* const CommandList);
static void GCRendererCommandList_CreateCommandBuffers(GCRendererCommandList* const CommandList);
static void GCRendererCommandList_CreateSemaphores(GCRendererCommandList* const CommandList);
static void GCRendererCommandList_CreateFences(GCRendererCommandList* const CommandList);
static void GCRendererCommandList_DestroyObjects(GCRendererCommandList* const CommandList);

GCRendererCommandList* GCRendererCommandList_Create(const GCRendererDevice* const Device)
{
	GCRendererCommandList* CommandList = (GCRendererCommandList*)GCMemory_Allocate(sizeof(GCRendererCommandList));
	CommandList->Device = Device;
	CommandList->CommandPoolHandle = VK_NULL_HANDLE;
	CommandList->TransientCommandPoolHandle = VK_NULL_HANDLE;
	CommandList->CommandBufferHandles = NULL;
	CommandList->ImageAvailableSemaphoreHandles = NULL;
	CommandList->RenderFinishedSemaphoreHandles = NULL;
	CommandList->InFlightFenceHandles = NULL;
	CommandList->ResizeCallbackFunction = NULL;
	CommandList->MaximumFramesInFlight = 2;
	CommandList->CurrentFrame = 0;
	CommandList->IsResized = false;

	GCRendererCommandList_CreateCommandPool(CommandList);
	GCRendererCommandList_CreateCommandBuffers(CommandList);
	GCRendererCommandList_CreateSemaphores(CommandList);
	GCRendererCommandList_CreateFences(CommandList);

	return CommandList;
}

void GCRendererCommandList_SetResize(GCRendererCommandList* const CommandList, const bool IsResized)
{
	CommandList->IsResized = IsResized;
}

void GCRendererCommandList_SetResizeCallback(GCRendererCommandList* const CommandList, const GCRendererCommandListResizeCallbackFunction ResizeCallbackFunction)
{
	CommandList->ResizeCallbackFunction = ResizeCallbackFunction;
}

void GCRendererCommandList_BeginRecord(const GCRendererCommandList* const CommandList)
{
	VkCommandBufferBeginInfo CommandBufferBeginInformation = { 0 };
	CommandBufferBeginInformation.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

	GC_VULKAN_VALIDATE(vkBeginCommandBuffer(CommandList->CommandBufferHandles[CommandList->CurrentFrame], &CommandBufferBeginInformation), "Failed to begin a Vulkan command buffer");
}

void GCRendererCommandList_BeginRenderPass(const GCRendererCommandList* const CommandList, const GCRendererSwapChain* const SwapChain, const GCRendererGraphicsPipeline* const GraphicsPipeline, const GCRendererFramebuffer* const Framebuffer, const GCRendererCommandListRecordData* RecordData, const float* const ClearColor)
{
	VkRenderPassBeginInfo RenderPassBeginInformation = { 0 };
	RenderPassBeginInformation.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	RenderPassBeginInformation.renderPass = GCRendererGraphicsPipeline_GetRenderPassHandle(GraphicsPipeline);
	RenderPassBeginInformation.framebuffer = GCRendererFramebuffer_GetFramebufferHandles(Framebuffer)[RecordData->SwapChainImageIndex];
	RenderPassBeginInformation.renderArea.offset = (VkOffset2D){ 0, 0 };
	RenderPassBeginInformation.renderArea.extent = GCRendererSwapChain_GetExtent(SwapChain);

	VkClearValue ClearValues[2] = { 0 };
	memcpy(ClearValues[0].color.float32, ClearColor, sizeof(ClearValues[0].color.float32));

	ClearValues[1].depthStencil.depth = 1.0f;
	ClearValues[1].depthStencil.stencil = 0;

	RenderPassBeginInformation.clearValueCount = 2;
	RenderPassBeginInformation.pClearValues = ClearValues;

	vkCmdBeginRenderPass(CommandList->CommandBufferHandles[CommandList->CurrentFrame], &RenderPassBeginInformation, VK_SUBPASS_CONTENTS_INLINE);
}

void GCRendererCommandList_BindVertexBuffer(const GCRendererCommandList* const CommandList, const GCRendererVertexBuffer* const VertexBuffer)
{
	const VkBuffer VertexBufferHandle[1] = { GCRendererVertexBuffer_GetHandle(VertexBuffer) };
	const VkDeviceSize Offsets[1] = { 0 };

	vkCmdBindVertexBuffers(CommandList->CommandBufferHandles[CommandList->CurrentFrame], 0, 1, VertexBufferHandle, Offsets);
}

void GCRendererCommandList_BindIndexBuffer(const GCRendererCommandList* const CommandList, const GCRendererIndexBuffer* const IndexBuffer)
{
	vkCmdBindIndexBuffer(CommandList->CommandBufferHandles[CommandList->CurrentFrame], GCRendererIndexBuffer_GetHandle(IndexBuffer), 0, VK_INDEX_TYPE_UINT32);
}

void GCRendererCommandList_UpdateUniformBuffer(const GCRendererCommandList* const CommandList, const GCRendererUniformBuffer* const UniformBuffer, const GCRendererCommandListRecordData* RecordData, const void* const Data, const size_t DataSize)
{
	(void)CommandList;

	void** UniformBufferData = GCRendererUniformBuffer_GetData(UniformBuffer);

	memcpy(UniformBufferData[RecordData->SwapChainImageIndex], Data, DataSize);
}

void GCRendererCommandList_BindGraphicsPipeline(const GCRendererCommandList* const CommandList, const GCRendererGraphicsPipeline* const GraphicsPipeline)
{
	vkCmdBindPipeline(CommandList->CommandBufferHandles[CommandList->CurrentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, GCRendererGraphicsPipeline_GetPipelineHandle(GraphicsPipeline));

	const VkDescriptorSet DescriptorSetHandle = GCRendererGraphicsPipeline_GetDescriptorSetHandles(GraphicsPipeline)[CommandList->CurrentFrame];
	vkCmdBindDescriptorSets(CommandList->CommandBufferHandles[CommandList->CurrentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, GCRendererGraphicsPipeline_GetPipelineLayoutHandle(GraphicsPipeline), 0, 1, &DescriptorSetHandle, 0, NULL);
}

void GCRendererCommandList_SetViewport(const GCRendererCommandList* const CommandList, const GCRendererSwapChain* const SwapChain)
{
	const VkExtent2D SwapChainExtent = GCRendererSwapChain_GetExtent(SwapChain);

	VkViewport Viewport = { 0 };
	Viewport.x = 0.0f;
	Viewport.y = 0.0f;
	Viewport.width = (float)SwapChainExtent.width;
	Viewport.height = (float)SwapChainExtent.height;
	Viewport.minDepth = 0.0f;
	Viewport.maxDepth = 1.0f;

	vkCmdSetViewport(CommandList->CommandBufferHandles[CommandList->CurrentFrame], 0, 1, &Viewport);

	VkRect2D Scissor = { 0 };
	Scissor.offset = (VkOffset2D){ 0, 0 };
	Scissor.extent = SwapChainExtent;

	vkCmdSetScissor(CommandList->CommandBufferHandles[CommandList->CurrentFrame], 0, 1, &Scissor);
}

void GCRendererCommandList_Draw(const GCRendererCommandList* const CommandList, const uint32_t VertexCount, const uint32_t FirstVertex)
{
	vkCmdDraw(CommandList->CommandBufferHandles[CommandList->CurrentFrame], VertexCount, 1, FirstVertex, 0);
}

void GCRendererCommandList_DrawIndexed(const GCRendererCommandList* const CommandList, const uint32_t IndexCount, const uint32_t FirstIndex)
{
	vkCmdDrawIndexed(CommandList->CommandBufferHandles[CommandList->CurrentFrame], IndexCount, 1, FirstIndex, 0, 0);
}

void GCRendererCommandList_EndRenderPass(const GCRendererCommandList* const CommandList)
{
	vkCmdEndRenderPass(CommandList->CommandBufferHandles[CommandList->CurrentFrame]);
}

void GCRendererCommandList_EndRecord(const GCRendererCommandList* const CommandList)
{
	GC_VULKAN_VALIDATE(vkEndCommandBuffer(CommandList->CommandBufferHandles[CommandList->CurrentFrame]), "Failed to end a Vulkan command buffer");
}

void GCRendererCommandList_SubmitAndPresent(GCRendererCommandList* const CommandList, const GCRendererSwapChain* const SwapChain, const GCRendererCommandListRecordFunction RecordFunction)
{
	const VkDevice DeviceHandle = GCRendererDevice_GetDeviceHandle(CommandList->Device);
	const VkSwapchainKHR SwapChainHandle[1] = { GCRendererSwapChain_GetHandle(SwapChain) };

	vkWaitForFences(DeviceHandle, 1, &CommandList->InFlightFenceHandles[CommandList->CurrentFrame], VK_TRUE, UINT64_MAX);

	uint32_t SwapChainImageIndex = 0;
	VkResult SwapChainCheckResult = vkAcquireNextImageKHR(DeviceHandle, SwapChainHandle[0], UINT64_MAX, CommandList->ImageAvailableSemaphoreHandles[CommandList->CurrentFrame], VK_NULL_HANDLE, &SwapChainImageIndex);

	if (SwapChainCheckResult == VK_ERROR_OUT_OF_DATE_KHR || SwapChainCheckResult == VK_SUBOPTIMAL_KHR)
	{
		CommandList->ResizeCallbackFunction();

		return;
	}

	vkResetFences(DeviceHandle, 1, &CommandList->InFlightFenceHandles[CommandList->CurrentFrame]);
	vkResetCommandBuffer(CommandList->CommandBufferHandles[CommandList->CurrentFrame], 0);

	GCRendererCommandListRecordData RecordData = { 0 };
	RecordData.SwapChainImageIndex = SwapChainImageIndex;

	if (RecordFunction)
	{
		RecordFunction(&RecordData);
	}

	VkSubmitInfo SubmitInformation = { 0 };
	SubmitInformation.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	const VkSemaphore WaitSemaphoreHandle[1] = { CommandList->ImageAvailableSemaphoreHandles[CommandList->CurrentFrame] };
	const VkPipelineStageFlags PipelineWaitStage[1] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	
	SubmitInformation.waitSemaphoreCount = 1;
	SubmitInformation.pWaitSemaphores = WaitSemaphoreHandle;
	SubmitInformation.pWaitDstStageMask = PipelineWaitStage;
	SubmitInformation.commandBufferCount = 1;
	SubmitInformation.pCommandBuffers = &CommandList->CommandBufferHandles[CommandList->CurrentFrame];

	const VkSemaphore SignalSemaphoreHandle[1] = { CommandList->RenderFinishedSemaphoreHandles[CommandList->CurrentFrame] };

	SubmitInformation.signalSemaphoreCount = 1;
	SubmitInformation.pSignalSemaphores = SignalSemaphoreHandle;

	GC_VULKAN_VALIDATE(vkQueueSubmit(GCRendererDevice_GetGraphicsQueueHandle(CommandList->Device), 1, &SubmitInformation, CommandList->InFlightFenceHandles[CommandList->CurrentFrame]), "Failed to submit a Vulkan queue");

	VkPresentInfoKHR PresentInformation = { 0 };
	PresentInformation.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	PresentInformation.waitSemaphoreCount = 1;
	PresentInformation.pWaitSemaphores = SignalSemaphoreHandle;
	PresentInformation.swapchainCount = 1;
	PresentInformation.pSwapchains = SwapChainHandle;
	PresentInformation.pImageIndices = &SwapChainImageIndex;

	SwapChainCheckResult = vkQueuePresentKHR(GCRendererDevice_GetPresentQueueHandle(CommandList->Device), &PresentInformation);

	if (SwapChainCheckResult == VK_ERROR_OUT_OF_DATE_KHR || SwapChainCheckResult == VK_SUBOPTIMAL_KHR || CommandList->IsResized)
	{
		CommandList->IsResized = false;
		CommandList->ResizeCallbackFunction();

		return;
	}

	CommandList->CurrentFrame = (CommandList->CurrentFrame + 1) % CommandList->MaximumFramesInFlight;
}

void GCRendererCommandList_Destroy(GCRendererCommandList* CommandList)
{
	GCRendererCommandList_DestroyObjects(CommandList);

	GCMemory_Free(CommandList);
}

uint32_t GCRendererCommandList_GetMaximumFramesInFlight(const GCRendererCommandList* const CommandList)
{
	return CommandList->MaximumFramesInFlight;
}

VkCommandPool GCRendererCommandList_GetTransientCommandPoolHandle(const GCRendererCommandList* const CommandList)
{
	return CommandList->TransientCommandPoolHandle;
}

void GCRendererCommandList_CreateCommandPool(GCRendererCommandList* const CommandList)
{
	const VkDevice DeviceHandle = GCRendererDevice_GetDeviceHandle(CommandList->Device);

	VkCommandPoolCreateInfo CommandPoolInformation = { 0 };
	CommandPoolInformation.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	CommandPoolInformation.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	CommandPoolInformation.queueFamilyIndex = GCRendererDevice_GetGraphicsFamilyQueueIndex(CommandList->Device);

	GC_VULKAN_VALIDATE(vkCreateCommandPool(DeviceHandle, &CommandPoolInformation, NULL, &CommandList->CommandPoolHandle), "Failed to create a Vulkan command pool");

	VkCommandPoolCreateInfo TransientCommandPoolInformation = { 0 };
	TransientCommandPoolInformation.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	TransientCommandPoolInformation.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
	TransientCommandPoolInformation.queueFamilyIndex = GCRendererDevice_GetGraphicsFamilyQueueIndex(CommandList->Device);

	GC_VULKAN_VALIDATE(vkCreateCommandPool(DeviceHandle, &TransientCommandPoolInformation, NULL, &CommandList->TransientCommandPoolHandle), "Failed to create a Vulkan transient command pool");
}

void GCRendererCommandList_CreateCommandBuffers(GCRendererCommandList* const CommandList)
{
	const VkDevice DeviceHandle = GCRendererDevice_GetDeviceHandle(CommandList->Device);

	CommandList->CommandBufferHandles = (VkCommandBuffer*)GCMemory_Allocate(CommandList->MaximumFramesInFlight * sizeof(VkCommandBuffer));

	VkCommandBufferAllocateInfo CommandBufferAllocateInformation = { 0 };
	CommandBufferAllocateInformation.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	CommandBufferAllocateInformation.commandPool = CommandList->CommandPoolHandle;
	CommandBufferAllocateInformation.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	CommandBufferAllocateInformation.commandBufferCount = CommandList->MaximumFramesInFlight;

	GC_VULKAN_VALIDATE(vkAllocateCommandBuffers(DeviceHandle, &CommandBufferAllocateInformation, CommandList->CommandBufferHandles), "Failed to allocate Vulkan command buffers");
}

void GCRendererCommandList_CreateSemaphores(GCRendererCommandList* const CommandList)
{
	const VkDevice DeviceHandle = GCRendererDevice_GetDeviceHandle(CommandList->Device);

	CommandList->ImageAvailableSemaphoreHandles = (VkSemaphore*)GCMemory_Allocate(CommandList->MaximumFramesInFlight * sizeof(VkSemaphore));
	CommandList->RenderFinishedSemaphoreHandles = (VkSemaphore*)GCMemory_Allocate(CommandList->MaximumFramesInFlight * sizeof(VkSemaphore));

	VkSemaphoreCreateInfo SemaphoreInformation = { 0 };
	SemaphoreInformation.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	for (uint32_t Counter = 0; Counter < CommandList->MaximumFramesInFlight; Counter++)
	{
		GC_VULKAN_VALIDATE(vkCreateSemaphore(DeviceHandle, &SemaphoreInformation, NULL, &CommandList->ImageAvailableSemaphoreHandles[Counter]), "Failed to create a Vulkan semaphore");
		GC_VULKAN_VALIDATE(vkCreateSemaphore(DeviceHandle, &SemaphoreInformation, NULL, &CommandList->RenderFinishedSemaphoreHandles[Counter]), "Failed to create a Vulkan semaphore");
	}
}

void GCRendererCommandList_CreateFences(GCRendererCommandList* const CommandList)
{
	const VkDevice DeviceHandle = GCRendererDevice_GetDeviceHandle(CommandList->Device);

	CommandList->InFlightFenceHandles = (VkFence*)GCMemory_Allocate(CommandList->MaximumFramesInFlight * sizeof(VkFence));

	VkFenceCreateInfo FenceInformation = { 0 };
	FenceInformation.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	FenceInformation.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	for (uint32_t Counter = 0; Counter < CommandList->MaximumFramesInFlight; Counter++)
	{
		GC_VULKAN_VALIDATE(vkCreateFence(DeviceHandle, &FenceInformation, NULL, &CommandList->InFlightFenceHandles[Counter]), "Failed to create a Vulkan fence");
	}
}

void GCRendererCommandList_DestroyObjects(GCRendererCommandList* const CommandList)
{
	const VkDevice DeviceHandle = GCRendererDevice_GetDeviceHandle(CommandList->Device);

	for (uint32_t Counter = 0; Counter < CommandList->MaximumFramesInFlight; Counter++)
	{
		vkDestroyFence(DeviceHandle, CommandList->InFlightFenceHandles[Counter], NULL);
		vkDestroySemaphore(DeviceHandle, CommandList->RenderFinishedSemaphoreHandles[Counter], NULL);
		vkDestroySemaphore(DeviceHandle, CommandList->ImageAvailableSemaphoreHandles[Counter], NULL);
	}

	vkDestroyCommandPool(DeviceHandle, CommandList->TransientCommandPoolHandle, NULL);
	vkDestroyCommandPool(DeviceHandle, CommandList->CommandPoolHandle, NULL);

	GCMemory_Free(CommandList->InFlightFenceHandles);
	GCMemory_Free(CommandList->RenderFinishedSemaphoreHandles);
	GCMemory_Free(CommandList->ImageAvailableSemaphoreHandles);
	GCMemory_Free(CommandList->CommandBufferHandles);
}