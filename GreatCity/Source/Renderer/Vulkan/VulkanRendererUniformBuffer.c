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

#include "Renderer/Vulkan/VulkanRendererUniformBuffer.h"
#include "Core/Assert.h"
#include "Core/Log.h"
#include "Core/Memory/Allocator.h"
#include "Renderer/RendererDevice.h"
#include "Renderer/RendererUniformBuffer.h"
#include "Renderer/Vulkan/VulkanRendererCommandList.h"
#include "Renderer/Vulkan/VulkanRendererDevice.h"
#include "Renderer/Vulkan/VulkanUtilities.h"

#include <stdbool.h>
#include <string.h>

#include <vulkan/vulkan.h>
#ifndef VMA_VULKAN_VERSION
#define VMA_VULKAN_VERSION 1000000
#endif
#include <vk_mem_alloc.h>

typedef struct GCRendererUniformBuffer
{
    const GCRendererDevice* Device;
    const GCRendererCommandList* CommandList;

    VkBuffer BufferHandle;
    VmaAllocation BufferAllocationHandle;
    VmaAllocationInfo BufferAllocationInformation;

    size_t DataSize;
} GCRendererUniformBuffer;

static void GCRendererUniformBuffer_CreateUniformBuffer(GCRendererUniformBuffer* const UniformBuffer);
static void GCRendererUniformBuffer_DestroyObjects(GCRendererUniformBuffer* const UniformBuffer);

GCRendererUniformBuffer* GCRendererUniformBuffer_Create(const GCRendererUniformBufferDescription* const Description)
{
    GCRendererUniformBuffer* UniformBuffer =
        (GCRendererUniformBuffer*)GCMemory_AllocateZero(sizeof(GCRendererUniformBuffer));
    UniformBuffer->Device = Description->Device;
    UniformBuffer->CommandList = Description->CommandList;
    UniformBuffer->DataSize = Description->DataSize;

    GCRendererUniformBuffer_CreateUniformBuffer(UniformBuffer);

    return UniformBuffer;
}

void GCRendererUniformBuffer_UpdateUniformBuffer(const GCRendererUniformBuffer* const UniformBuffer,
                                                 const void* const Data, const size_t DataSize)
{
    const VmaAllocator AllocatorHandle = GCRendererDevice_GetAllocatorHandle(UniformBuffer->Device);

    memcpy(UniformBuffer->BufferAllocationInformation.pMappedData, Data, DataSize);
    vmaFlushAllocation(AllocatorHandle, UniformBuffer->BufferAllocationHandle, 0, VK_WHOLE_SIZE);
}

void GCRendererUniformBuffer_Destroy(GCRendererUniformBuffer* UniformBuffer)
{
    GCRendererDevice_WaitIdle(UniformBuffer->Device);

    GCRendererUniformBuffer_DestroyObjects(UniformBuffer);

    GCMemory_Free(UniformBuffer);
}

VkBuffer GCRendererUniformBuffer_GetBufferHandle(const GCRendererUniformBuffer* const UniformBuffer)
{
    return UniformBuffer->BufferHandle;
}

size_t GCRendererUniformBuffer_GetDataSize(const GCRendererUniformBuffer* const UniformBuffer)
{
    return UniformBuffer->DataSize;
}

void GCRendererUniformBuffer_CreateUniformBuffer(GCRendererUniformBuffer* const UniformBuffer)
{
    GCVulkanUtilities_CreateBuffer(UniformBuffer->Device, UniformBuffer->DataSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                   VMA_ALLOCATION_CREATE_MAPPED_BIT |
                                       VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
                                   VMA_MEMORY_USAGE_AUTO, &UniformBuffer->BufferHandle,
                                   &UniformBuffer->BufferAllocationHandle, &UniformBuffer->BufferAllocationInformation);
}

void GCRendererUniformBuffer_DestroyObjects(GCRendererUniformBuffer* const UniformBuffer)
{
    const VmaAllocator AllocatorHandle = GCRendererDevice_GetAllocatorHandle(UniformBuffer->Device);

    vmaDestroyBuffer(AllocatorHandle, UniformBuffer->BufferHandle, UniformBuffer->BufferAllocationHandle);
}