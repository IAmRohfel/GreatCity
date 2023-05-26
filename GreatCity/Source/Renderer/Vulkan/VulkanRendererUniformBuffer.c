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

	VkDescriptorSetLayout DescriptorSetLayoutHandle;
	VkBuffer* UniformBufferHandles;
	VkDeviceMemory* UniformBufferMemoryHandles;
	VkDescriptorPool DescriptorPoolHandle;
	VkDescriptorSet* DescriptorSetHandles;

	void** Data;
	size_t DataSize;
} GCRendererUniformBuffer;

VkDescriptorSetLayout GCRendererUniformBuffer_GetDescriptorSetLayoutHandle(const GCRendererUniformBuffer* const UniformBuffer);
VkDescriptorSet* GCRendererUniformBuffer_GetDescriptorSetHandles(const GCRendererUniformBuffer* const UniformBuffer);
void** GCRendererUniformBuffer_GetData(const GCRendererUniformBuffer* const UniformBuffer);

extern VkDevice GCRendererDevice_GetDeviceHandle(const GCRendererDevice* const Device);
extern uint32_t GCRendererCommandList_GetMaximumFramesInFlight(const GCRendererCommandList* const CommandList);

static void GCRendererUniformBuffer_CreateDescriptorSetLayout(GCRendererUniformBuffer* const UniformBuffer);
static void GCRendererUniformBuffer_CreateUniformBuffer(GCRendererUniformBuffer* const UniformBuffer);
static void GCRendererUniformBuffer_CreateDescriptorPool(GCRendererUniformBuffer* const UniformBuffer);
static void GCRendererUniformBuffer_CreateDescriptorSets(GCRendererUniformBuffer* const UniformBuffer);
static void GCRendererUniformBuffer_DestroyObjects(GCRendererUniformBuffer* const UniformBuffer);

GCRendererUniformBuffer* GCRendererUniformBuffer_Create(const GCRendererDevice* const Device, const GCRendererCommandList* const CommandList, const size_t DataSize)
{
	GCRendererUniformBuffer* UniformBuffer = (GCRendererUniformBuffer*)GCMemory_Allocate(sizeof(GCRendererUniformBuffer));
	UniformBuffer->Device = Device;
	UniformBuffer->CommandList = CommandList;
	UniformBuffer->DescriptorSetLayoutHandle = VK_NULL_HANDLE;
	UniformBuffer->UniformBufferHandles = NULL;
	UniformBuffer->UniformBufferMemoryHandles = NULL;
	UniformBuffer->DescriptorPoolHandle = VK_NULL_HANDLE;
	UniformBuffer->DescriptorSetHandles = NULL;
	UniformBuffer->Data = NULL;
	UniformBuffer->DataSize = DataSize;

	GCRendererUniformBuffer_CreateDescriptorSetLayout(UniformBuffer);
	GCRendererUniformBuffer_CreateUniformBuffer(UniformBuffer);
	GCRendererUniformBuffer_CreateDescriptorPool(UniformBuffer);
	GCRendererUniformBuffer_CreateDescriptorSets(UniformBuffer);

	return UniformBuffer;
}

void GCRendererUniformBuffer_Destroy(GCRendererUniformBuffer* UniformBuffer)
{
	GCRendererUniformBuffer_DestroyObjects(UniformBuffer);

	GCMemory_Free(UniformBuffer->Data);
	GCMemory_Free(UniformBuffer->DescriptorSetHandles);
	GCMemory_Free(UniformBuffer->UniformBufferMemoryHandles);
	GCMemory_Free(UniformBuffer->UniformBufferHandles);
	GCMemory_Free(UniformBuffer);
}

VkDescriptorSetLayout GCRendererUniformBuffer_GetDescriptorSetLayoutHandle(const GCRendererUniformBuffer* const UniformBuffer)
{
	return UniformBuffer->DescriptorSetLayoutHandle;
}

VkDescriptorSet* GCRendererUniformBuffer_GetDescriptorSetHandles(const GCRendererUniformBuffer* const UniformBuffer)
{
	return UniformBuffer->DescriptorSetHandles;
}

void** GCRendererUniformBuffer_GetData(const GCRendererUniformBuffer* const UniformBuffer)
{
	return UniformBuffer->Data;
}

