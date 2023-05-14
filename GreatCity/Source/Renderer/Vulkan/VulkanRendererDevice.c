#include "Renderer/RendererDevice.h"
#include "Renderer/Vulkan/VulkanUtilities.h"
#include "Core/Memory/Allocator.h"
#include "Core/Assert.h"

#include <string.h>
#include <stdbool.h>

#include <vulkan/vulkan.h>

#ifdef GC_PLATFORM_WINDOWS
	#define GC_VULKAN_PLATFORM_REQUIRED_EXTENSION_NAME "VK_KHR_win32_surface"
#endif

typedef struct GCRendererDevice
{
	VkInstance InstanceHandle;
	VkDebugUtilsMessengerEXT DebugMessengerHandle;
	VkSurfaceKHR SurfaceHandle;
	VkPhysicalDevice PhysicalDeviceHandle;
	VkDevice DeviceHandle;
	VkQueue GraphicsQueueHandle;
	VkQueue PresentQueueHandle;

	bool IsValidationLayerEnabled;
} GCRendererDevice;

typedef struct GCRendererDeviceQueueFamilyIndices
{
	uint32_t GraphicsFamily;
	bool GraphicsFamilyHasValue;
	uint32_t PresentFamily;
	bool PresentFamilyHasValue;
} GCRendererDeviceQueueFamilyIndices;

static bool GCRendererDevice_IsValidationLayerSupported(void);
static bool GCRendererDevice_IsDeviceSuitable(const VkPhysicalDevice PhysicalDeviceHandle, const VkSurfaceKHR SurfaceHandle);
static GCRendererDeviceQueueFamilyIndices GCRendererDevice_FindQueueFamilies(const VkPhysicalDevice PhysicalDeviceHandle, const VkSurfaceKHR SurfaceHandle);

static VkDebugUtilsMessengerCreateInfoEXT GCRendererDevice_InitializeDebugMessengerInformation(void);
static void GCRendererDevice_CreateInstance(GCRendererDevice* const Device);
static void GCRendererDevice_CreateDebugMessenger(GCRendererDevice* const Device);
extern void GCRendererDevice_CreateSurface(const VkInstance InstanceHandle, VkSurfaceKHR* SurfaceHandle);
static void GCRendererDevice_SelectPhysicalDevice(GCRendererDevice* const Device);
static void GCRendererDevice_CreateDevice(GCRendererDevice* const Device);
static VKAPI_ATTR VkBool32 VKAPI_CALL GCRendererDevice_DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT Severity, VkDebugUtilsMessageTypeFlagsEXT Type, const VkDebugUtilsMessengerCallbackDataEXT* CallbackData, void* UserData);

static VkResult GCRendererDevice_vkCreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pMessenger);
static void GCRendererDevice_vkDestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT messenger, const VkAllocationCallbacks* pAllocator);

GCRendererDevice* GCRendererDevice_Create(void)
{
	GCRendererDevice* Device = (GCRendererDevice*)GCMemory_Allocate(sizeof(GCRendererDevice));
	Device->InstanceHandle = VK_NULL_HANDLE;
	Device->DebugMessengerHandle = VK_NULL_HANDLE;
	Device->SurfaceHandle = VK_NULL_HANDLE;
	Device->PhysicalDeviceHandle = VK_NULL_HANDLE;
	Device->DeviceHandle = VK_NULL_HANDLE;
	Device->GraphicsQueueHandle = VK_NULL_HANDLE;
	Device->PresentQueueHandle = VK_NULL_HANDLE;

#ifndef GC_BUILD_TYPE_DISTRIBUTION
	Device->IsValidationLayerEnabled = true;
#else
	Device->IsValidationLayerEnabled = false;
#endif

	if (Device->IsValidationLayerEnabled && !GCRendererDevice_IsValidationLayerSupported())
	{
		GC_ASSERT_WITH_MESSAGE(false, "Vulkan validation layer is not supported");
	}

	GCRendererDevice_CreateInstance(Device);

	if (Device->IsValidationLayerEnabled)
	{
		GCRendererDevice_CreateDebugMessenger(Device);
	}

	GCRendererDevice_CreateSurface(Device->InstanceHandle, &Device->SurfaceHandle);
	GCRendererDevice_SelectPhysicalDevice(Device);
	GCRendererDevice_CreateDevice(Device);

	return Device;
}

