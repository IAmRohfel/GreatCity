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
#include "Renderer/Vulkan/VulkanRendererDevice.h"
#include "Renderer/Vulkan/VulkanRendererCommandList.h"
#include "Renderer/Vulkan/VulkanUtilities.h"
#include "Renderer/RendererUniformBuffer.h"
#include "Renderer/RendererDevice.h"
#include "Core/Memory/Allocator.h"
#include "Core/Log.h"
#include "Core/Assert.h"

#include <string.h>
#include <stdbool.h>

#include <vulkan/vulkan.h>

typedef struct GCRendererUniformBuffer
{
	const GCRendererDevice* Device;
	const GCRendererCommandList* CommandList;

	VkBuffer UniformBufferHandle;
	VkDeviceMemory UniformBufferMemoryHandle;

	void* Data;
	size_t DataSize;
} GCRendererUniformBuffer;

static void GCRendererUniformBuffer_CreateUniformBuffer(GCRendererUniformBuffer* const UniformBuffer);
static void GCRendererUniformBuffer_DestroyObjects(GCRendererUniformBuffer* const UniformBuffer);

GCRendererUniformBuffer* GCRendererUniformBuffer_Create(const GCRendererUniformBufferDescription* const Description)
{
	GCRendererUniformBuffer* UniformBuffer = (GCRendererUniformBuffer*)GCMemory_Allocate(sizeof(GCRendererUniformBuffer));
	UniformBuffer->Device = Description->Device;
	UniformBuffer->CommandList = Description->CommandList;
	UniformBuffer->UniformBufferHandle = VK_NULL_HANDLE;
	UniformBuffer->UniformBufferMemoryHandle = VK_NULL_HANDLE;
	UniformBuffer->Data = NULL;
	UniformBuffer->DataSize = Description->DataSize;

	GCRendererUniformBuffer_CreateUniformBuffer(UniformBuffer);

	return UniformBuffer;
}

void GCRendererUniformBuffer_UpdateUniformBuffer(const GCRendererUniformBuffer* const UniformBuffer, const void* const Data, const size_t DataSize)
{
	memcpy(UniformBuffer->Data, Data, DataSize);
}

void GCRendererUniformBuffer_Destroy(GCRendererUniformBuffer* UniformBuffer)
{
	GCRendererDevice_WaitIdle(UniformBuffer->Device);

	GCRendererUniformBuffer_DestroyObjects(UniformBuffer);

	GCMemory_Free(UniformBuffer);
}

VkBuffer GCRendererUniformBuffer_GetBufferHandle(const GCRendererUniformBuffer* const UniformBuffer)
{
	return UniformBuffer->UniformBufferHandle;
}

void* GCRendererUniformBuffer_GetData(const GCRendererUniformBuffer* const UniformBuffer)
{
	return UniformBuffer->Data;
}

size_t GCRendererUniformBuffer_GetDataSize(const GCRendererUniformBuffer* const UniformBuffer)
{
	return UniformBuffer->DataSize;
}

void GCRendererUniformBuffer_CreateUniformBuffer(GCRendererUniformBuffer* const UniformBuffer)
{
	const VkDevice DeviceHandle = GCRendererDevice_GetDeviceHandle(UniformBuffer->Device);

	GCVulkanUtilities_CreateBuffer(UniformBuffer->Device, UniformBuffer->DataSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &UniformBuffer->UniformBufferHandle, &UniformBuffer->UniformBufferMemoryHandle);

	vkMapMemory(DeviceHandle, UniformBuffer->UniformBufferMemoryHandle, 0, UniformBuffer->DataSize, 0, &UniformBuffer->Data);
}

void GCRendererUniformBuffer_DestroyObjects(GCRendererUniformBuffer* const UniformBuffer)
{
	const VkDevice DeviceHandle = GCRendererDevice_GetDeviceHandle(UniformBuffer->Device);

	vkFreeMemory(DeviceHandle, UniformBuffer->UniformBufferMemoryHandle, NULL);
	vkDestroyBuffer(DeviceHandle, UniformBuffer->UniformBufferHandle, NULL);
}