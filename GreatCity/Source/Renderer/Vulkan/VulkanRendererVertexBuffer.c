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

#include "Renderer/RendererVertexBuffer.h"
#include "Renderer/Vulkan/VulkanUtilities.h"
#include "Core/Memory/Allocator.h"
#include "Core/Log.h"
#include "Core/Assert.h"

#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include <vulkan/vulkan.h>

typedef struct GCRendererVertexBuffer
{
	const GCRendererDevice* Device;
	const GCRendererCommandList* CommandList;

	VkBuffer VertexBufferHandle;
	VkDeviceMemory VertexBufferMemoryHandle;

	void* Vertices;
	size_t VertexSize;
} GCRendererVertexBuffer;

VkBuffer GCRendererVertexBuffer_GetHandle(const GCRendererVertexBuffer* const VertexBuffer);

extern VkDevice GCRendererDevice_GetDeviceHandle(const GCRendererDevice* const Device);

static void GCRendererVertexBuffer_CreateVertexBuffer(GCRendererVertexBuffer* const VertexBuffer);
static void GCRendererVertexBuffer_CreateVertexBufferDynamic(GCRendererVertexBuffer* const VertexBuffer);
static void GCRendererVertexBuffer_DestroyObjects(GCRendererVertexBuffer* const VertexBuffer);

GCRendererVertexBuffer* GCRendererVertexBuffer_Create(const GCRendererDevice* const Device, const GCRendererCommandList* const CommandList, void* const Vertices, const size_t VertexSize)
{
	GCRendererVertexBuffer* VertexBuffer = (GCRendererVertexBuffer*)GCMemory_Allocate(sizeof(GCRendererVertexBuffer));
	VertexBuffer->Device = Device;
	VertexBuffer->CommandList = CommandList;
	VertexBuffer->VertexBufferHandle = VK_NULL_HANDLE;
	VertexBuffer->VertexBufferMemoryHandle = VK_NULL_HANDLE;
	VertexBuffer->Vertices = Vertices;
	VertexBuffer->VertexSize = VertexSize;

	GCRendererVertexBuffer_CreateVertexBuffer(VertexBuffer);

	return VertexBuffer;
}

GCRendererVertexBuffer* GCRendererVertexBuffer_CreateDynamic(const GCRendererDevice* const Device, const GCRendererCommandList* const CommandList, const size_t VertexSize)
{
	GCRendererVertexBuffer* VertexBuffer = (GCRendererVertexBuffer*)GCMemory_Allocate(sizeof(GCRendererVertexBuffer));
	VertexBuffer->Device = Device;
	VertexBuffer->CommandList = CommandList;
	VertexBuffer->VertexBufferHandle = VK_NULL_HANDLE;
	VertexBuffer->VertexBufferMemoryHandle = VK_NULL_HANDLE;
	VertexBuffer->Vertices = NULL;
	VertexBuffer->VertexSize = VertexSize;

	GCRendererVertexBuffer_CreateVertexBufferDynamic(VertexBuffer);

	return VertexBuffer;
}

void GCRendererVertexBuffer_SetVertices(GCRendererVertexBuffer* const VertexBuffer, const void* const Vertices, const size_t VertexSize)
{
	memcpy(VertexBuffer->Vertices, Vertices, VertexSize);
}

void GCRendererVertexBuffer_Destroy(GCRendererVertexBuffer* VertexBuffer)
{
	GCRendererVertexBuffer_DestroyObjects(VertexBuffer);

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

	GCVulkanUtilities_CreateBuffer(VertexBuffer->Device, VertexBuffer->VertexSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &StagingVertexBufferHandle, &StagingVertexBufferMemoryHandle);

	void* VertexData = NULL;
	vkMapMemory(DeviceHandle, StagingVertexBufferMemoryHandle, 0, VertexBuffer->VertexSize, 0, &VertexData);
	memcpy(VertexData, VertexBuffer->Vertices, VertexBuffer->VertexSize);
	vkUnmapMemory(DeviceHandle, StagingVertexBufferMemoryHandle);

	GCVulkanUtilities_CreateBuffer(VertexBuffer->Device, VertexBuffer->VertexSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &VertexBuffer->VertexBufferHandle, &VertexBuffer->VertexBufferMemoryHandle);
	GCVulkanUtilities_CopyBuffer(VertexBuffer->Device, VertexBuffer->CommandList, StagingVertexBufferHandle, VertexBuffer->VertexBufferHandle, VertexBuffer->VertexSize);

	vkFreeMemory(DeviceHandle, StagingVertexBufferMemoryHandle, NULL);
	vkDestroyBuffer(DeviceHandle, StagingVertexBufferHandle, NULL);
}

void GCRendererVertexBuffer_CreateVertexBufferDynamic(GCRendererVertexBuffer* const VertexBuffer)
{
	const VkDevice DeviceHandle = GCRendererDevice_GetDeviceHandle(VertexBuffer->Device);

	GCVulkanUtilities_CreateBuffer(VertexBuffer->Device, VertexBuffer->VertexSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &VertexBuffer->VertexBufferHandle, &VertexBuffer->VertexBufferMemoryHandle);

	vkMapMemory(DeviceHandle, VertexBuffer->VertexBufferMemoryHandle, 0, VertexBuffer->VertexSize, 0, &VertexBuffer->Vertices);
}

void GCRendererVertexBuffer_DestroyObjects(GCRendererVertexBuffer* const VertexBuffer)
{
	const VkDevice DeviceHandle = GCRendererDevice_GetDeviceHandle(VertexBuffer->Device);

	vkFreeMemory(DeviceHandle, VertexBuffer->VertexBufferMemoryHandle, NULL);
	vkDestroyBuffer(DeviceHandle, VertexBuffer->VertexBufferHandle, NULL);
}