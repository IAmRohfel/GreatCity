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

#include "Renderer/Vulkan/VulkanRendererTexture2D.h"
#include "Renderer/Vulkan/VulkanRendererDevice.h"
#include "Renderer/Vulkan/VulkanUtilities.h"
#include "Renderer/RendererTexture2D.h"
#include "Renderer/RendererDevice.h"
#include "Core/Memory/Allocator.h"
#include "Core/Log.h"
#include "Core/Assert.h"

#include <string.h>
#include <math.h>
#include <stdint.h>
#include <stdbool.h>

#include <vulkan/vulkan.h>
#include <stb_image.h>

typedef struct GCRendererTexture2D
{
	const GCRendererDevice* Device;
	const GCRendererCommandList* CommandList;

	VkImage ImageHandle;
	VkDeviceMemory ImageMemoryHandle;
	VkImageView ImageViewHandle;
	VkSampler ImageSamplerHandle;

	VkDescriptorSet DescriptorSetHandle;
} GCRendererTexture2D;

static uint8_t* GCRendererTexture2D_LoadImage(const char* const TexturePath, uint32_t* const Width, uint32_t* const Height, uint32_t* const Channels);
static void GCRendererTexture2D_CreateTexture(GCRendererTexture2D* const Texture2D, const char* const TexturePath);
static void GCRendererTexture2D_DestroyObjects(GCRendererTexture2D* const Texture2D);

GCRendererTexture2D* GCRendererTexture2D_Create(const GCRendererTexture2DDescription* const Description)
{
	GCRendererTexture2D* Texture2D = (GCRendererTexture2D*)GCMemory_Allocate(sizeof(GCRendererTexture2D));
	Texture2D->Device = Description->Device;
	Texture2D->CommandList = Description->CommandList;
	Texture2D->ImageHandle = VK_NULL_HANDLE;
	Texture2D->ImageMemoryHandle = VK_NULL_HANDLE;
	Texture2D->ImageViewHandle = VK_NULL_HANDLE;
	Texture2D->ImageSamplerHandle = VK_NULL_HANDLE;
	Texture2D->DescriptorSetHandle = VK_NULL_HANDLE;

	GCRendererTexture2D_CreateTexture(Texture2D, Description->TexturePath);

	return Texture2D;
}

void GCRendererTexture2D_Destroy(GCRendererTexture2D* Texture2D)
{
	GCRendererTexture2D_DestroyObjects(Texture2D);

	GCMemory_Free(Texture2D);
}

VkImageView GCRendererTexture2D_GetImageViewHandle(const GCRendererTexture2D* const Texture2D)
{
	return Texture2D->ImageViewHandle;
}

VkSampler GCRendererTexture2D_GetSamplerHandle(const GCRendererTexture2D* const Texture2D)
{
	return Texture2D->ImageSamplerHandle;
}

uint8_t* GCRendererTexture2D_LoadImage(const char* const TexturePath, uint32_t* const Width, uint32_t* const Height, uint32_t* const Channels)
{
	int32_t TextureWidth = 0, TextureHeight = 0, TextureChannels = 0;
	stbi_uc* TextureData = stbi_load(TexturePath, &TextureWidth, &TextureHeight, &TextureChannels, STBI_rgb_alpha);

	if (!TextureData)
	{
		GC_ASSERT_WITH_MESSAGE(false, "Failed to load an image file at: %s", TexturePath);
	}

	*Width = TextureWidth;
	*Height = TextureHeight;
	*Channels = TextureChannels;

	return TextureData;
}

void GCRendererTexture2D_CreateTexture(GCRendererTexture2D* const Texture2D, const char* const TexturePath)
{
	const VkDevice DeviceHandle = GCRendererDevice_GetDeviceHandle(Texture2D->Device);

	uint32_t TextureWidth = 0, TextureHeight = 0, TextureChannels = 0;
	uint8_t* TextureData = GCRendererTexture2D_LoadImage(TexturePath, &TextureWidth, &TextureHeight, &TextureChannels);

	const size_t ImageSize = TextureWidth * TextureHeight * 4;
	const uint32_t MipLevels = (uint32_t)floorf(log2f(fmaxf((float)TextureWidth, (float)TextureHeight))) + 1;

	VkBuffer StagingImageBufferHandle = VK_NULL_HANDLE;
	VkDeviceMemory StagingImageBufferMemoryHandle = VK_NULL_HANDLE;

	GCVulkanUtilities_CreateBuffer(Texture2D->Device, ImageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &StagingImageBufferHandle, &StagingImageBufferMemoryHandle);

	void* ImageData = NULL;
	vkMapMemory(DeviceHandle, StagingImageBufferMemoryHandle, 0, ImageSize, 0, &ImageData);
	memcpy(ImageData, TextureData, ImageSize);
	vkUnmapMemory(DeviceHandle, StagingImageBufferMemoryHandle);

	stbi_image_free(TextureData);

	GCVulkanUtilities_CreateImage(Texture2D->Device, TextureWidth, TextureHeight, MipLevels, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &Texture2D->ImageHandle, &Texture2D->ImageMemoryHandle);

	const VkCommandBuffer CommandBufferHandle = GCVulkanUtilities_BeginSingleTimeCommands(Texture2D->Device, Texture2D->CommandList);
	GCVulkanUtilities_TransitionImageLayout(CommandBufferHandle, Texture2D->ImageHandle, MipLevels, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	GCVulkanUtilities_CopyBufferToImage(CommandBufferHandle, StagingImageBufferHandle, Texture2D->ImageHandle, TextureWidth, TextureHeight);
	GCVulkanUtilities_GenerateMipmap(Texture2D->Device, CommandBufferHandle, Texture2D->ImageHandle, TextureWidth, TextureHeight, MipLevels, VK_FORMAT_R8G8B8A8_SRGB);
	GCVulkanUtilities_EndSingleTimeCommands(Texture2D->Device, Texture2D->CommandList, CommandBufferHandle);

	vkFreeMemory(DeviceHandle, StagingImageBufferMemoryHandle, NULL);
	vkDestroyBuffer(DeviceHandle, StagingImageBufferHandle, NULL);

	GCVulkanUtilities_CreateImageView(Texture2D->Device, Texture2D->ImageHandle, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT, MipLevels, &Texture2D->ImageViewHandle);
	GCVulkanUtilities_CreateSampler(Texture2D->Device, VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_REPEAT, MipLevels, &Texture2D->ImageSamplerHandle);
}

void GCRendererTexture2D_DestroyObjects(GCRendererTexture2D* const Texture2D)
{
	const VkDevice DeviceHandle = GCRendererDevice_GetDeviceHandle(Texture2D->Device);

	vkDestroySampler(DeviceHandle, Texture2D->ImageSamplerHandle, NULL);

	vkDestroyImageView(DeviceHandle, Texture2D->ImageViewHandle, NULL);

	vkFreeMemory(DeviceHandle, Texture2D->ImageMemoryHandle, NULL);
	vkDestroyImage(DeviceHandle, Texture2D->ImageHandle, NULL);
}