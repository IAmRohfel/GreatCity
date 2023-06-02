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

#include "Renderer/RendererIndexBuffer.h"
#include "Renderer/Vulkan/VulkanUtilities.h"
#include "Core/Memory/Allocator.h"
#include "Core/Log.h"
#include "Core/Assert.h"

#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include <vulkan/vulkan.h>

typedef struct GCRendererIndexBuffer
{
	const GCRendererDevice* Device;
	const GCRendererCommandList* CommandList;

	VkBuffer IndexBufferHandle;
	VkDeviceMemory IndexBufferMemoryHandle;

	const uint32_t* Indices;
	size_t IndexSize;
} GCRendererIndexBuffer;

VkBuffer GCRendererIndexBuffer_GetHandle(const GCRendererIndexBuffer* const IndexBuffer);

extern VkDevice GCRendererDevice_GetDeviceHandle(const GCRendererDevice* const Device);

static void GCRendererIndexBuffer_CreateIndexBuffer(GCRendererIndexBuffer* const IndexBuffer);
static void GCRendererIndexBuffer_DestroyObjects(GCRendererIndexBuffer* const IndexBuffer);

GCRendererIndexBuffer* GCRendererIndexBuffer_Create(const GCRendererDevice* const Device, const GCRendererCommandList* const CommandList, const uint32_t* const Indices, const size_t IndexSize)
{
	GCRendererIndexBuffer* IndexBuffer = (GCRendererIndexBuffer*)GCMemory_Allocate(sizeof(GCRendererIndexBuffer));
	IndexBuffer->Device = Device;
	IndexBuffer->CommandList = CommandList;
	IndexBuffer->IndexBufferHandle = VK_NULL_HANDLE;
	IndexBuffer->IndexBufferMemoryHandle = VK_NULL_HANDLE;
	IndexBuffer->Indices = Indices;
	IndexBuffer->IndexSize = IndexSize;

	GCRendererIndexBuffer_CreateIndexBuffer(IndexBuffer);

	return IndexBuffer;
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

	GCVulkanUtilities_CreateBuffer(IndexBuffer->Device, IndexBuffer->IndexSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &StagingIndexBufferHandle, &StagingIndexBufferMemoryHandle);

	void* IndexData = NULL;
	vkMapMemory(DeviceHandle, StagingIndexBufferMemoryHandle, 0, IndexBuffer->IndexSize, 0, &IndexData);
	memcpy(IndexData, IndexBuffer->Indices, IndexBuffer->IndexSize);
	vkUnmapMemory(DeviceHandle, StagingIndexBufferMemoryHandle);

	GCVulkanUtilities_CreateBuffer(IndexBuffer->Device, IndexBuffer->IndexSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &IndexBuffer->IndexBufferHandle, &IndexBuffer->IndexBufferMemoryHandle);
	GCVulkanUtilities_CopyBuffer(IndexBuffer->Device, IndexBuffer->CommandList, StagingIndexBufferHandle, IndexBuffer->IndexBufferHandle, IndexBuffer->IndexSize);

	vkFreeMemory(DeviceHandle, StagingIndexBufferMemoryHandle, NULL);
	vkDestroyBuffer(DeviceHandle, StagingIndexBufferHandle, NULL);
}

void GCRendererIndexBuffer_DestroyObjects(GCRendererIndexBuffer* const IndexBuffer)
{
	const VkDevice DeviceHandle = GCRendererDevice_GetDeviceHandle(IndexBuffer->Device);

	vkFreeMemory(DeviceHandle, IndexBuffer->IndexBufferMemoryHandle, NULL);
	vkDestroyBuffer(DeviceHandle, IndexBuffer->IndexBufferHandle, NULL);
}