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

typedef struct GCRendererIndexBuffer
{
    const GCRendererDevice* Device;
    const GCRendererCommandList* CommandList;

    VkBuffer IndexBufferHandle;
    VkDeviceMemory IndexBufferMemoryHandle;

    uint32_t* Indices;
    uint32_t IndexCount;
    size_t IndexSize;
} GCRendererIndexBuffer;

static void GCRendererIndexBuffer_CreateIndexBuffer(GCRendererIndexBuffer* const IndexBuffer);
static void GCRendererIndexBuffer_DestroyObjects(GCRendererIndexBuffer* const IndexBuffer);

GCRendererIndexBuffer* GCRendererIndexBuffer_Create(const GCRendererIndexBufferDescription* const Description)
{
    GCRendererIndexBuffer* IndexBuffer = (GCRendererIndexBuffer*)GCMemory_Allocate(sizeof(GCRendererIndexBuffer));
    IndexBuffer->Device = Description->Device;
    IndexBuffer->CommandList = Description->CommandList;
    IndexBuffer->IndexBufferHandle = VK_NULL_HANDLE;
    IndexBuffer->IndexBufferMemoryHandle = VK_NULL_HANDLE;
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
    return IndexBuffer->IndexBufferHandle;
}

void GCRendererIndexBuffer_CreateIndexBuffer(GCRendererIndexBuffer* const IndexBuffer)
{
    const VkDevice DeviceHandle = GCRendererDevice_GetDeviceHandle(IndexBuffer->Device);

    VkBuffer StagingIndexBufferHandle = VK_NULL_HANDLE;
    VkDeviceMemory StagingIndexBufferMemoryHandle = VK_NULL_HANDLE;

    GCVulkanUtilities_CreateBuffer(IndexBuffer->Device, IndexBuffer->IndexSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                   &StagingIndexBufferHandle, &StagingIndexBufferMemoryHandle);

    void* IndexData = NULL;
    vkMapMemory(DeviceHandle, StagingIndexBufferMemoryHandle, 0, IndexBuffer->IndexSize, 0, &IndexData);
    memcpy(IndexData, IndexBuffer->Indices, IndexBuffer->IndexSize);
    vkUnmapMemory(DeviceHandle, StagingIndexBufferMemoryHandle);

    GCVulkanUtilities_CreateBuffer(IndexBuffer->Device, IndexBuffer->IndexSize,
                                   VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                                   VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &IndexBuffer->IndexBufferHandle,
                                   &IndexBuffer->IndexBufferMemoryHandle);

    const VkCommandBuffer CommandBufferHandle = GCRendererCommandList_BeginSingleTimeCommands(IndexBuffer->CommandList);
    GCVulkanUtilities_CopyBuffer(CommandBufferHandle, StagingIndexBufferHandle, IndexBuffer->IndexBufferHandle,
                                 IndexBuffer->IndexSize);
    GCRendererCommandList_EndSingleTimeCommands(IndexBuffer->CommandList, CommandBufferHandle);

    vkFreeMemory(DeviceHandle, StagingIndexBufferMemoryHandle, NULL);
    vkDestroyBuffer(DeviceHandle, StagingIndexBufferHandle, NULL);
}

void GCRendererIndexBuffer_DestroyObjects(GCRendererIndexBuffer* const IndexBuffer)
{
    const VkDevice DeviceHandle = GCRendererDevice_GetDeviceHandle(IndexBuffer->Device);

    vkFreeMemory(DeviceHandle, IndexBuffer->IndexBufferMemoryHandle, NULL);
    vkDestroyBuffer(DeviceHandle, IndexBuffer->IndexBufferHandle, NULL);
}