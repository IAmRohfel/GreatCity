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

#include "Renderer/Vulkan/VulkanRendererIndexBuffer.h"
#include "Core/Assert.h"
#include "Core/Log.h"
#include "Core/Memory/Allocator.h"
#include "Renderer/RendererIndexBuffer.h"
#include "Renderer/Vulkan/VulkanRendererCommandList.h"
#include "Renderer/Vulkan/VulkanRendererDevice.h"
#include "Renderer/Vulkan/VulkanUtilities.h"

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include <vulkan/vulkan.h>
#ifndef VMA_VULKAN_VERSION
#define VMA_VULKAN_VERSION 1001000
#endif
#include <vk_mem_alloc.h>

typedef struct GCRendererIndexBuffer
{
    const GCRendererDevice* Device;
    const GCRendererCommandList* CommandList;

    VkBuffer BufferHandle;
    VmaAllocation BufferAllocationHandle;

    uint32_t* Indices;
    uint32_t IndexCount;
    size_t IndexSize;
} GCRendererIndexBuffer;

static void GCRendererIndexBuffer_CreateIndexBuffer(GCRendererIndexBuffer* const IndexBuffer);
static void GCRendererIndexBuffer_DestroyObjects(GCRendererIndexBuffer* const IndexBuffer);

GCRendererIndexBuffer* GCRendererIndexBuffer_Create(const GCRendererIndexBufferDescription* const Description)
{
    GCRendererIndexBuffer* IndexBuffer = (GCRendererIndexBuffer*)GCMemory_AllocateZero(sizeof(GCRendererIndexBuffer));
    IndexBuffer->Device = Description->Device;
    IndexBuffer->CommandList = Description->CommandList;
    IndexBuffer->Indices = Description->Indices;
    IndexBuffer->IndexCount = Description->IndexCount;
    IndexBuffer->IndexSize = Description->IndexSize;

    GCRendererIndexBuffer_CreateIndexBuffer(IndexBuffer);

    return IndexBuffer;
}

uint32_t GCRendererIndexBuffer_GetIndexCount(const GCRendererIndexBuffer* const indexBuffer)
{
    return indexBuffer->IndexCount;
}

void GCRendererIndexBuffer_Destroy(GCRendererIndexBuffer* IndexBuffer)
{
    GCRendererIndexBuffer_DestroyObjects(IndexBuffer);

    GCMemory_Free(IndexBuffer);
}

VkBuffer GCRendererIndexBuffer_GetHandle(const GCRendererIndexBuffer* const IndexBuffer)
{
    return IndexBuffer->BufferHandle;
}

void GCRendererIndexBuffer_CreateIndexBuffer(GCRendererIndexBuffer* const IndexBuffer)
{
    const VmaAllocator AllocatorHandle = GCRendererDevice_GetAllocatorHandle(IndexBuffer->Device);

    VkBuffer StagingBufferHandle = VK_NULL_HANDLE;
    VmaAllocation StagingAllocationHandle = VK_NULL_HANDLE;
    VmaAllocationInfo StagingAllocationInformation = {0};

    GCVulkanUtilities_CreateBuffer(
        IndexBuffer->Device, IndexBuffer->IndexSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VMA_ALLOCATION_CREATE_MAPPED_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
        VMA_MEMORY_USAGE_AUTO, &StagingBufferHandle, &StagingAllocationHandle, &StagingAllocationInformation
    );

    memcpy(StagingAllocationInformation.pMappedData, IndexBuffer->Indices, IndexBuffer->IndexSize);
    vmaFlushAllocation(AllocatorHandle, StagingAllocationHandle, 0, VK_WHOLE_SIZE);

    GCVulkanUtilities_CreateBuffer(
        IndexBuffer->Device, IndexBuffer->IndexSize,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, 0, VMA_MEMORY_USAGE_AUTO,
        &IndexBuffer->BufferHandle, &IndexBuffer->BufferAllocationHandle, NULL
    );

    const VkCommandBuffer CommandBufferHandle = GCRendererCommandList_BeginSingleTimeCommands(IndexBuffer->CommandList);
    GCVulkanUtilities_CopyBuffer(
        CommandBufferHandle, StagingBufferHandle, IndexBuffer->BufferHandle, IndexBuffer->IndexSize
    );
    GCRendererCommandList_EndSingleTimeCommands(IndexBuffer->CommandList, CommandBufferHandle);

    vmaDestroyBuffer(AllocatorHandle, StagingBufferHandle, StagingAllocationHandle);
}

void GCRendererIndexBuffer_DestroyObjects(GCRendererIndexBuffer* const IndexBuffer)
{
    const VmaAllocator AllocatorHandle = GCRendererDevice_GetAllocatorHandle(IndexBuffer->Device);

    vmaDestroyBuffer(AllocatorHandle, IndexBuffer->BufferHandle, IndexBuffer->BufferAllocationHandle);
}