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

#include "Renderer/Vulkan/VulkanRendererSwapChain.h"
#include "Renderer/Vulkan/VulkanRendererDevice.h"
#include "Renderer/Vulkan/VulkanUtilities.h"
#include "Renderer/RendererSwapChain.h"
#include "Renderer/RendererDevice.h"
#include "Core/Memory/Allocator.h"
#include "ApplicationCore/GenericPlatform/Window.h"
#include "ApplicationCore/Application.h"
#include "Core/Log.h"
#include "Core/Assert.h"

#include <limits.h>
#include <stdbool.h>

#include <vulkan/vulkan.h>

typedef struct GCRendererSwapChain
{
	const GCRendererDevice* Device;

	VkSurfaceCapabilitiesKHR SurfaceCapabilities;
	VkExtent2D Extent;
	VkSurfaceFormatKHR SurfaceFormat;
	VkPresentModeKHR PresentMode;
	uint32_t ImageCount;

	VkSwapchainKHR SwapChainHandle;
	VkImage* ImageHandles;
	VkImageView* ImageViewHandles;
} GCRendererSwapChain;

static void GCRendererSwapChain_QuerySwapChainSupport(GCRendererSwapChain* const SwapChain);
static void GCRendererSwapChain_SelectExtent(GCRendererSwapChain* const SwapChain);
static void GCRendererSwapChain_SelectSurfaceFormat(GCRendererSwapChain* const SwapChain, const VkSurfaceFormatKHR* const Formats, const uint32_t FormatCount);
static void GCRendererSwapChain_SelectPresentMode(GCRendererSwapChain* const SwapChain, const VkPresentModeKHR* const PresentModes, const uint32_t PresentModeCount);
static void GCRendererSwapChain_CreateSwapChain(GCRendererSwapChain* const SwapChain);
static void GCRendererSwapChain_CreateImageViews(GCRendererSwapChain* const SwapChain);
static void GCRendererSwapChain_DestroyObjects(GCRendererSwapChain* const SwapChain);

static void GCRendererSwapChain_ClampExtent(VkExtent2D* const Extent, const VkSurfaceCapabilitiesKHR* const SurfaceCapabilities);
static VkFormat GCRendererSwapChain_GetSupportedFormat(const GCRendererSwapChain* const SwapChain, const VkFormat* const Formats, const uint32_t FormatCount, const VkImageTiling Tiling, const VkFormatFeatureFlags FormatFeature);

GCRendererSwapChain* GCRendererSwapChain_Create(const GCRendererDevice* const Device)
{
	GCRendererSwapChain* SwapChain = (GCRendererSwapChain*)GCMemory_Allocate(sizeof(GCRendererSwapChain));
	SwapChain->Device = Device;
	SwapChain->SurfaceCapabilities = (VkSurfaceCapabilitiesKHR){ 0 };
	SwapChain->Extent = (VkExtent2D){ 0 };
	SwapChain->SurfaceFormat = (VkSurfaceFormatKHR){ 0 };
	SwapChain->PresentMode = (VkPresentModeKHR)0;
	SwapChain->ImageCount = 0;
	SwapChain->SwapChainHandle = VK_NULL_HANDLE;
	SwapChain->ImageHandles = NULL;
	SwapChain->ImageViewHandles = NULL;

	GCRendererSwapChain_QuerySwapChainSupport(SwapChain);
	GCRendererSwapChain_CreateSwapChain(SwapChain);
	GCRendererSwapChain_CreateImageViews(SwapChain);

	return SwapChain;
}

void GCRendererSwapChain_Recreate(GCRendererSwapChain* const SwapChain)
{
	GCRendererDevice_WaitIdle(SwapChain->Device);

	GCRendererSwapChain_DestroyObjects(SwapChain);

	GCRendererSwapChain_QuerySwapChainSupport(SwapChain);
	GCRendererSwapChain_CreateSwapChain(SwapChain);
	GCRendererSwapChain_CreateImageViews(SwapChain);
}

void GCRendererSwapChain_Destroy(GCRendererSwapChain* SwapChain)
{
	GCRendererDevice_WaitIdle(SwapChain->Device);

	GCRendererSwapChain_DestroyObjects(SwapChain);

	GCMemory_Free(SwapChain);
}

