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

#include "Renderer/Vulkan/VulkanRendererCommandList.h"
#include "Core/Assert.h"
#include "Core/Log.h"
#include "Core/Memory/Allocator.h"
#include "Renderer/RendererCommandList.h"
#include "Renderer/RendererDevice.h"
#include "Renderer/Vulkan/VulkanRendererDevice.h"
#include "Renderer/Vulkan/VulkanRendererFramebuffer.h"
#include "Renderer/Vulkan/VulkanRendererGraphicsPipeline.h"
#include "Renderer/Vulkan/VulkanRendererIndexBuffer.h"
#include "Renderer/Vulkan/VulkanRendererSwapChain.h"
#include "Renderer/Vulkan/VulkanRendererUniformBuffer.h"
#include "Renderer/Vulkan/VulkanRendererVertexBuffer.h"
#include "Renderer/Vulkan/VulkanUtilities.h"

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include <vulkan/vulkan.h>

typedef struct GCRendererCommandList
{
    const GCRendererDevice* Device;
    const GCRendererSwapChain* SwapChain;

    VkCommandPool CommandPoolHandle, TransientCommandPoolHandle;
    VkCommandBuffer* CommandBufferHandles;
    VkSemaphore* ImageAvailableSemaphoreHandles;
    VkSemaphore* RenderFinishedSemaphoreHandles;
    VkFence* InFlightFenceHandles;

    GCRendererCommandListResizeCallbackFunction SwapChainResizeCallbackFunction;
    GCRendererCommandListResizeCallbackFunction AttachmentResizeCallbackFunction;
    bool ShouldSwapChainResize;
    bool ShouldAttachmentResize;
    uint32_t MaximumFramesInFlight;
    uint32_t CurrentFrame, CurrentImageIndex;
} GCRendererCommandList;

static void GCRendererCommandList_CreateCommandPool(GCRendererCommandList* const CommandList);
static void GCRendererCommandList_CreateCommandBuffers(GCRendererCommandList* const CommandList);
static void GCRendererCommandList_CreateSemaphores(GCRendererCommandList* const CommandList);
static void GCRendererCommandList_CreateFences(GCRendererCommandList* const CommandList);
static void GCRendererCommandList_DestroyObjects(GCRendererCommandList* const CommandList);

GCRendererCommandList* GCRendererCommandList_Create(const GCRendererCommandListDescription* const Description)
{
    GCRendererCommandList* CommandList = (GCRendererCommandList*)GCMemory_Allocate(sizeof(GCRendererCommandList));
    CommandList->Device = Description->Device;
    CommandList->SwapChain = Description->SwapChain;
    CommandList->CommandPoolHandle = VK_NULL_HANDLE;
    CommandList->TransientCommandPoolHandle = VK_NULL_HANDLE;
    CommandList->CommandBufferHandles = NULL;
    CommandList->ImageAvailableSemaphoreHandles = NULL;
    CommandList->RenderFinishedSemaphoreHandles = NULL;
    CommandList->InFlightFenceHandles = NULL;
    CommandList->SwapChainResizeCallbackFunction = NULL;
    CommandList->AttachmentResizeCallbackFunction = NULL;
    CommandList->ShouldSwapChainResize = false;
    CommandList->ShouldAttachmentResize = false;
    CommandList->MaximumFramesInFlight = 2;
    CommandList->CurrentFrame = 0;
    CommandList->CurrentImageIndex = 0;

    GCRendererCommandList_CreateCommandPool(CommandList);
    GCRendererCommandList_CreateCommandBuffers(CommandList);
    GCRendererCommandList_CreateSemaphores(CommandList);
    GCRendererCommandList_CreateFences(CommandList);

    return CommandList;
}

void GCRendererCommandList_ShouldSwapChainResize(GCRendererCommandList* const CommandList, const bool ShouldResize)
{
    CommandList->ShouldSwapChainResize = ShouldResize;
}

void GCRendererCommandList_ShouldAttachmentResize(GCRendererCommandList* const CommandList, const bool ShouldResize)
{
    CommandList->ShouldAttachmentResize = ShouldResize;
}

