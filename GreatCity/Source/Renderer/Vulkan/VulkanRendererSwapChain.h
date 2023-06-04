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

#ifndef GC_RENDERER_VULKAN_VULKAN_RENDERER_SWAP_CHAIN_H
#define GC_RENDERER_VULKAN_VULKAN_RENDERER_SWAP_CHAIN_H

#include <stdint.h>
#include <stdbool.h>

#include <vulkan/vulkan.h>

#ifdef __cplusplus
extern "C"
{
#endif

	typedef struct GCRendererSwapChain GCRendererSwapChain;

	bool GCRendererSwapChain_IsSwapChainSupported(const VkPhysicalDevice PhysicalDeviceHandle, const VkSurfaceKHR SurfaceHandle);
	VkFormat GCRendererSwapChain_GetFormat(const GCRendererSwapChain* const SwapChain);
	VkExtent2D GCRendererSwapChain_GetExtent(const GCRendererSwapChain* const SwapChain);
	VkSwapchainKHR GCRendererSwapChain_GetHandle(const GCRendererSwapChain* const SwapChain);
	VkImageView* GCRendererSwapChain_GetImageViewHandles(const GCRendererSwapChain* const SwapChain);
	uint32_t GCRendererSwapChain_GetImageCount(const GCRendererSwapChain* const SwapChain);
	VkFormat GCRendererSwapChain_GetDepthFormat(const GCRendererSwapChain* const SwapChain);
	VkSampleCountFlagBits GCRendererSwapChain_GetMaximumUsableSampleCount(const GCRendererSwapChain* const SwapChain);

#ifdef __cplusplus
}
#endif

#endif