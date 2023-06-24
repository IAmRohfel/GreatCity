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

#include "Renderer/Vulkan/VulkanRendererVertexBuffer.h"
#include "Core/Assert.h"
#include "Core/Log.h"
#include "Core/Memory/Allocator.h"
#include "Renderer/RendererVertexBuffer.h"
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

typedef struct GCRendererVertexBuffer
{
    const GCRendererDevice* Device;
    const GCRendererCommandList* CommandList;

    VkBuffer BufferHandle;
    VmaAllocation BufferAllocationHandle;
    VmaAllocationInfo DynamicBufferAllocationInformation;

    void* Vertices;
    uint32_t VertexCount;
    size_t VertexSize;
} GCRendererVertexBuffer;

static void GCRendererVertexBuffer_CreateVertexBuffer(GCRendererVertexBuffer* const VertexBuffer);
static void GCRendererVertexBuffer_CreateVertexBufferDynamic(GCRendererVertexBuffer* const VertexBuffer);
static void GCRendererVertexBuffer_DestroyObjects(GCRendererVertexBuffer* const VertexBuffer);

GCRendererVertexBuffer* GCRendererVertexBuffer_Create(const GCRendererVertexBufferDescription* const Description)
{
    GCRendererVertexBuffer* VertexBuffer =
        (GCRendererVertexBuffer*)GCMemory_AllocateZero(sizeof(GCRendererVertexBuffer));
    VertexBuffer->Device = Description->Device;
    VertexBuffer->CommandList = Description->CommandList;
    VertexBuffer->Vertices = NULL;
    VertexBuffer->VertexCount = Description->VertexCount;
    VertexBuffer->VertexSize = Description->VertexSize;

    VertexBuffer->Vertices = (void*)GCMemory_AllocateZero(VertexBuffer->VertexSize);
    memcpy(VertexBuffer->Vertices, Description->Vertices, VertexBuffer->VertexSize);

    GCRendererVertexBuffer_CreateVertexBuffer(VertexBuffer);

    return VertexBuffer;
}

GCRendererVertexBuffer* GCRendererVertexBuffer_CreateDynamic(const GCRendererVertexBufferDescription* const Description)
{
    GCRendererVertexBuffer* VertexBuffer =
        (GCRendererVertexBuffer*)GCMemory_AllocateZero(sizeof(GCRendererVertexBuffer));
    VertexBuffer->Device = Description->Device;
    VertexBuffer->CommandList = Description->CommandList;
    VertexBuffer->Vertices = NULL;
    VertexBuffer->VertexCount = Description->VertexCount;
    VertexBuffer->VertexSize = Description->VertexSize;

    if (Description->Vertices)
    {
        VertexBuffer->Vertices = (void*)GCMemory_AllocateZero(VertexBuffer->VertexSize);
        memcpy(VertexBuffer->Vertices, Description->Vertices, VertexBuffer->VertexSize);
    }

    GCRendererVertexBuffer_CreateVertexBufferDynamic(VertexBuffer);

    if (VertexBuffer->Vertices)
    {
        GCRendererVertexBuffer_SetVertices(VertexBuffer, VertexBuffer->Vertices, VertexBuffer->VertexSize);
    }

    return VertexBuffer;
}

void GCRendererVertexBuffer_SetVertices(
    GCRendererVertexBuffer* const VertexBuffer, const void* const Vertices, const size_t VertexSize
)
{
    const VmaAllocator AllocatorHandle = GCRendererDevice_GetAllocatorHandle(VertexBuffer->Device);

    memcpy(VertexBuffer->DynamicBufferAllocationInformation.pMappedData, Vertices, VertexSize);
    vmaFlushAllocation(AllocatorHandle, VertexBuffer->BufferAllocationHandle, 0, VK_WHOLE_SIZE);
}

void* GCRendererVertexBuffer_GetVertices(const GCRendererVertexBuffer* const VertexBuffer)
{
    return VertexBuffer->Vertices;
}

uint32_t GCRendererVertexBuffer_GetVertexCount(const GCRendererVertexBuffer* const VertexBuffer)
{
    return VertexBuffer->VertexCount;
}

void GCRendererVertexBuffer_Destroy(GCRendererVertexBuffer* VertexBuffer)
{
    GCRendererVertexBuffer_DestroyObjects(VertexBuffer);

    GCMemory_Free(VertexBuffer->Vertices);
    GCMemory_Free(VertexBuffer);
}

VkBuffer GCRendererVertexBuffer_GetHandle(const GCRendererVertexBuffer* const VertexBuffer)
{
    return VertexBuffer->BufferHandle;
}

void GCRendererVertexBuffer_CreateVertexBuffer(GCRendererVertexBuffer* const VertexBuffer)
{
    const VmaAllocator AllocatorHandle = GCRendererDevice_GetAllocatorHandle(VertexBuffer->Device);

    VkBuffer StagingBufferHandle = VK_NULL_HANDLE;
    VmaAllocation StagingAllocationHandle = VK_NULL_HANDLE;
    VmaAllocationInfo StagingAllocationInformation = {0};

    GCVulkanUtilities_CreateBuffer(
        VertexBuffer->Device, VertexBuffer->VertexSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT, VMA_MEMORY_USAGE_AUTO, &StagingBufferHandle,
        &StagingAllocationHandle, &StagingAllocationInformation
    );

    memcpy(StagingAllocationInformation.pMappedData, VertexBuffer->Vertices, VertexBuffer->VertexSize);
    vmaFlushAllocation(AllocatorHandle, StagingAllocationHandle, 0, VK_WHOLE_SIZE);

    GCVulkanUtilities_CreateBuffer(
        VertexBuffer->Device, VertexBuffer->VertexSize,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, 0, VMA_MEMORY_USAGE_AUTO,
        &VertexBuffer->BufferHandle, &VertexBuffer->BufferAllocationHandle, NULL
    );

    const VkCommandBuffer CommandBufferHandle =
        GCRendererCommandList_BeginSingleTimeCommands(VertexBuffer->CommandList);
    GCVulkanUtilities_CopyBuffer(
        CommandBufferHandle, StagingBufferHandle, VertexBuffer->BufferHandle, VertexBuffer->VertexSize
    );
    GCRendererCommandList_EndSingleTimeCommands(VertexBuffer->CommandList, CommandBufferHandle);

    vmaDestroyBuffer(AllocatorHandle, VertexBuffer->BufferHandle, VertexBuffer->BufferAllocationHandle);
}

void GCRendererVertexBuffer_CreateVertexBufferDynamic(GCRendererVertexBuffer* const VertexBuffer)
{
    GCVulkanUtilities_CreateBuffer(
        VertexBuffer->Device, VertexBuffer->VertexSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VMA_ALLOCATION_CREATE_MAPPED_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
        VMA_MEMORY_USAGE_AUTO, &VertexBuffer->BufferHandle, &VertexBuffer->BufferAllocationHandle,
        &VertexBuffer->DynamicBufferAllocationInformation
    );
}

void GCRendererVertexBuffer_DestroyObjects(GCRendererVertexBuffer* const VertexBuffer)
{
    const VmaAllocator AllocatorHandle = GCRendererDevice_GetAllocatorHandle(VertexBuffer->Device);

    vmaDestroyBuffer(AllocatorHandle, VertexBuffer->BufferHandle, VertexBuffer->BufferAllocationHandle);
}