void GCRendererCommandList_SetSwapChainResizeCallback(
    GCRendererCommandList* const CommandList, const GCRendererCommandListResizeCallbackFunction ResizeCallbackFunction)
{
    CommandList->SwapChainResizeCallbackFunction = ResizeCallbackFunction;
}

void GCRendererCommandList_SetAttachmentResizeCallback(
    GCRendererCommandList* const CommandList, const GCRendererCommandListResizeCallbackFunction ResizeCallbackFunction)
{
    CommandList->AttachmentResizeCallbackFunction = ResizeCallbackFunction;
}

void GCRendererCommandList_BeginRecord(GCRendererCommandList* const CommandList)
{
    if (CommandList->ShouldAttachmentResize)
    {
        CommandList->AttachmentResizeCallbackFunction();
        CommandList->ShouldAttachmentResize = false;
    }

    const VkDevice DeviceHandle = GCRendererDevice_GetDeviceHandle(CommandList->Device);
    const VkSwapchainKHR SwapChainHandle[1] = {GCRendererSwapChain_GetHandle(CommandList->SwapChain)};

    vkWaitForFences(DeviceHandle, 1, &CommandList->InFlightFenceHandles[CommandList->CurrentFrame], VK_TRUE,
                    UINT64_MAX);
    VkResult SwapChainCheckResult =
        vkAcquireNextImageKHR(DeviceHandle, SwapChainHandle[0], UINT64_MAX,
                              CommandList->ImageAvailableSemaphoreHandles[CommandList->CurrentFrame], VK_NULL_HANDLE,
                              &CommandList->CurrentImageIndex);

    if (SwapChainCheckResult == VK_ERROR_OUT_OF_DATE_KHR || SwapChainCheckResult == VK_SUBOPTIMAL_KHR)
    {
        CommandList->SwapChainResizeCallbackFunction();

        return;
    }

    vkResetFences(DeviceHandle, 1, &CommandList->InFlightFenceHandles[CommandList->CurrentFrame]);
    vkResetCommandBuffer(CommandList->CommandBufferHandles[CommandList->CurrentFrame], 0);

    VkCommandBufferBeginInfo CommandBufferBeginInformation = {0};
    CommandBufferBeginInformation.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    GC_VULKAN_VALIDATE(vkBeginCommandBuffer(CommandList->CommandBufferHandles[CommandList->CurrentFrame],
                                            &CommandBufferBeginInformation),
                       "Failed to begin a Vulkan command buffer");
}

void GCRendererCommandList_BeginSwapChainRenderPass(const GCRendererCommandList* const CommandList,
                                                    const GCRendererGraphicsPipeline* const GraphicsPipeline,
                                                    const GCRendererFramebuffer* const Framebuffer,
                                                    const float* const ClearColor)
{
    VkRenderPassBeginInfo RenderPassBeginInformation = {0};
    RenderPassBeginInformation.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    RenderPassBeginInformation.renderPass = GCRendererGraphicsPipeline_GetSwapChainRenderPassHandle(GraphicsPipeline);
    RenderPassBeginInformation.framebuffer =
        GCRendererFramebuffer_GetSwapChainFramebufferHandles(Framebuffer)[CommandList->CurrentImageIndex];
    RenderPassBeginInformation.renderArea.offset = (VkOffset2D){0, 0};
    RenderPassBeginInformation.renderArea.extent = GCRendererSwapChain_GetExtent(CommandList->SwapChain);

    VkClearValue ClearValue = {0};
    memcpy(ClearValue.color.float32, ClearColor, sizeof(ClearValue.color.float32));

    RenderPassBeginInformation.clearValueCount = 1;
    RenderPassBeginInformation.pClearValues = &ClearValue;

    vkCmdBeginRenderPass(CommandList->CommandBufferHandles[CommandList->CurrentFrame], &RenderPassBeginInformation,
                         VK_SUBPASS_CONTENTS_INLINE);
}