void GCRendererDevice_Destroy(GCRendererDevice* Device)
{
	vkDestroyDevice(Device->DeviceHandle, NULL);
	vkDestroySurfaceKHR(Device->InstanceHandle, Device->SurfaceHandle, NULL);

	if (Device->IsValidationLayerEnabled)
	{
		GCRendererDevice_vkDestroyDebugUtilsMessengerEXT(Device->InstanceHandle, Device->DebugMessengerHandle, NULL);
	}

	vkDestroyInstance(Device->InstanceHandle, NULL);

	GCMemory_Free(Device);
}

bool GCRendererDevice_IsValidationLayerSupported(void)
{
	uint32_t LayerCount = 0;
	vkEnumerateInstanceLayerProperties(&LayerCount, NULL);

	VkLayerProperties* AvailableLayers = (VkLayerProperties*)GCMemory_Allocate(LayerCount * sizeof(VkLayerProperties));
	vkEnumerateInstanceLayerProperties(&LayerCount, AvailableLayers);

	bool IsLayerFound = false;

	for (uint32_t Counter = 0; Counter < LayerCount; Counter++)
	{
		if (!strcmp("VK_LAYER_KHRONOS_validation", AvailableLayers[Counter].layerName))
		{
			IsLayerFound = true;

			break;
		}
	}

	GCMemory_Free(AvailableLayers);

	return IsLayerFound;
}

bool GCRendererDevice_IsDeviceSuitable(const VkPhysicalDevice PhysicalDeviceHandle, const VkSurfaceKHR SurfaceHandle)
{
	const GCRendererDeviceQueueFamilyIndices QueueFamilyIndices = GCRendererDevice_FindQueueFamilies(PhysicalDeviceHandle, SurfaceHandle);

	return QueueFamilyIndices.GraphicsFamilyHasValue && QueueFamilyIndices.PresentFamilyHasValue;
}

GCRendererDeviceQueueFamilyIndices GCRendererDevice_FindQueueFamilies(const VkPhysicalDevice PhysicalDeviceHandle, const VkSurfaceKHR SurfaceHandle)
{
	GCRendererDeviceQueueFamilyIndices QueueFamilyIndices = { 0 };

	uint32_t QueueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(PhysicalDeviceHandle, &QueueFamilyCount, NULL);

	VkQueueFamilyProperties* QueueFamilies = (VkQueueFamilyProperties*)GCMemory_Allocate(QueueFamilyCount * sizeof(VkQueueFamilyProperties));
	vkGetPhysicalDeviceQueueFamilyProperties(PhysicalDeviceHandle, &QueueFamilyCount, QueueFamilies);

	for (uint32_t Counter = 0; Counter < QueueFamilyCount; Counter++)
	{
		if (QueueFamilies[Counter].queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			QueueFamilyIndices.GraphicsFamily = Counter;
			QueueFamilyIndices.GraphicsFamilyHasValue = true;

			VkBool32 PresentSupport = VK_FALSE;
			vkGetPhysicalDeviceSurfaceSupportKHR(PhysicalDeviceHandle, Counter, SurfaceHandle, &PresentSupport);

			if (PresentSupport)
			{
				QueueFamilyIndices.PresentFamily = Counter;
				QueueFamilyIndices.PresentFamilyHasValue = true;
			}

			break;
		}
	}

	GCMemory_Free(QueueFamilies);

	return QueueFamilyIndices;
}

VkDebugUtilsMessengerCreateInfoEXT GCRendererDevice_InitializeDebugMessengerInformation(void)
{
	VkDebugUtilsMessengerCreateInfoEXT DebugMessengerInformation = { 0 };
	DebugMessengerInformation.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	DebugMessengerInformation.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	DebugMessengerInformation.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	DebugMessengerInformation.pfnUserCallback = GCRendererDevice_DebugCallback;

	return DebugMessengerInformation;
}

