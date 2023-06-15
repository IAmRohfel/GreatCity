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

typedef struct GCRendererVertexBuffer
{
    const GCRendererDevice* Device;
    const GCRendererCommandList* CommandList;

    VkBuffer VertexBufferHandle;
    VkDeviceMemory VertexBufferMemoryHandle;

    void* Vertices;
    void* DynamicVertices;
    uint32_t VertexCount;
    size_t VertexSize;
} GCRendererVertexBuffer;

static void GCRendererVertexBuffer_CreateVertexBuffer(GCRendererVertexBuffer* const VertexBuffer);
static void GCRendererVertexBuffer_CreateVertexBufferDynamic(GCRendererVertexBuffer* const VertexBuffer);
static void GCRendererVertexBuffer_DestroyObjects(GCRendererVertexBuffer* const VertexBuffer);

GCRendererVertexBuffer* GCRendererVertexBuffer_Create(const GCRendererVertexBufferDescription* const Description)
{
    GCRendererVertexBuffer* VertexBuffer = (GCRendererVertexBuffer*)GCMemory_Allocate(sizeof(GCRendererVertexBuffer));
    VertexBuffer->Device = Description->Device;
    VertexBuffer->CommandList = Description->CommandList;
    VertexBuffer->VertexBufferHandle = VK_NULL_HANDLE;
    VertexBuffer->VertexBufferMemoryHandle = VK_NULL_HANDLE;
    VertexBuffer->Vertices = NULL;
    VertexBuffer->VertexCount = Description->VertexCount;
    VertexBuffer->VertexSize = Description->VertexSize;

    VertexBuffer->Vertices = (void*)GCMemory_Allocate(VertexBuffer->VertexSize);
    memcpy(VertexBuffer->Vertices, Description->Vertices, VertexBuffer->VertexSize);

    GCRendererVertexBuffer_CreateVertexBuffer(VertexBuffer);

    return VertexBuffer;
}

GCRendererVertexBuffer* GCRendererVertexBuffer_CreateDynamic(const GCRendererVertexBufferDescription* const Description)
{
    GCRendererVertexBuffer* VertexBuffer = (GCRendererVertexBuffer*)GCMemory_Allocate(sizeof(GCRendererVertexBuffer));
    VertexBuffer->Device = Description->Device;
    VertexBuffer->CommandList = Description->CommandList;
    VertexBuffer->VertexBufferHandle = VK_NULL_HANDLE;
    VertexBuffer->VertexBufferMemoryHandle = VK_NULL_HANDLE;
    VertexBuffer->Vertices = Description->Vertices;
    VertexBuffer->VertexCount = Description->VertexCount;
    VertexBuffer->VertexSize = Description->VertexSize;

    VertexBuffer->Vertices = (void*)GCMemory_Allocate(VertexBuffer->VertexSize);
    memcpy(VertexBuffer->Vertices, Description->Vertices, VertexBuffer->VertexSize);

    GCRendererVertexBuffer_CreateVertexBufferDynamic(VertexBuffer);
    GCRendererVertexBuffer_SetVertices(VertexBuffer, VertexBuffer->Vertices, VertexBuffer->VertexSize);

    return VertexBuffer;
}

void GCRendererVertexBuffer_SetVertices(GCRendererVertexBuffer* const VertexBuffer, const void* const Vertices,
                                        const size_t VertexSize)
{
    memcpy(VertexBuffer->DynamicVertices, Vertices, VertexSize);
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
    return VertexBuffer->VertexBufferHandle;
}

void GCRendererVertexBuffer_CreateVertexBuffer(GCRendererVertexBuffer* const VertexBuffer)
{
    const VkDevice DeviceHandle = GCRendererDevice_GetDeviceHandle(VertexBuffer->Device);

    VkBuffer StagingVertexBufferHandle = VK_NULL_HANDLE;
    VkDeviceMemory StagingVertexBufferMemoryHandle = VK_NULL_HANDLE;

    GCVulkanUtilities_CreateBuffer(VertexBuffer->Device, VertexBuffer->VertexSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                   &StagingVertexBufferHandle, &StagingVertexBufferMemoryHandle);

    void* VertexData = NULL;
    vkMapMemory(DeviceHandle, StagingVertexBufferMemoryHandle, 0, VertexBuffer->VertexSize, 0, &VertexData);
    memcpy(VertexData, VertexBuffer->Vertices, VertexBuffer->VertexSize);
    vkUnmapMemory(DeviceHandle, StagingVertexBufferMemoryHandle);

    GCVulkanUtilities_CreateBuffer(VertexBuffer->Device, VertexBuffer->VertexSize,
                                   VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                                   VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &VertexBuffer->VertexBufferHandle,
                                   &VertexBuffer->VertexBufferMemoryHandle);

    const VkCommandBuffer CommandBufferHandle =
        GCRendererCommandList_BeginSingleTimeCommands(VertexBuffer->CommandList);
    GCVulkanUtilities_CopyBuffer(CommandBufferHandle, StagingVertexBufferHandle, VertexBuffer->VertexBufferHandle,
                                 VertexBuffer->VertexSize);
    GCRendererCommandList_EndSingleTimeCommands(VertexBuffer->CommandList, CommandBufferHandle);

    vkFreeMemory(DeviceHandle, StagingVertexBufferMemoryHandle, NULL);
    vkDestroyBuffer(DeviceHandle, StagingVertexBufferHandle, NULL);
}

void GCRendererVertexBuffer_CreateVertexBufferDynamic(GCRendererVertexBuffer* const VertexBuffer)
{
    const VkDevice DeviceHandle = GCRendererDevice_GetDeviceHandle(VertexBuffer->Device);

    GCVulkanUtilities_CreateBuffer(VertexBuffer->Device, VertexBuffer->VertexSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                                   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                   &VertexBuffer->VertexBufferHandle, &VertexBuffer->VertexBufferMemoryHandle);

    vkMapMemory(DeviceHandle, VertexBuffer->VertexBufferMemoryHandle, 0, VertexBuffer->VertexSize, 0,
                &VertexBuffer->DynamicVertices);
}

void GCRendererVertexBuffer_DestroyObjects(GCRendererVertexBuffer* const VertexBuffer)
{
    const VkDevice DeviceHandle = GCRendererDevice_GetDeviceHandle(VertexBuffer->Device);

    vkFreeMemory(DeviceHandle, VertexBuffer->VertexBufferMemoryHandle, NULL);
    vkDestroyBuffer(DeviceHandle, VertexBuffer->VertexBufferHandle, NULL);
}