void GCRendererCommandList_BeginAttachmentRenderPass(const GCRendererCommandList* const CommandList,
                                                     const GCRendererGraphicsPipeline* const GraphicsPipeline,
                                                     const GCRendererFramebuffer* const Framebuffer,
                                                     const float* const ClearColor)
{
    VkRenderPassBeginInfo RenderPassBeginInformation = {0};
    RenderPassBeginInformation.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    RenderPassBeginInformation.renderPass = GCRendererGraphicsPipeline_GetAttachmentRenderPassHandle(GraphicsPipeline);
    RenderPassBeginInformation.framebuffer = GCRendererFramebuffer_GetAttachmentFramebufferHandle(Framebuffer);
    RenderPassBeginInformation.renderArea.offset = (VkOffset2D){0, 0};
    RenderPassBeginInformation.renderArea.extent = GCRendererFramebuffer_GetFramebufferSize(Framebuffer);

    VkClearValue ClearValues[3] = {0};
    memcpy(ClearValues[0].color.float32, ClearColor, sizeof(ClearValues[0].color.float32));

    const int32_t ClearColorInt[4] = {-1, -1, -1, -1};
    memcpy(ClearValues[1].color.int32, ClearColorInt, sizeof(ClearValues[1].color.int32));

    ClearValues[2].depthStencil.depth = 1.0f;
    ClearValues[2].depthStencil.stencil = 0;

    RenderPassBeginInformation.clearValueCount = 3;
    RenderPassBeginInformation.pClearValues = ClearValues;

    vkCmdBeginRenderPass(CommandList->CommandBufferHandles[CommandList->CurrentFrame], &RenderPassBeginInformation,
                         VK_SUBPASS_CONTENTS_INLINE);
}

void GCRendererCommandList_BindVertexBuffer(const GCRendererCommandList* const CommandList,
                                            const GCRendererVertexBuffer* const VertexBuffer)
{
    const VkBuffer VertexBufferHandle[1] = {GCRendererVertexBuffer_GetHandle(VertexBuffer)};
    const VkDeviceSize Offsets[1] = {0};

    vkCmdBindVertexBuffers(CommandList->CommandBufferHandles[CommandList->CurrentFrame], 0, 1, VertexBufferHandle,
                           Offsets);
}

void GCRendererCommandList_BindIndexBuffer(const GCRendererCommandList* const CommandList,
                                           const GCRendererIndexBuffer* const IndexBuffer)
{
    vkCmdBindIndexBuffer(CommandList->CommandBufferHandles[CommandList->CurrentFrame],
                         GCRendererIndexBuffer_GetHandle(IndexBuffer), 0, VK_INDEX_TYPE_UINT32);
}

