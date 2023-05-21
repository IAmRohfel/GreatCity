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

	const void* Vertices;
	size_t VertexSize;
} GCRendererVertexBuffer;

VkBuffer GCRendererVertexBuffer_GetHandle(const GCRendererVertexBuffer* const VertexBuffer);

extern VkDevice GCRendererDevice_GetDeviceHandle(const GCRendererDevice* const Device);

static void GCRendererVertexBuffer_CreateVertexBuffer(GCRendererVertexBuffer* const VertexBuffer);
static void GCRendererVertexBuffer_DestroyObjects(GCRendererVertexBuffer* const VertexBuffer);

GCRendererVertexBuffer* GCRendererVertexBuffer_Create(const GCRendererDevice* const Device, const GCRendererCommandList* const CommandList, const void* const Vertices, const size_t VertexSize)
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

void GCRendererVertexBuffer_DestroyObjects(GCRendererVertexBuffer* const VertexBuffer)
{
	const VkDevice DeviceHandle = GCRendererDevice_GetDeviceHandle(VertexBuffer->Device);

	vkFreeMemory(DeviceHandle, VertexBuffer->VertexBufferMemoryHandle, NULL);
	vkDestroyBuffer(DeviceHandle, VertexBuffer->VertexBufferHandle, NULL);
}