#include "Renderer/RendererDevice.h"
#include "Core/Memory/Allocator.h"
#include "Core/Assert.h"

#include <stdbool.h>

#include <vulkan/vulkan.h>

#ifdef GC_PLATFORM_WINDOWS
	#define GC_VULKAN_PLATFORM_REQUIRED_EXTENSION_NAME "VK_KHR_win32_surface"
#endif

typedef struct GCRendererDevice
{
	VkInstance InstanceHandle;
} GCRendererDevice;

GCRendererDevice* GCRendererDevice_Create(void)
{
	GCRendererDevice* Device = (GCRendererDevice*)GCMemory_Allocate(sizeof(GCRendererDevice));
	Device->InstanceHandle = VK_NULL_HANDLE;

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

	const char* const RequiredExtensionNames[2] =
	{
		VK_KHR_SURFACE_EXTENSION_NAME,
		GC_VULKAN_PLATFORM_REQUIRED_EXTENSION_NAME
	};

	InstanceInformation.enabledExtensionCount = 2;
	InstanceInformation.ppEnabledExtensionNames = RequiredExtensionNames;

	if (vkCreateInstance(&InstanceInformation, NULL, &Device->InstanceHandle) != VK_SUCCESS)
	{
		GC_ASSERT_WITH_MESSAGE(false, "Failed to create a Vulkan instance");
	}

	return Device;
}

void GCRendererDevice_Destroy(GCRendererDevice* Device)
{
	vkDestroyInstance(Device->InstanceHandle, NULL);

	GCMemory_Free(Device);
}