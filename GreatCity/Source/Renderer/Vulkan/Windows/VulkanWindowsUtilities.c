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

#define VK_USE_PLATFORM_WIN32_KHR
#include "ApplicationCore/Application.h"
#include "Core/Assert.h"
#include "Core/Log.h"
#include "Renderer/Vulkan/VulkanUtilities.h"

#include <stdbool.h>

#include <Windows.h>

#include <vulkan/vulkan.h>

typedef struct GCPlatformWindow GCWindowsWindow;

extern HWND GCWindowsWindow_GetWindowHandle(const GCWindowsWindow* const Window);
extern HINSTANCE GCWindowsWindow_GetInstanceHandle(const GCWindowsWindow* const Window);

void GCRendererDevice_CreateSurface(const VkInstance InstanceHandle, VkSurfaceKHR* SurfaceHandle)
{
    VkWin32SurfaceCreateInfoKHR SurfaceInformation = {0};
    SurfaceInformation.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    SurfaceInformation.hinstance = GCWindowsWindow_GetInstanceHandle(GCApplication_GetWindow());
    SurfaceInformation.hwnd = GCWindowsWindow_GetWindowHandle(GCApplication_GetWindow());

    GC_VULKAN_VALIDATE(vkCreateWin32SurfaceKHR(InstanceHandle, &SurfaceInformation, NULL, SurfaceHandle),
                       "Failed to create a Win32 Vulkan surface");
}