void GCRendererDevice_CreateInstance(GCRendererDevice* const Device)
{
	VkApplicationInfo ApplicationInformation = { 0 };
	ApplicationInformation.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	ApplicationInformation.pApplicationName = "Great City";
	ApplicationInformation.applicationVersion = VK_MAKE_API_VERSION(0, 1, 0, 0);
	ApplicationInformation.pEngineName = "Great City Engine";
	ApplicationInformation.engineVersion = VK_MAKE_API_VERSION(0, 1, 0, 0);
	ApplicationInformation.apiVersion = VK_API_VERSION_1_0;

	VkInstanceCreateInfo InstanceInformation = { 0 };
	InstanceInformation.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	InstanceInformation.pApplicationInfo = &ApplicationInformation;

	VkDebugUtilsMessengerCreateInfoEXT DebugMessengerInformation = GCRendererDevice_InitializeDebugMessengerInformation();

	if (Device->IsValidationLayerEnabled)
	{
		InstanceInformation.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&DebugMessengerInformation;

		const char* const ValidationLayerName = "VK_LAYER_KHRONOS_validation";

		InstanceInformation.enabledLayerCount = 1;
		InstanceInformation.ppEnabledLayerNames = &ValidationLayerName;
	}

	const char* const RequiredExtensionNames[3] =
	{
		VK_KHR_SURFACE_EXTENSION_NAME,
		GC_VULKAN_PLATFORM_REQUIRED_EXTENSION_NAME,
		VK_EXT_DEBUG_UTILS_EXTENSION_NAME
	};

	InstanceInformation.enabledExtensionCount = !Device->IsValidationLayerEnabled ? 2 : 3;
	InstanceInformation.ppEnabledExtensionNames = RequiredExtensionNames;

	GC_VULKAN_VALIDATE(vkCreateInstance(&InstanceInformation, NULL, &Device->InstanceHandle), "Failed to create a Vulkan instance");
}

void GCRendererDevice_CreateDebugMessenger(GCRendererDevice* const Device)
{
	const VkDebugUtilsMessengerCreateInfoEXT DebugMessengerInformation = GCRendererDevice_InitializeDebugMessengerInformation();
	
	GC_VULKAN_VALIDATE(GCRendererDevice_vkCreateDebugUtilsMessengerEXT(Device->InstanceHandle, &DebugMessengerInformation, NULL, &Device->DebugMessengerHandle), "Failed to create a Vulkan debug messenger");
}

void GCRendererDevice_SelectPhysicalDevice(GCRendererDevice* const Device)
{
	uint32_t DeviceCount = 0;
	vkEnumeratePhysicalDevices(Device->InstanceHandle, &DeviceCount, NULL);

	if (!DeviceCount)
	{
		GC_ASSERT_WITH_MESSAGE(false, "Failed to find GPUs with Vulkan support");
	}

	VkPhysicalDevice* PhysicalDeviceHandles = (VkPhysicalDevice*)GCMemory_Allocate(DeviceCount * sizeof(VkPhysicalDevice));
	vkEnumeratePhysicalDevices(Device->InstanceHandle, &DeviceCount, PhysicalDeviceHandles);

	for (uint32_t Counter = 0; Counter < DeviceCount; Counter++)
	{
		if (GCRendererDevice_IsDeviceSuitable(PhysicalDeviceHandles[Counter], Device->SurfaceHandle))
		{
			Device->PhysicalDeviceHandle = PhysicalDeviceHandles[Counter];

			break;
		}
	}

	if (!Device->PhysicalDeviceHandle)
	{
		GC_ASSERT_WITH_MESSAGE(false, "Failed to find a suitable GPU");
	}

	GCMemory_Free(PhysicalDeviceHandles);
}

