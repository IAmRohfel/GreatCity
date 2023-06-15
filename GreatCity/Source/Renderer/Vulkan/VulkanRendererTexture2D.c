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

#define _CRT_SECURE_NO_WARNINGS
#include "Renderer/Vulkan/VulkanRendererTexture2D.h"
#include "Core/Assert.h"
#include "Core/Log.h"
#include "Core/Memory/Allocator.h"
#include "Renderer/RendererDevice.h"
#include "Renderer/RendererTexture2D.h"
#include "Renderer/Vulkan/VulkanRendererCommandList.h"
#include "Renderer/Vulkan/VulkanRendererDevice.h"
#include "Renderer/Vulkan/VulkanUtilities.h"

#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include <png.h>
#include <vulkan/vulkan.h>

typedef struct GCRendererTexture2D
{
    const GCRendererDevice* Device;
    const GCRendererCommandList* CommandList;

    VkImage ImageHandle;
    VkDeviceMemory ImageMemoryHandle;
    VkImageView ImageViewHandle;
    VkSampler ImageSamplerHandle;
} GCRendererTexture2D;

static void GCRendererTexture2D_LoadPNG(const char* const TexturePath, uint32_t* const Width, uint32_t* const Height,
                                        uint32_t* const Channels, uint8_t** Data, uint8_t*** RowPointers);
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

void GCRendererTexture2D_LoadPNG(const char* const TexturePath, uint32_t* const Width, uint32_t* const Height,
                                 uint32_t* const Channels, uint8_t** Data, uint8_t*** RowPointers)
{
    FILE* TextureFile = fopen(TexturePath, "rb");

    if (TextureFile)
    {
        uint8_t PNGSignature[8] = {0};
        fread(PNGSignature, 1, 8, TextureFile);

        if (!png_check_sig(PNGSignature, 8))
        {
            GC_ASSERT_WITH_MESSAGE(false, "'%s': Invalid PNG file.");
        }

        png_structp PNGReadStruct = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
        png_infop PNGInfoStruct = png_create_info_struct(PNGReadStruct);

        png_init_io(PNGReadStruct, TextureFile);
        png_set_sig_bytes(PNGReadStruct, 8);
        png_read_info(PNGReadStruct, PNGInfoStruct);

        *Width = png_get_image_width(PNGReadStruct, PNGInfoStruct);
        *Height = png_get_image_height(PNGReadStruct, PNGInfoStruct);
        *Channels = png_get_channels(PNGReadStruct, PNGInfoStruct);
        const uint32_t BitDepth = png_get_bit_depth(PNGReadStruct, PNGInfoStruct);
        const uint32_t ColorType = png_get_color_type(PNGReadStruct, PNGInfoStruct);

        if (BitDepth == 16)
        {
            png_set_strip_16(PNGReadStruct);
        }

        if (ColorType == PNG_COLOR_TYPE_PALETTE)
        {
            png_set_palette_to_rgb(PNGReadStruct);
        }

        if (ColorType == PNG_COLOR_TYPE_GRAY && BitDepth < 8)
        {
            png_set_expand_gray_1_2_4_to_8(PNGReadStruct);
        }

        if (png_get_valid(PNGReadStruct, PNGInfoStruct, PNG_INFO_tRNS))
        {
            png_set_tRNS_to_alpha(PNGReadStruct);
        }

        if (ColorType == PNG_COLOR_TYPE_RGB || ColorType == PNG_COLOR_TYPE_GRAY || ColorType == PNG_COLOR_TYPE_PALETTE)
        {
            png_set_filler(PNGReadStruct, 0xff, PNG_FILLER_AFTER);
        }

        if (ColorType == PNG_COLOR_TYPE_GRAY || ColorType == PNG_COLOR_TYPE_GRAY_ALPHA)
        {
            png_set_gray_to_rgb(PNGReadStruct);
        }

        png_read_update_info(PNGReadStruct, PNGInfoStruct);

        const size_t RowBytes = png_get_rowbytes(PNGReadStruct, PNGInfoStruct);
        *Data = (uint8_t*)GCMemory_Allocate(RowBytes * *Height * sizeof(uint8_t));
        *RowPointers = (uint8_t**)GCMemory_Allocate(*Height * sizeof(uint8_t*));

        for (uint32_t Counter = 0; Counter < *Height; Counter++)
        {
            (*RowPointers)[*Height - 1 - Counter] = *Data + Counter * RowBytes;
        }

        png_read_image(PNGReadStruct, *RowPointers);

        fclose(TextureFile);

        png_destroy_read_struct(&PNGReadStruct, &PNGInfoStruct, NULL);
    }
}

