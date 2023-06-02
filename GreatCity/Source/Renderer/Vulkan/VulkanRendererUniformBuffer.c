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

#include "Renderer/RendererUniformBuffer.h"
#include "Renderer/Vulkan/VulkanUtilities.h"
#include "Core/Memory/Allocator.h"
#include "Core/Log.h"
#include "Core/Assert.h"

#include <stdbool.h>

#include <vulkan/vulkan.h>

typedef struct GCRendererUniformBuffer
{
	const GCRendererDevice* Device;
	const GCRendererCommandList* CommandList;

	VkBuffer* UniformBufferHandles;
	VkDeviceMemory* UniformBufferMemoryHandles;

	void** Data;
	size_t DataSize;
} GCRendererUniformBuffer;

VkBuffer* GCRendererUniformBuffer_GetBufferHandles(const GCRendererUniformBuffer* const UniformBuffer);
void** GCRendererUniformBuffer_GetData(const GCRendererUniformBuffer* const UniformBuffer);
size_t GCRendererUniformBuffer_GetDataSize(const GCRendererUniformBuffer* const UniformBuffer);

extern VkDevice GCRendererDevice_GetDeviceHandle(const GCRendererDevice* const Device);
extern uint32_t GCRendererCommandList_GetMaximumFramesInFlight(const GCRendererCommandList* const CommandList);

static void GCRendererUniformBuffer_CreateUniformBuffer(GCRendererUniformBuffer* const UniformBuffer);
static void GCRendererUniformBuffer_DestroyObjects(GCRendererUniformBuffer* const UniformBuffer);

GCRendererUniformBuffer* GCRendererUniformBuffer_Create(const GCRendererDevice* const Device, const GCRendererCommandList* const CommandList, const size_t DataSize)
{
	GCRendererUniformBuffer* UniformBuffer = (GCRendererUniformBuffer*)GCMemory_Allocate(sizeof(GCRendererUniformBuffer));
	UniformBuffer->Device = Device;
	UniformBuffer->CommandList = CommandList;
	UniformBuffer->UniformBufferHandles = NULL;
	UniformBuffer->UniformBufferMemoryHandles = NULL;
	UniformBuffer->Data = NULL;
	UniformBuffer->DataSize = DataSize;

	GCRendererUniformBuffer_CreateUniformBuffer(UniformBuffer);

	return UniformBuffer;
}

void GCRendererUniformBuffer_Destroy(GCRendererUniformBuffer* UniformBuffer)
{
	GCRendererUniformBuffer_DestroyObjects(UniformBuffer);

	GCMemory_Free(UniformBuffer->Data);
	GCMemory_Free(UniformBuffer->UniformBufferMemoryHandles);
	GCMemory_Free(UniformBuffer->UniformBufferHandles);
	GCMemory_Free(UniformBuffer);
}

VkBuffer* GCRendererUniformBuffer_GetBufferHandles(const GCRendererUniformBuffer* const UniformBuffer)
{
	return UniformBuffer->UniformBufferHandles;
}

void** GCRendererUniformBuffer_GetData(const GCRendererUniformBuffer* const UniformBuffer)
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

	const uint32_t MaximumFramesInFlight = GCRendererCommandList_GetMaximumFramesInFlight(UniformBuffer->CommandList);

	UniformBuffer->UniformBufferHandles = (VkBuffer*)GCMemory_Allocate(MaximumFramesInFlight * sizeof(VkBuffer));
	UniformBuffer->UniformBufferMemoryHandles = (VkDeviceMemory*)GCMemory_Allocate(MaximumFramesInFlight * sizeof(VkDeviceMemory));
	UniformBuffer->Data = (void**)GCMemory_Allocate(MaximumFramesInFlight * sizeof(void*));

	for (uint32_t Counter = 0; Counter < MaximumFramesInFlight; Counter++)
	{
		GCVulkanUtilities_CreateBuffer(UniformBuffer->Device, UniformBuffer->DataSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &UniformBuffer->UniformBufferHandles[Counter], &UniformBuffer->UniformBufferMemoryHandles[Counter]);

		vkMapMemory(DeviceHandle, UniformBuffer->UniformBufferMemoryHandles[Counter], 0, UniformBuffer->DataSize, 0, &UniformBuffer->Data[Counter]);
	}
}

void GCRendererUniformBuffer_DestroyObjects(GCRendererUniformBuffer* const UniformBuffer)
{
	const VkDevice DeviceHandle = GCRendererDevice_GetDeviceHandle(UniformBuffer->Device);
	const uint32_t MaximumFramesInFlight = GCRendererCommandList_GetMaximumFramesInFlight(UniformBuffer->CommandList);

	for (uint32_t Counter = 0; Counter < MaximumFramesInFlight; Counter++)
	{
		vkFreeMemory(DeviceHandle, UniformBuffer->UniformBufferMemoryHandles[Counter], NULL);
		vkDestroyBuffer(DeviceHandle, UniformBuffer->UniformBufferHandles[Counter], NULL);
	}
}