void GCRendererDevice_CreateDevice(GCRendererDevice* const Device)
{
	const GCRendererDeviceQueueFamilyIndices QueueFamilyIndices = GCRendererDevice_FindQueueFamilies(Device->PhysicalDeviceHandle, Device->SurfaceHandle);

	VkDeviceQueueCreateInfo DeviceQueueInformation[2] = { 0 };
	uint32_t DeviceQueueInformationCount = 0;

	if (QueueFamilyIndices.GraphicsFamily == QueueFamilyIndices.PresentFamily)
	{
		DeviceQueueInformation[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		DeviceQueueInformation[0].queueFamilyIndex = QueueFamilyIndices.GraphicsFamily;
		DeviceQueueInformation[0].queueCount = 1;

		const float QueuePriority = 1.0f;
		DeviceQueueInformation[0].pQueuePriorities = &QueuePriority;

		DeviceQueueInformationCount = 1;
	}
	else
	{
		const float QueuePriority = 1.0f;

		DeviceQueueInformation[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		DeviceQueueInformation[0].queueFamilyIndex = QueueFamilyIndices.GraphicsFamily;
		DeviceQueueInformation[0].queueCount = 1;
		DeviceQueueInformation[0].pQueuePriorities = &QueuePriority;

		DeviceQueueInformation[1].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		DeviceQueueInformation[1].queueFamilyIndex = QueueFamilyIndices.PresentFamily;
		DeviceQueueInformation[1].queueCount = 1;
		DeviceQueueInformation[1].pQueuePriorities = &QueuePriority;

		DeviceQueueInformationCount = 2;
	}

	VkPhysicalDeviceFeatures DeviceFeatures = { 0 };

	VkDeviceCreateInfo DeviceInformation = { 0 };
	DeviceInformation.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	DeviceInformation.queueCreateInfoCount = DeviceQueueInformationCount;
	DeviceInformation.pQueueCreateInfos = DeviceQueueInformation;

	if (Device->IsValidationLayerEnabled)
	{
		const char* const ValidationLayerName = "VK_LAYER_KHRONOS_validation";

		DeviceInformation.enabledLayerCount = 1;
		DeviceInformation.ppEnabledLayerNames = &ValidationLayerName;
	}

	DeviceInformation.pEnabledFeatures = &DeviceFeatures;

	GC_VULKAN_VALIDATE(vkCreateDevice(Device->PhysicalDeviceHandle, &DeviceInformation, NULL, &Device->DeviceHandle), "Failed to create a Vulkan device");

	vkGetDeviceQueue(Device->DeviceHandle, QueueFamilyIndices.GraphicsFamily, 0, &Device->GraphicsQueueHandle);
	vkGetDeviceQueue(Device->DeviceHandle, QueueFamilyIndices.PresentFamily, 0, &Device->PresentQueueHandle);
}

VKAPI_ATTR VkBool32 VKAPI_CALL GCRendererDevice_DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT Severity, VkDebugUtilsMessageTypeFlagsEXT Type, const VkDebugUtilsMessengerCallbackDataEXT* CallbackData, void* UserData)
{
	(void)Type;
	(void)UserData;

	switch (Severity)
	{
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
		{
			GC_LOG_TRACE("%s", CallbackData->pMessage);

			break;
		}
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
		{
			GC_LOG_WARNING("%s", CallbackData->pMessage);

			break;
		}
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
		{
			GC_ASSERT_WITH_MESSAGE(false, "%s", CallbackData->pMessage);

			break;
		}
	}

	return VK_FALSE;
}

VkResult GCRendererDevice_vkCreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pMessenger)
{
	const PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessengerEXTFunction = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");

	if (vkCreateDebugUtilsMessengerEXTFunction)
	{
		return vkCreateDebugUtilsMessengerEXTFunction(instance, pCreateInfo, pAllocator, pMessenger);
	}
	else
	{
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}

void GCRendererDevice_vkDestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT messenger, const VkAllocationCallbacks* pAllocator)
{
	const PFN_vkDestroyDebugUtilsMessengerEXT vkDestroyDebugUtilsMessengerEXTFunction = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");

	if (vkDestroyDebugUtilsMessengerEXTFunction)
	{
		vkDestroyDebugUtilsMessengerEXTFunction(instance, messenger, pAllocator);
	}
}