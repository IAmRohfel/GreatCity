#include "Renderer/Vulkan/VulkanUtilities.h"
#include "Renderer/RendererVertexBuffer.h"
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
} GCRendererVertexBuffer;

VkBuffer GCRendererVertexBuffer_GetHandle(const GCRendererVertexBuffer* const VertexBuffer);

extern VkDevice GCRendererDevice_GetDeviceHandle(const GCRendererDevice* const Device);

static void GCRendererVertexBuffer_CreateVertexBuffer(GCRendererVertexBuffer* const VertexBuffer, const void* const Vertices, const size_t VertexSize);
static void GCRendererVertexBuffer_DestroyObjects(GCRendererVertexBuffer* const VertexBuffer);

GCRendererVertexBuffer* GCRendererVertexBuffer_Create(const GCRendererDevice* const Device, const GCRendererCommandList* const CommandList, const void* const Vertices, const size_t VertexSize)
{
	GCRendererVertexBuffer* VertexBuffer = (GCRendererVertexBuffer*)GCMemory_Allocate(sizeof(GCRendererVertexBuffer));
	VertexBuffer->Device = Device;
	VertexBuffer->CommandList = CommandList;
	VertexBuffer->VertexBufferHandle = VK_NULL_HANDLE;
	VertexBuffer->VertexBufferMemoryHandle = VK_NULL_HANDLE;

	GCRendererVertexBuffer_CreateVertexBuffer(VertexBuffer, Vertices, VertexSize);

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

void GCRendererVertexBuffer_CreateVertexBuffer(GCRendererVertexBuffer* const VertexBuffer, const void* const Vertices, const size_t VertexSize)
{
	const VkDevice DeviceHandle = GCRendererDevice_GetDeviceHandle(VertexBuffer->Device);

	VkBuffer StagingVertexBufferHandle = VK_NULL_HANDLE;
	VkDeviceMemory StagingDeviceMemoryHandle = VK_NULL_HANDLE;

	GCVulkanUtilities_CreateBuffer(VertexBuffer->Device, VertexSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &StagingVertexBufferHandle, &StagingDeviceMemoryHandle);

	void* VertexData = NULL;
	vkMapMemory(DeviceHandle, StagingDeviceMemoryHandle, 0, VertexSize, 0, &VertexData);
	memcpy(VertexData, Vertices, VertexSize);
	vkUnmapMemory(DeviceHandle, StagingDeviceMemoryHandle);

	GCVulkanUtilities_CreateBuffer(VertexBuffer->Device, VertexSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &VertexBuffer->VertexBufferHandle, &VertexBuffer->VertexBufferMemoryHandle);
	GCVulkanUtilities_CopyBuffer(VertexBuffer->Device, VertexBuffer->CommandList, StagingVertexBufferHandle, VertexBuffer->VertexBufferHandle, VertexSize);

	vkFreeMemory(DeviceHandle, StagingDeviceMemoryHandle, NULL);
	vkDestroyBuffer(DeviceHandle, StagingVertexBufferHandle, NULL);
}

void GCRendererVertexBuffer_DestroyObjects(GCRendererVertexBuffer* const VertexBuffer)
{
	const VkDevice DeviceHandle = GCRendererDevice_GetDeviceHandle(VertexBuffer->Device);

	vkDestroyBuffer(DeviceHandle, VertexBuffer->VertexBufferHandle, NULL);
	vkFreeMemory(DeviceHandle, VertexBuffer->VertexBufferMemoryHandle, NULL);
}