void GCRendererTexture2D_CreateTexture(GCRendererTexture2D* const Texture2D, const char* const TexturePath)
{
    const VkDevice DeviceHandle = GCRendererDevice_GetDeviceHandle(Texture2D->Device);

    uint32_t TextureWidth = 0, TextureHeight = 0, TextureChannels = 0;
    uint8_t* TextureData = NULL;
    uint8_t** TextureRowPointers = NULL;

    GCRendererTexture2D_LoadPNG(TexturePath, &TextureWidth, &TextureHeight, &TextureChannels, &TextureData,
                                &TextureRowPointers);

    const size_t ImageSize = TextureWidth * TextureHeight * TextureChannels;
    const uint32_t MipLevels = (uint32_t)floorf(log2f(fmaxf((float)TextureWidth, (float)TextureHeight))) + 1;

    VkBuffer StagingImageBufferHandle = VK_NULL_HANDLE;
    VkDeviceMemory StagingImageBufferMemoryHandle = VK_NULL_HANDLE;

    GCVulkanUtilities_CreateBuffer(Texture2D->Device, ImageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                   &StagingImageBufferHandle, &StagingImageBufferMemoryHandle);

    void* ImageData = NULL;
    vkMapMemory(DeviceHandle, StagingImageBufferMemoryHandle, 0, ImageSize, 0, &ImageData);
    memcpy(ImageData, TextureData, ImageSize);
    vkUnmapMemory(DeviceHandle, StagingImageBufferMemoryHandle);

    GCVulkanUtilities_CreateImage(
        Texture2D->Device, TextureWidth, TextureHeight, MipLevels, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL,
        VK_SAMPLE_COUNT_1_BIT,
        VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &Texture2D->ImageHandle, &Texture2D->ImageMemoryHandle);

    const VkCommandBuffer CommandBufferHandle = GCRendererCommandList_BeginSingleTimeCommands(Texture2D->CommandList);
    GCVulkanUtilities_TransitionImageLayout(CommandBufferHandle, Texture2D->ImageHandle, MipLevels,
                                            VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    GCVulkanUtilities_CopyBufferToImage(CommandBufferHandle, StagingImageBufferHandle, Texture2D->ImageHandle,
                                        TextureWidth, TextureHeight);
    GCVulkanUtilities_GenerateMipmap(Texture2D->Device, CommandBufferHandle, Texture2D->ImageHandle, TextureWidth,
                                     TextureHeight, MipLevels, VK_FORMAT_R8G8B8A8_SRGB);
    GCRendererCommandList_EndSingleTimeCommands(Texture2D->CommandList, CommandBufferHandle);

    vkFreeMemory(DeviceHandle, StagingImageBufferMemoryHandle, NULL);
    vkDestroyBuffer(DeviceHandle, StagingImageBufferHandle, NULL);

    GCVulkanUtilities_CreateImageView(Texture2D->Device, Texture2D->ImageHandle, VK_FORMAT_R8G8B8A8_SRGB,
                                      VK_IMAGE_ASPECT_COLOR_BIT, MipLevels, &Texture2D->ImageViewHandle);
    GCVulkanUtilities_CreateSampler(Texture2D->Device, VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_REPEAT, MipLevels,
                                    &Texture2D->ImageSamplerHandle);

    GCMemory_Free(TextureRowPointers);
    GCMemory_Free(TextureData);
}

void GCRendererTexture2D_DestroyObjects(GCRendererTexture2D* const Texture2D)
{
    const VkDevice DeviceHandle = GCRendererDevice_GetDeviceHandle(Texture2D->Device);

    vkDestroySampler(DeviceHandle, Texture2D->ImageSamplerHandle, NULL);

    vkDestroyImageView(DeviceHandle, Texture2D->ImageViewHandle, NULL);

    vkFreeMemory(DeviceHandle, Texture2D->ImageMemoryHandle, NULL);
    vkDestroyImage(DeviceHandle, Texture2D->ImageHandle, NULL);
}