void GCRendererUniformBuffer_CreateDescriptorSetLayout(GCRendererUniformBuffer* const UniformBuffer)
{
	const VkDevice DeviceHandle = GCRendererDevice_GetDeviceHandle(UniformBuffer->Device);

	VkDescriptorSetLayoutBinding DescriptorSetLayoutBinding = { 0 };
	DescriptorSetLayoutBinding.binding = 0;
	DescriptorSetLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	DescriptorSetLayoutBinding.descriptorCount = 1;
	DescriptorSetLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

	VkDescriptorSetLayoutCreateInfo DescriptorSetLayoutInformation = { 0 };
	DescriptorSetLayoutInformation.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	DescriptorSetLayoutInformation.bindingCount = 1;
	DescriptorSetLayoutInformation.pBindings = &DescriptorSetLayoutBinding;

	GC_VULKAN_VALIDATE(vkCreateDescriptorSetLayout(DeviceHandle, &DescriptorSetLayoutInformation, NULL, &UniformBuffer->DescriptorSetLayoutHandle), "Failed to create a Vulkan uniform buffer descriptor set layout");
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

void GCRendererUniformBuffer_CreateDescriptorPool(GCRendererUniformBuffer* const UniformBuffer)
{
	const VkDevice DeviceHandle = GCRendererDevice_GetDeviceHandle(UniformBuffer->Device);
	const uint32_t MaximumFramesInFlight = GCRendererCommandList_GetMaximumFramesInFlight(UniformBuffer->CommandList);

	VkDescriptorPoolSize DescriptorPoolSize = { 0 };
	DescriptorPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	DescriptorPoolSize.descriptorCount = MaximumFramesInFlight;

	VkDescriptorPoolCreateInfo DescriptorPoolInformation = { 0 };
	DescriptorPoolInformation.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	DescriptorPoolInformation.maxSets = MaximumFramesInFlight;
	DescriptorPoolInformation.poolSizeCount = 1;
	DescriptorPoolInformation.pPoolSizes = &DescriptorPoolSize;

	GC_VULKAN_VALIDATE(vkCreateDescriptorPool(DeviceHandle, &DescriptorPoolInformation, NULL, &UniformBuffer->DescriptorPoolHandle), "Failed to create a Vulkan uniform buffer descriptor pool");
}

void GCRendererUniformBuffer_CreateDescriptorSets(GCRendererUniformBuffer* const UniformBuffer)
{
	const VkDevice DeviceHandle = GCRendererDevice_GetDeviceHandle(UniformBuffer->Device);
	const uint32_t MaximumFramesInFlight = GCRendererCommandList_GetMaximumFramesInFlight(UniformBuffer->CommandList);

	VkDescriptorSetLayout* const DescriptorSetLayoutHandles = (VkDescriptorSetLayout* const)GCMemory_Allocate(MaximumFramesInFlight * sizeof(VkDescriptorSetLayout));

	for (uint32_t Counter = 0; Counter < MaximumFramesInFlight; Counter++)
	{
		DescriptorSetLayoutHandles[Counter] = UniformBuffer->DescriptorSetLayoutHandle;
	}

	UniformBuffer->DescriptorSetHandles = (VkDescriptorSet*)GCMemory_Allocate(MaximumFramesInFlight * sizeof(VkDescriptorSet));

	VkDescriptorSetAllocateInfo DescriptorSetAllocateInformation = { 0 };
	DescriptorSetAllocateInformation.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	DescriptorSetAllocateInformation.descriptorPool = UniformBuffer->DescriptorPoolHandle;
	DescriptorSetAllocateInformation.descriptorSetCount = MaximumFramesInFlight;
	DescriptorSetAllocateInformation.pSetLayouts = DescriptorSetLayoutHandles;

	GC_VULKAN_VALIDATE(vkAllocateDescriptorSets(DeviceHandle, &DescriptorSetAllocateInformation, UniformBuffer->DescriptorSetHandles), "Failed to allocate Vulkan uniform buffer descriptor sets");

	for (uint32_t Counter = 0; Counter < MaximumFramesInFlight; Counter++)
	{
		VkDescriptorBufferInfo DescriptorBufferInformation = { 0 };
		DescriptorBufferInformation.buffer = UniformBuffer->UniformBufferHandles[Counter];
		DescriptorBufferInformation.offset = 0;
		DescriptorBufferInformation.range = UniformBuffer->DataSize;

		VkWriteDescriptorSet WriteDescriptorSet = { 0 };
		WriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		WriteDescriptorSet.dstSet = UniformBuffer->DescriptorSetHandles[Counter];
		WriteDescriptorSet.dstBinding = 0;
		WriteDescriptorSet.dstArrayElement = 0;
		WriteDescriptorSet.descriptorCount = 1;
		WriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		WriteDescriptorSet.pBufferInfo = &DescriptorBufferInformation;

		vkUpdateDescriptorSets(DeviceHandle, 1, &WriteDescriptorSet, 0, NULL);
	}

	GCMemory_Free(DescriptorSetLayoutHandles);
}

void GCRendererUniformBuffer_DestroyObjects(GCRendererUniformBuffer* const UniformBuffer)
{
	const VkDevice DeviceHandle = GCRendererDevice_GetDeviceHandle(UniformBuffer->Device);
	const uint32_t MaximumFramesInFlight = GCRendererCommandList_GetMaximumFramesInFlight(UniformBuffer->CommandList);

	vkDestroyDescriptorPool(DeviceHandle, UniformBuffer->DescriptorPoolHandle, NULL);

	for (uint32_t Counter = 0; Counter < MaximumFramesInFlight; Counter++)
	{
		vkFreeMemory(DeviceHandle, UniformBuffer->UniformBufferMemoryHandles[Counter], NULL);
		vkDestroyBuffer(DeviceHandle, UniformBuffer->UniformBufferHandles[Counter], NULL);
	}

	vkDestroyDescriptorSetLayout(DeviceHandle, UniformBuffer->DescriptorSetLayoutHandle, NULL);
}