void GCRendererCommandList_BindGraphicsPipeline(const GCRendererCommandList* const CommandList,
                                                const GCRendererGraphicsPipeline* const GraphicsPipeline)
{
    vkCmdBindPipeline(CommandList->CommandBufferHandles[CommandList->CurrentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS,
                      GCRendererGraphicsPipeline_GetPipelineHandle(GraphicsPipeline));

    const VkDescriptorSet DescriptorSetHandle = GCRendererGraphicsPipeline_GetDescriptorSetHandle(GraphicsPipeline);
    vkCmdBindDescriptorSets(
        CommandList->CommandBufferHandles[CommandList->CurrentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS,
        GCRendererGraphicsPipeline_GetPipelineLayoutHandle(GraphicsPipeline), 0, 1, &DescriptorSetHandle, 0, NULL);
}

void GCRendererCommandList_SetViewport(const GCRendererCommandList* const CommandList,
                                       const GCRendererFramebuffer* const Framebuffer)
{
    const VkExtent2D TextureExtent = GCRendererFramebuffer_GetFramebufferSize(Framebuffer);

    VkViewport Viewport = {0};
    Viewport.x = 0.0f;
    Viewport.y = 0.0f;
    Viewport.width = (float)TextureExtent.width;
    Viewport.height = (float)TextureExtent.height;
    Viewport.minDepth = 0.0f;
    Viewport.maxDepth = 1.0f;

    vkCmdSetViewport(CommandList->CommandBufferHandles[CommandList->CurrentFrame], 0, 1, &Viewport);

    VkRect2D Scissor = {0};
    Scissor.offset = (VkOffset2D){0, 0};
    Scissor.extent = TextureExtent;

    vkCmdSetScissor(CommandList->CommandBufferHandles[CommandList->CurrentFrame], 0, 1, &Scissor);
}

void GCRendererCommandList_Draw(const GCRendererCommandList* const CommandList, const uint32_t VertexCount,
                                const uint32_t FirstVertex)
{
    vkCmdDraw(CommandList->CommandBufferHandles[CommandList->CurrentFrame], VertexCount, 1, FirstVertex, 0);
}

void GCRendererCommandList_DrawIndexed(const GCRendererCommandList* const CommandList, const uint32_t IndexCount,
                                       const uint32_t FirstIndex)
{
    vkCmdDrawIndexed(CommandList->CommandBufferHandles[CommandList->CurrentFrame], IndexCount, 1, FirstIndex, 0, 0);
}

void GCRendererCommandList_EndSwapChainRenderPass(const GCRendererCommandList* const CommandList)
{
    vkCmdEndRenderPass(CommandList->CommandBufferHandles[CommandList->CurrentFrame]);
}

void GCRendererCommandList_EndAttachmentRenderPass(const GCRendererCommandList* const CommandList,
                                                   const GCRendererFramebuffer* const Framebuffer)
{
    vkCmdEndRenderPass(CommandList->CommandBufferHandles[CommandList->CurrentFrame]);

    GCVulkanUtilities_TransitionImageLayout(CommandList->CommandBufferHandles[CommandList->CurrentFrame],
                                            GCRendererFramebuffer_GetColorAttachmentImageHandle(Framebuffer, 1), 1,
                                            VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                            VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
}

void GCRendererCommandList_EndRecord(const GCRendererCommandList* const CommandList)
{
    GC_VULKAN_VALIDATE(vkEndCommandBuffer(CommandList->CommandBufferHandles[CommandList->CurrentFrame]),
                       "Failed to end a Vulkan command buffer");
}

void GCRendererCommandList_SubmitAndPresent(GCRendererCommandList* const CommandList)
{
    const VkSwapchainKHR SwapChainHandle[1] = {GCRendererSwapChain_GetHandle(CommandList->SwapChain)};

    VkSubmitInfo SubmitInformation = {0};
    SubmitInformation.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    const VkSemaphore WaitSemaphoreHandle[1] = {CommandList->ImageAvailableSemaphoreHandles[CommandList->CurrentFrame]};
    const VkPipelineStageFlags PipelineWaitStage[1] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

    SubmitInformation.waitSemaphoreCount = 1;
    SubmitInformation.pWaitSemaphores = WaitSemaphoreHandle;
    SubmitInformation.pWaitDstStageMask = PipelineWaitStage;
    SubmitInformation.commandBufferCount = 1;
    SubmitInformation.pCommandBuffers = &CommandList->CommandBufferHandles[CommandList->CurrentFrame];

    const VkSemaphore SignalSemaphoreHandle[1] = {
        CommandList->RenderFinishedSemaphoreHandles[CommandList->CurrentFrame]};

    SubmitInformation.signalSemaphoreCount = 1;
    SubmitInformation.pSignalSemaphores = SignalSemaphoreHandle;

    GC_VULKAN_VALIDATE(vkQueueSubmit(GCRendererDevice_GetGraphicsQueueHandle(CommandList->Device), 1,
                                     &SubmitInformation, CommandList->InFlightFenceHandles[CommandList->CurrentFrame]),
                       "Failed to submit a Vulkan queue");

    VkPresentInfoKHR PresentInformation = {0};
    PresentInformation.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    PresentInformation.waitSemaphoreCount = 1;
    PresentInformation.pWaitSemaphores = SignalSemaphoreHandle;
    PresentInformation.swapchainCount = 1;
    PresentInformation.pSwapchains = SwapChainHandle;
    PresentInformation.pImageIndices = &CommandList->CurrentImageIndex;

    VkResult PresentCheckResult =
        vkQueuePresentKHR(GCRendererDevice_GetPresentQueueHandle(CommandList->Device), &PresentInformation);

    if (PresentCheckResult == VK_ERROR_OUT_OF_DATE_KHR || PresentCheckResult == VK_SUBOPTIMAL_KHR ||
        CommandList->ShouldSwapChainResize)
    {
        CommandList->SwapChainResizeCallbackFunction();
        CommandList->ShouldSwapChainResize = false;

        return;
    }

    CommandList->CurrentFrame = (CommandList->CurrentFrame + 1) % CommandList->MaximumFramesInFlight;
}

void GCRendererCommandList_Destroy(GCRendererCommandList* CommandList)
{
    GCRendererDevice_WaitIdle(CommandList->Device);

    GCRendererCommandList_DestroyObjects(CommandList);

    GCMemory_Free(CommandList);
}

VkCommandBuffer GCRendererCommandList_BeginSingleTimeCommands(const GCRendererCommandList* const CommandList)
{
    const VkDevice DeviceHandle = GCRendererDevice_GetDeviceHandle(CommandList->Device);

    VkCommandBufferAllocateInfo CommandBufferAllocateInformation = {0};
    CommandBufferAllocateInformation.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    CommandBufferAllocateInformation.commandPool = CommandList->TransientCommandPoolHandle;
    CommandBufferAllocateInformation.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    CommandBufferAllocateInformation.commandBufferCount = 1;

    VkCommandBuffer CommandBufferHandle = VK_NULL_HANDLE;
    GC_VULKAN_VALIDATE(vkAllocateCommandBuffers(DeviceHandle, &CommandBufferAllocateInformation, &CommandBufferHandle),
                       "Failed to allocate a Vulkan command buffer");

    VkCommandBufferBeginInfo CommandBufferBeginInformation = {0};
    CommandBufferBeginInformation.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    CommandBufferBeginInformation.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    GC_VULKAN_VALIDATE(vkBeginCommandBuffer(CommandBufferHandle, &CommandBufferBeginInformation),
                       "Failed to begin a Vulkan command buffer");

    return CommandBufferHandle;
}

void GCRendererCommandList_EndSingleTimeCommands(const GCRendererCommandList* const CommandList,
                                                 const VkCommandBuffer CommandBufferHandle)
{
    const VkDevice DeviceHandle = GCRendererDevice_GetDeviceHandle(CommandList->Device);
    const VkCommandPool TransientCommandPoolHandle = CommandList->TransientCommandPoolHandle;
    const VkQueue GraphicsQueueHandle = GCRendererDevice_GetGraphicsQueueHandle(CommandList->Device);

    GC_VULKAN_VALIDATE(vkEndCommandBuffer(CommandBufferHandle), "Failed to end a Vulkan command buffer");

    VkSubmitInfo SubmitInformation = {0};
    SubmitInformation.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    SubmitInformation.commandBufferCount = 1;
    SubmitInformation.pCommandBuffers = &CommandBufferHandle;

    vkQueueSubmit(GraphicsQueueHandle, 1, &SubmitInformation, VK_NULL_HANDLE);
    vkQueueWaitIdle(GraphicsQueueHandle);

    vkFreeCommandBuffers(DeviceHandle, TransientCommandPoolHandle, 1, &CommandBufferHandle);
}

VkCommandBuffer GCRendererCommandList_GetCurrentFrameCommandBufferHandle(const GCRendererCommandList* const CommandList)
{
    return CommandList->CommandBufferHandles[CommandList->CurrentFrame];
}

void GCRendererCommandList_CreateCommandPool(GCRendererCommandList* const CommandList)
{
    const VkDevice DeviceHandle = GCRendererDevice_GetDeviceHandle(CommandList->Device);

    VkCommandPoolCreateInfo CommandPoolInformation = {0};
    CommandPoolInformation.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    CommandPoolInformation.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    CommandPoolInformation.queueFamilyIndex = GCRendererDevice_GetGraphicsFamilyQueueIndex(CommandList->Device);

    GC_VULKAN_VALIDATE(
        vkCreateCommandPool(DeviceHandle, &CommandPoolInformation, NULL, &CommandList->CommandPoolHandle),
        "Failed to create a Vulkan command pool");

    VkCommandPoolCreateInfo TransientCommandPoolInformation = {0};
    TransientCommandPoolInformation.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    TransientCommandPoolInformation.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
    TransientCommandPoolInformation.queueFamilyIndex =
        GCRendererDevice_GetGraphicsFamilyQueueIndex(CommandList->Device);

    GC_VULKAN_VALIDATE(vkCreateCommandPool(DeviceHandle, &TransientCommandPoolInformation, NULL,
                                           &CommandList->TransientCommandPoolHandle),
                       "Failed to create a Vulkan transient command pool");
}

void GCRendererCommandList_CreateCommandBuffers(GCRendererCommandList* const CommandList)
{
    const VkDevice DeviceHandle = GCRendererDevice_GetDeviceHandle(CommandList->Device);

    CommandList->CommandBufferHandles =
        (VkCommandBuffer*)GCMemory_Allocate(CommandList->MaximumFramesInFlight * sizeof(VkCommandBuffer));

    VkCommandBufferAllocateInfo CommandBufferAllocateInformation = {0};
    CommandBufferAllocateInformation.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    CommandBufferAllocateInformation.commandPool = CommandList->CommandPoolHandle;
    CommandBufferAllocateInformation.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    CommandBufferAllocateInformation.commandBufferCount = CommandList->MaximumFramesInFlight;

    GC_VULKAN_VALIDATE(
        vkAllocateCommandBuffers(DeviceHandle, &CommandBufferAllocateInformation, CommandList->CommandBufferHandles),
        "Failed to allocate Vulkan command buffers");
}

void GCRendererCommandList_CreateSemaphores(GCRendererCommandList* const CommandList)
{
    const VkDevice DeviceHandle = GCRendererDevice_GetDeviceHandle(CommandList->Device);

    CommandList->ImageAvailableSemaphoreHandles =
        (VkSemaphore*)GCMemory_Allocate(CommandList->MaximumFramesInFlight * sizeof(VkSemaphore));
    CommandList->RenderFinishedSemaphoreHandles =
        (VkSemaphore*)GCMemory_Allocate(CommandList->MaximumFramesInFlight * sizeof(VkSemaphore));

    VkSemaphoreCreateInfo SemaphoreInformation = {0};
    SemaphoreInformation.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    for (uint32_t Counter = 0; Counter < CommandList->MaximumFramesInFlight; Counter++)
    {
        GC_VULKAN_VALIDATE(vkCreateSemaphore(DeviceHandle, &SemaphoreInformation, NULL,
                                             &CommandList->ImageAvailableSemaphoreHandles[Counter]),
                           "Failed to create a Vulkan semaphore");
        GC_VULKAN_VALIDATE(vkCreateSemaphore(DeviceHandle, &SemaphoreInformation, NULL,
                                             &CommandList->RenderFinishedSemaphoreHandles[Counter]),
                           "Failed to create a Vulkan semaphore");
    }
}

void GCRendererCommandList_CreateFences(GCRendererCommandList* const CommandList)
{
    const VkDevice DeviceHandle = GCRendererDevice_GetDeviceHandle(CommandList->Device);

    CommandList->InFlightFenceHandles =
        (VkFence*)GCMemory_Allocate(CommandList->MaximumFramesInFlight * sizeof(VkFence));

    VkFenceCreateInfo FenceInformation = {0};
    FenceInformation.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    FenceInformation.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (uint32_t Counter = 0; Counter < CommandList->MaximumFramesInFlight; Counter++)
    {
        GC_VULKAN_VALIDATE(
            vkCreateFence(DeviceHandle, &FenceInformation, NULL, &CommandList->InFlightFenceHandles[Counter]),
            "Failed to create a Vulkan fence");
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