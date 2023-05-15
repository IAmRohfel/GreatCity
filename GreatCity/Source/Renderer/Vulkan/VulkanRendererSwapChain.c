#include "Renderer/Vulkan/VulkanUtilities.h"
#include "Renderer/RendererSwapChain.h"
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
	VkSurfaceFormatKHR SurfaceFormat;
	VkPresentModeKHR PresentMode;
	VkExtent2D Extent;

	VkSwapchainKHR SwapChainHandle;
	VkImage* SwapChainImageHandles;
} GCRendererSwapChain;

bool GCRendererSwapChain_IsSwapChainSupported(const VkPhysicalDevice PhysicalDeviceHandle, const VkSurfaceKHR SurfaceHandle);

extern VkSurfaceKHR GCRendererDevice_GetSurfaceHandle(const GCRendererDevice* const Device);
extern VkPhysicalDevice GCRendererDevice_GetPhysicalDeviceHandle(const GCRendererDevice* const Device);
extern VkDevice GCRendererDevice_GetDeviceHandle(const GCRendererDevice* const Device);
extern uint32_t GCRendererDevice_GetGraphicsFamilyQueueIndex(const GCRendererDevice* const Device);
extern uint32_t GCRendererDevice_GetPresentFamilyQueueIndex(const GCRendererDevice* const Device);

static void GCRendererSwapChain_QuerySwapChainSupport(GCRendererSwapChain* const SwapChain);
static void GCRendererSwapChain_SelectExtent(GCRendererSwapChain* const SwapChain);
static void GCRendererSwapChain_SelectSurfaceFormat(GCRendererSwapChain* const SwapChain, const VkSurfaceFormatKHR* const Formats, const uint32_t FormatCount);
static void GCRendererSwapChain_SelectPresentMode(GCRendererSwapChain* const SwapChain, const VkPresentModeKHR* const PresentModes, const uint32_t PresentModeCount);
static void GCRendererSwapChain_CreateSwapChain(GCRendererSwapChain* const SwapChain);

static void GCRendererSwapChain_ClampExtent(VkExtent2D* const Extent, const VkSurfaceCapabilitiesKHR* const SurfaceCapabilities);

GCRendererSwapChain* GCRendererSwapChain_Create(const GCRendererDevice* const Device)
{
	GCRendererSwapChain* SwapChain = (GCRendererSwapChain*)GCMemory_Allocate(sizeof(GCRendererSwapChain));
	SwapChain->Device = Device;
	SwapChain->SurfaceCapabilities = (VkSurfaceCapabilitiesKHR){ 0 };
	SwapChain->SurfaceFormat = (VkSurfaceFormatKHR){ 0 };
	SwapChain->PresentMode = (VkPresentModeKHR)0;
	SwapChain->Extent = (VkExtent2D){ 0 };
	SwapChain->SwapChainHandle = VK_NULL_HANDLE;
	SwapChain->SwapChainImageHandles = NULL;

	GCRendererSwapChain_QuerySwapChainSupport(SwapChain);
	GCRendererSwapChain_CreateSwapChain(SwapChain);

	return SwapChain;
}

void GCRendererSwapChain_Destroy(GCRendererSwapChain* SwapChain)
{
	const VkDevice DeviceHandle = GCRendererDevice_GetDeviceHandle(SwapChain->Device);

	vkDestroySwapchainKHR(DeviceHandle, SwapChain->SwapChainHandle, NULL);

	GCMemory_Free(SwapChain->SwapChainImageHandles);
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

	uint32_t ImageCount = SwapChain->SurfaceCapabilities.minImageCount + 1;

	if (SwapChain->SurfaceCapabilities.maxImageCount > 0 && ImageCount > SwapChain->SurfaceCapabilities.maxImageCount)
	{
		ImageCount = SwapChain->SurfaceCapabilities.maxImageCount;
	}

	VkSwapchainCreateInfoKHR SwapChainInformation = { 0 };
	SwapChainInformation.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	SwapChainInformation.surface = SurfaceHandle;
	SwapChainInformation.minImageCount = ImageCount;
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

	GC_VULKAN_VALIDATE(vkGetSwapchainImagesKHR(DeviceHandle, SwapChain->SwapChainHandle, &ImageCount, NULL), "Failed to get Vulkan swap chain images");
	SwapChain->SwapChainImageHandles = (VkImage*)GCMemory_Allocate(ImageCount * sizeof(VkImage));
	GC_VULKAN_VALIDATE(vkGetSwapchainImagesKHR(DeviceHandle, SwapChain->SwapChainHandle, &ImageCount, SwapChain->SwapChainImageHandles), "Failed to get Vulkan swap chain images");
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