bool GCRendererSwapChain_IsSwapChainSupported(const VkPhysicalDevice PhysicalDeviceHandle, const VkSurfaceKHR SurfaceHandle)
{
	uint32_t FormatCount = 0;
	vkGetPhysicalDeviceSurfaceFormatsKHR(PhysicalDeviceHandle, SurfaceHandle, &FormatCount, NULL);

	uint32_t PresentModeCount = 0;
	vkGetPhysicalDeviceSurfacePresentModesKHR(PhysicalDeviceHandle, SurfaceHandle, &PresentModeCount, NULL);

	return FormatCount && PresentModeCount;
}

VkFormat GCRendererSwapChain_GetFormat(const GCRendererSwapChain* const SwapChain)
{
	return SwapChain->SurfaceFormat.format;
}

VkExtent2D GCRendererSwapChain_GetExtent(const GCRendererSwapChain* const SwapChain)
{
	return SwapChain->Extent;
}

VkSwapchainKHR GCRendererSwapChain_GetHandle(const GCRendererSwapChain* const SwapChain)
{
	return SwapChain->SwapChainHandle;
}

VkImageView* GCRendererSwapChain_GetImageViewHandles(const GCRendererSwapChain* const SwapChain)
{
	return SwapChain->ImageViewHandles;
}

uint32_t GCRendererSwapChain_GetImageCount(const GCRendererSwapChain* const SwapChain)
{
	return SwapChain->ImageCount;
}

