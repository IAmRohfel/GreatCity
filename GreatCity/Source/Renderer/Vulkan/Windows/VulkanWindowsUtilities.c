#include "Renderer/Vulkan/VulkanUtilities.h"
#include "ApplicationCore/Application.h"
#include "Core/Log.h"
#include "Core/Assert.h"

#include <stdbool.h>

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>

typedef struct GCPlatformWindow GCWindowsWindow;

extern HWND GCWindowsWindow_GetWindowHandle(const GCWindowsWindow* const Window);
extern HINSTANCE GCWindowsWindow_GetInstanceHandle(const GCWindowsWindow* const Window);

void GCRendererDevice_CreateSurface(const VkInstance InstanceHandle, VkSurfaceKHR* SurfaceHandle)
{
	VkWin32SurfaceCreateInfoKHR SurfaceInformation = { 0 };
	SurfaceInformation.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	SurfaceInformation.hinstance = GCWindowsWindow_GetInstanceHandle(GCApplication_GetWindow());
	SurfaceInformation.hwnd = GCWindowsWindow_GetWindowHandle(GCApplication_GetWindow());

	GC_VULKAN_VALIDATE(vkCreateWin32SurfaceKHR(InstanceHandle, &SurfaceInformation, NULL, SurfaceHandle), "Failed to create a Win32 Vulkan surface");
}