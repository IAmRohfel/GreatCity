#include "Renderer/RendererTexture2D.h"
#include "Renderer/RendererDevice.h"
#include "Renderer/Vulkan/VulkanUtilities.h"
#include "Core/Memory/Allocator.h"
#include "Core/Log.h"
#include "Core/Assert.h"

#include <string.h>
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
} GCRendererTexture2D;

VkImageView GCRendererTexture2D_GetImageViewHandle(const GCRendererTexture2D* const Texture2D);
VkSampler GCRendererTexture2D_GetSamplerHandle(const GCRendererTexture2D* const Texture2D);

extern VkDevice GCRendererDevice_GetDeviceHandle(const GCRendererDevice* const Device);

static uint8_t* GCRendererTexture2D_LoadImage(const char* const TexturePath, uint32_t* const Width, uint32_t* const Height, uint32_t* const Channels);
static void GCRendererTexture2D_CreateTextureImage(GCRendererTexture2D* const Texture2D, const char* const TexturePath);
static void GCRendererTexture2D_CreateTextureSampler(GCRendererTexture2D* const Texture2D);
static void GCRendererTexture2D_DestroyObjects(GCRendererTexture2D* const Texture2D);

GCRendererTexture2D* GCRendererTexture2D_Create(const GCRendererDevice* const Device, const GCRendererCommandList* const CommandList, const char* const TexturePath)
{
	GCRendererTexture2D* Texture2D = (GCRendererTexture2D*)GCMemory_Allocate(sizeof(GCRendererTexture2D));
	Texture2D->Device = Device;
	Texture2D->CommandList = CommandList;
	Texture2D->ImageHandle = VK_NULL_HANDLE;
	Texture2D->ImageMemoryHandle = VK_NULL_HANDLE;
	Texture2D->ImageViewHandle = VK_NULL_HANDLE;
	Texture2D->ImageSamplerHandle = VK_NULL_HANDLE;

	GCRendererTexture2D_CreateTextureImage(Texture2D, TexturePath);
	GCRendererTexture2D_CreateTextureSampler(Texture2D);

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

void GCRendererTexture2D_CreateTextureImage(GCRendererTexture2D* const Texture2D, const char* const TexturePath)
{
	const VkDevice DeviceHandle = GCRendererDevice_GetDeviceHandle(Texture2D->Device);

	uint32_t TextureWidth = 0, TextureHeight = 0, TextureChannels = 0;
	uint8_t* TextureData = GCRendererTexture2D_LoadImage(TexturePath, &TextureWidth, &TextureHeight, &TextureChannels);

	const size_t ImageSize = TextureWidth * TextureHeight * 4;

	VkBuffer StagingImageBufferHandle = VK_NULL_HANDLE;
	VkDeviceMemory StagingImageBufferMemoryHandle = VK_NULL_HANDLE;

	GCVulkanUtilities_CreateBuffer(Texture2D->Device, ImageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &StagingImageBufferHandle, &StagingImageBufferMemoryHandle);

	void* ImageData = NULL;
	vkMapMemory(DeviceHandle, StagingImageBufferMemoryHandle, 0, ImageSize, 0, &ImageData);
	memcpy(ImageData, TextureData, ImageSize);
	vkUnmapMemory(DeviceHandle, StagingImageBufferMemoryHandle);

	stbi_image_free(TextureData);

	GCVulkanUtilities_CreateImage(Texture2D->Device, TextureWidth, TextureHeight, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &Texture2D->ImageHandle, &Texture2D->ImageMemoryHandle);
	GCVulkanUtilities_TransitionImageLayout(Texture2D->Device, Texture2D->CommandList, Texture2D->ImageHandle, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	GCVulkanUtilities_CopyBufferToImage(Texture2D->Device, Texture2D->CommandList, StagingImageBufferHandle, Texture2D->ImageHandle, TextureWidth, TextureHeight);
	GCVulkanUtilities_TransitionImageLayout(Texture2D->Device, Texture2D->CommandList, Texture2D->ImageHandle, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	vkFreeMemory(DeviceHandle, StagingImageBufferMemoryHandle, NULL);
	vkDestroyBuffer(DeviceHandle, StagingImageBufferHandle, NULL);

	GCVulkanUtilities_CreateImageView(Texture2D->Device, Texture2D->ImageHandle, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT, &Texture2D->ImageViewHandle);
}

void GCRendererTexture2D_CreateTextureSampler(GCRendererTexture2D* const Texture2D)
{
	const VkDevice DeviceHandle = GCRendererDevice_GetDeviceHandle(Texture2D->Device);
	const GCRendererDeviceCapabilities DeviceCapabilities = GCRendererDevice_GetDeviceCapabilities(Texture2D->Device);

	VkSamplerCreateInfo SamplerInformation = { 0 };
	SamplerInformation.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	SamplerInformation.magFilter = VK_FILTER_LINEAR;
	SamplerInformation.minFilter = VK_FILTER_LINEAR;
	SamplerInformation.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	SamplerInformation.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	SamplerInformation.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	SamplerInformation.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	SamplerInformation.mipLodBias = 0.0f;

	if (DeviceCapabilities.IsAnisotropySupported)
	{
		SamplerInformation.anisotropyEnable = VK_TRUE;
		SamplerInformation.maxAnisotropy = DeviceCapabilities.MaximumAnisotropy;
	}
	else
	{
		SamplerInformation.anisotropyEnable = VK_FALSE;
		SamplerInformation.maxAnisotropy = 1.0f;
	}

	SamplerInformation.compareEnable = VK_FALSE;
	SamplerInformation.compareOp = VK_COMPARE_OP_ALWAYS;
	SamplerInformation.minLod = 0.0f;
	SamplerInformation.maxLod = 0.0f;
	SamplerInformation.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	SamplerInformation.unnormalizedCoordinates = VK_FALSE;

	GC_VULKAN_VALIDATE(vkCreateSampler(DeviceHandle, &SamplerInformation, NULL, &Texture2D->ImageSamplerHandle), "Failed to create a Vulkan texture sampler");
}

void GCRendererTexture2D_DestroyObjects(GCRendererTexture2D* const Texture2D)
{
	const VkDevice DeviceHandle = GCRendererDevice_GetDeviceHandle(Texture2D->Device);

	vkDestroySampler(DeviceHandle, Texture2D->ImageSamplerHandle, NULL);

	vkDestroyImageView(DeviceHandle, Texture2D->ImageViewHandle, NULL);

	vkFreeMemory(DeviceHandle, Texture2D->ImageMemoryHandle, NULL);
	vkDestroyImage(DeviceHandle, Texture2D->ImageHandle, NULL);
}