VkFormat GCRendererSwapChain_GetDepthFormat(const GCRendererSwapChain* const SwapChain)
{
	const VkFormat Formats[3] = { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT };
	return GCRendererSwapChain_GetSupportedFormat(SwapChain, Formats, sizeof(Formats) / sizeof(VkFormat), VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
}

void GCRendererSwapChain_QuerySwapChainSupport(GCRendererSwapChain* const SwapChain)
{
	const VkPhysicalDevice PhysicalDeviceHandle = GCRendererDevice_GetPhysicalDeviceHandle(SwapChain->Device);
	const VkSurfaceKHR SurfaceHandle = GCRendererDevice_GetSurfaceHandle(SwapChain->Device);

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(PhysicalDeviceHandle, SurfaceHandle, &SwapChain->SurfaceCapabilities);

	uint32_t FormatCount = 0;
	vkGetPhysicalDeviceSurfaceFormatsKHR(PhysicalDeviceHandle, SurfaceHandle, &FormatCount, NULL);

	VkSurfaceFormatKHR* Formats = (VkSurfaceFormatKHR*)GCMemory_Allocate(FormatCount * sizeof(VkSurfaceFormatKHR));
	vkGetPhysicalDeviceSurfaceFormatsKHR(PhysicalDeviceHandle, SurfaceHandle, &FormatCount, Formats);

	uint32_t PresentModeCount = 0;
	vkGetPhysicalDeviceSurfacePresentModesKHR(PhysicalDeviceHandle, SurfaceHandle, &PresentModeCount, NULL);

	VkPresentModeKHR* PresentModes = (VkPresentModeKHR*)GCMemory_Allocate(PresentModeCount * sizeof(VkPresentModeKHR));
	vkGetPhysicalDeviceSurfacePresentModesKHR(PhysicalDeviceHandle, SurfaceHandle, &PresentModeCount, PresentModes);

	GCRendererSwapChain_SelectExtent(SwapChain);
	GCRendererSwapChain_SelectSurfaceFormat(SwapChain, Formats, FormatCount);
	GCRendererSwapChain_SelectPresentMode(SwapChain, PresentModes, PresentModeCount);

	GCMemory_Free(PresentModes);
	GCMemory_Free(Formats);
}

void GCRendererSwapChain_SelectExtent(GCRendererSwapChain* const SwapChain)
{
	const VkSurfaceCapabilitiesKHR SurfaceCapabilities = SwapChain->SurfaceCapabilities;

	if (SurfaceCapabilities.currentExtent.width != UINT32_MAX)
	{
		SwapChain->Extent = SurfaceCapabilities.currentExtent;
	}
	else
	{
		uint32_t Width = 0, Height = 0;
		GCWindow_GetWindowSize(GCApplication_GetWindow(), &Width, &Height);

		VkExtent2D Extent = { 0 };
		Extent.width = Width;
		Extent.height = Height;

		GCRendererSwapChain_ClampExtent(&Extent, &SurfaceCapabilities);

		SwapChain->Extent = Extent;
	}
}

void GCRendererSwapChain_SelectSurfaceFormat(GCRendererSwapChain* const SwapChain, const VkSurfaceFormatKHR* const Formats, const uint32_t FormatCount)
{
	for (uint32_t Counter = 0; Counter < FormatCount; Counter++)
	{
		if (Formats[Counter].format == VK_FORMAT_B8G8R8A8_SRGB && Formats[Counter].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
		{
			SwapChain->SurfaceFormat = Formats[Counter];

			return;
		}
	}

	SwapChain->SurfaceFormat = Formats[0];
}

void GCRendererSwapChain_SelectPresentMode(GCRendererSwapChain* const SwapChain, const VkPresentModeKHR* const PresentModes, const uint32_t PresentModeCount)
{
	for (uint32_t Counter = 0; Counter < PresentModeCount; Counter++)
	{
		if (PresentModes[Counter] == VK_PRESENT_MODE_MAILBOX_KHR)
		{
			SwapChain->PresentMode = PresentModes[Counter];

			return;
		}
	}

	SwapChain->PresentMode = VK_PRESENT_MODE_FIFO_KHR;
}

void GCRendererSwapChain_CreateSwapChain(GCRendererSwapChain* const SwapChain)
{
	const VkDevice DeviceHandle = GCRendererDevice_GetDeviceHandle(SwapChain->Device);
	const VkSurfaceKHR SurfaceHandle = GCRendererDevice_GetSurfaceHandle(SwapChain->Device);

	SwapChain->ImageCount = SwapChain->SurfaceCapabilities.minImageCount + 1;

	if (SwapChain->SurfaceCapabilities.maxImageCount > 0 && SwapChain->ImageCount > SwapChain->SurfaceCapabilities.maxImageCount)
	{
		SwapChain->ImageCount = SwapChain->SurfaceCapabilities.maxImageCount;
	}

	VkSwapchainCreateInfoKHR SwapChainInformation = { 0 };
	SwapChainInformation.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	SwapChainInformation.surface = SurfaceHandle;
	SwapChainInformation.minImageCount = SwapChain->ImageCount;
	SwapChainInformation.imageFormat = SwapChain->SurfaceFormat.format;
	SwapChainInformation.imageColorSpace = SwapChain->SurfaceFormat.colorSpace;
	SwapChainInformation.imageExtent = SwapChain->Extent;
	SwapChainInformation.imageArrayLayers = 1;
	SwapChainInformation.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	const uint32_t GraphicsFamilyQueueIndex = GCRendererDevice_GetGraphicsFamilyQueueIndex(SwapChain->Device);
	const uint32_t PresentFamilyQueueIndex = GCRendererDevice_GetPresentFamilyQueueIndex(SwapChain->Device);
	const uint32_t QueueFamilyIndices[2] = { GraphicsFamilyQueueIndex, PresentFamilyQueueIndex };

	if (GraphicsFamilyQueueIndex != PresentFamilyQueueIndex)
	{
		SwapChainInformation.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		SwapChainInformation.queueFamilyIndexCount = 2;
		SwapChainInformation.pQueueFamilyIndices = QueueFamilyIndices;
	}
	else
	{
		SwapChainInformation.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	}

	SwapChainInformation.preTransform = SwapChain->SurfaceCapabilities.currentTransform;
	SwapChainInformation.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	SwapChainInformation.presentMode = SwapChain->PresentMode;
	SwapChainInformation.clipped = VK_TRUE;
	SwapChainInformation.oldSwapchain = VK_NULL_HANDLE;

	GC_VULKAN_VALIDATE(vkCreateSwapchainKHR(DeviceHandle, &SwapChainInformation, NULL, &SwapChain->SwapChainHandle), "Failed to create a Vulkan swap chain");

	GC_VULKAN_VALIDATE(vkGetSwapchainImagesKHR(DeviceHandle, SwapChain->SwapChainHandle, &SwapChain->ImageCount, NULL), "Failed to get Vulkan swap chain images");
	SwapChain->ImageHandles = (VkImage*)GCMemory_Allocate(SwapChain->ImageCount * sizeof(VkImage));
	GC_VULKAN_VALIDATE(vkGetSwapchainImagesKHR(DeviceHandle, SwapChain->SwapChainHandle, &SwapChain->ImageCount, SwapChain->ImageHandles), "Failed to get Vulkan swap chain images");
}

void GCRendererSwapChain_CreateImageViews(GCRendererSwapChain* const SwapChain)
{
	SwapChain->ImageViewHandles = (VkImageView*)GCMemory_Allocate(SwapChain->ImageCount * sizeof(VkImageView));

	for (uint32_t Counter = 0; Counter < SwapChain->ImageCount; Counter++)
	{
		GCVulkanUtilities_CreateImageView(SwapChain->Device, SwapChain->ImageHandles[Counter], SwapChain->SurfaceFormat.format, VK_IMAGE_ASPECT_COLOR_BIT, 1, &SwapChain->ImageViewHandles[Counter]);
	}
}

void GCRendererSwapChain_DestroyObjects(GCRendererSwapChain* const SwapChain)
{
	const VkDevice DeviceHandle = GCRendererDevice_GetDeviceHandle(SwapChain->Device);

	for (uint32_t Counter = 0; Counter < SwapChain->ImageCount; Counter++)
	{
		vkDestroyImageView(DeviceHandle, SwapChain->ImageViewHandles[Counter], NULL);
	}

	vkDestroySwapchainKHR(DeviceHandle, SwapChain->SwapChainHandle, NULL);

	GCMemory_Free(SwapChain->ImageViewHandles);
	GCMemory_Free(SwapChain->ImageHandles);
}

void GCRendererSwapChain_ClampExtent(VkExtent2D* const Extent, const VkSurfaceCapabilitiesKHR* const SurfaceCapabilities)
{
	uint32_t* const Width = &Extent->width;
	uint32_t* const Height = &Extent->height;

	if (*Width < SurfaceCapabilities->minImageExtent.width)
	{
		*Width = SurfaceCapabilities->minImageExtent.width;
	}
	else if (*Width > SurfaceCapabilities->maxImageExtent.width)
	{
		*Width = SurfaceCapabilities->maxImageExtent.width;
	}

	if (*Height < SurfaceCapabilities->minImageExtent.height)
	{
		*Height = SurfaceCapabilities->minImageExtent.height;
	}
	else if (*Height > SurfaceCapabilities->maxImageExtent.height)
	{
		*Height = SurfaceCapabilities->maxImageExtent.height;
	}
}

VkFormat GCRendererSwapChain_GetSupportedFormat(const GCRendererSwapChain* const SwapChain, const VkFormat* const Formats, const uint32_t FormatCount, const VkImageTiling Tiling, const VkFormatFeatureFlags FormatFeature)
{
	const VkPhysicalDevice PhysicalDeviceHandle = GCRendererDevice_GetPhysicalDeviceHandle(SwapChain->Device);

	for (uint32_t Counter = 0; Counter < FormatCount; Counter++)
	{
		VkFormatProperties FormatProperties = { 0 };
		vkGetPhysicalDeviceFormatProperties(PhysicalDeviceHandle, Formats[Counter], &FormatProperties);

		if (Tiling == VK_IMAGE_TILING_LINEAR && (FormatProperties.linearTilingFeatures & FormatFeature) == FormatFeature)
		{
			return Formats[Counter];
		}
		else if (Tiling == VK_IMAGE_TILING_OPTIMAL && (FormatProperties.optimalTilingFeatures & FormatFeature) == FormatFeature)
		{
			return Formats[Counter];
		}
	}

	GC_ASSERT_WITH_MESSAGE(false, "Failed to find a supported Vulkan format");
	return VK_FORMAT_UNDEFINED;
}