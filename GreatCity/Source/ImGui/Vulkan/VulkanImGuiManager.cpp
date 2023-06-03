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

#include "Renderer/Vulkan/VulkanUtilities.h"
#include "Renderer/Renderer.h"
#include "Renderer/RendererDevice.h"
#include "Renderer/RendererSwapChain.h"
#include "Renderer/RendererCommandList.h"
#include "Renderer/RendererGraphicsPipeline.h"
#include "Core/Memory/Allocator.h"
#include "Core/Log.h"
#include "Core/Assert.h"

#include <array>

#include <vulkan/vulkan.h>
#include <imgui.h>
#include <imgui_impl_vulkan.h>

extern "C" VkInstance GCRendererDevice_GetInstanceHandle(const GCRendererDevice* const Device);
extern "C" VkPhysicalDevice GCRendererDevice_GetPhysicalDeviceHandle(const GCRendererDevice* const Device);
extern "C" VkDevice GCRendererDevice_GetDeviceHandle(const GCRendererDevice* const Device);
extern "C" uint32_t GCRendererDevice_GetGraphicsFamilyQueueIndex(const GCRendererDevice* const Device);
extern "C" VkQueue GCRendererDevice_GetGraphicsQueueHandle(const GCRendererDevice* const Device);
extern "C" uint32_t GCRendererSwapChain_GetImageCount(const GCRendererSwapChain* const SwapChain);
extern "C" VkCommandBuffer GCRendererCommandList_GetCurrentFrameCommandBufferHandle(const GCRendererCommandList* const CommandList);
extern "C" VkRenderPass GCRendererGraphicsPipeline_GetSwapChainRenderPassHandle(const GCRendererGraphicsPipeline* const GraphicsPipeline);
extern "C" VkImageView GCRendererFramebuffer_GetTextureImageViewHandle(const GCRendererFramebuffer* const Framebuffer);
extern "C" VkSampler GCRendererFramebuffer_GetTextureSamplerHandle(const GCRendererFramebuffer* const Framebuffer);

static VkDescriptorPool ImGuiDescriptorPoolHandle = VK_NULL_HANDLE;
static VkDescriptorSet ImGuiDescriptorSetHandle = VK_NULL_HANDLE;

static void GCImGuiManager_CreateDescriptorPool(void);
static void GCImGuiManager_UpdateDescriptorSet(void);
static void GCImGuiManager_CheckVulkanResult(VkResult Result);

extern "C" void GCImGuiManager_InitializeRenderer(void)
{
	const GCRendererDevice* const RendererDevice = GCRenderer_GetDevice();
	const GCRendererSwapChain* const RendererSwapChain = GCRenderer_GetSwapChain();
	const GCRendererCommandList* const RendererCommandList = GCRenderer_GetCommandList();
	const GCRendererGraphicsPipeline* const RendererGraphicsPipeline = GCRenderer_GetGraphicsPipeline();
	const GCRendererFramebuffer* const RendererFramebuffer = GCRenderer_GetFramebuffer();

	GCImGuiManager_CreateDescriptorPool();

	ImGui_ImplVulkan_InitInfo VulkanInformation{};
	VulkanInformation.Instance = GCRendererDevice_GetInstanceHandle(RendererDevice);
	VulkanInformation.PhysicalDevice = GCRendererDevice_GetPhysicalDeviceHandle(RendererDevice);
	VulkanInformation.Device = GCRendererDevice_GetDeviceHandle(RendererDevice);
	VulkanInformation.QueueFamily = GCRendererDevice_GetGraphicsFamilyQueueIndex(RendererDevice);
	VulkanInformation.Queue = GCRendererDevice_GetGraphicsQueueHandle(RendererDevice);
	VulkanInformation.PipelineCache = VK_NULL_HANDLE;
	VulkanInformation.DescriptorPool = ImGuiDescriptorPoolHandle;
	VulkanInformation.Subpass = 0;
	VulkanInformation.MinImageCount = 2;
	VulkanInformation.ImageCount = GCRendererSwapChain_GetImageCount(RendererSwapChain);
	VulkanInformation.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
	VulkanInformation.Allocator = NULL;
	VulkanInformation.CheckVkResultFn = GCImGuiManager_CheckVulkanResult;

	ImGui_ImplVulkan_Init(&VulkanInformation, GCRendererGraphicsPipeline_GetSwapChainRenderPassHandle(RendererGraphicsPipeline));

	const VkCommandBuffer CommandBufferHandle = GCVulkanUtilities_BeginSingleTimeCommands(RendererDevice, RendererCommandList);
	ImGui_ImplVulkan_CreateFontsTexture(CommandBufferHandle);
	GCVulkanUtilities_EndSingleTimeCommands(RendererDevice, RendererCommandList, CommandBufferHandle);

	ImGui_ImplVulkan_DestroyFontUploadObjects();

	ImGuiDescriptorSetHandle = ImGui_ImplVulkan_AddTexture(GCRendererFramebuffer_GetTextureSamplerHandle(RendererFramebuffer), GCRendererFramebuffer_GetTextureImageViewHandle(RendererFramebuffer), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
}

extern "C" void GCImGuiManager_BeginFrameRenderer(void)
{
	ImGui_ImplVulkan_NewFrame();
}

extern "C" void* GCImGuiManager_GetTexturePlatform(void)
{
	GCRendererDevice_WaitIdle(GCRenderer_GetDevice());
	GCImGuiManager_UpdateDescriptorSet();

	return ImGuiDescriptorSetHandle;
}

extern "C" void GCImGuiManager_RenderDrawData(void)
{
	const GCRendererCommandList* const RendererCommandList = GCRenderer_GetCommandList();
	ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), GCRendererCommandList_GetCurrentFrameCommandBufferHandle(RendererCommandList), VK_NULL_HANDLE);
}

extern "C" void GCImGuiManager_TerminateRenderer(void)
{
	ImGui_ImplVulkan_Shutdown();

	const VkDevice DeviceHandle = GCRendererDevice_GetDeviceHandle(GCRenderer_GetDevice());
	vkDestroyDescriptorPool(DeviceHandle, ImGuiDescriptorPoolHandle, NULL);
}

void GCImGuiManager_CreateDescriptorPool(void)
{
	const VkDevice DeviceHandle = GCRendererDevice_GetDeviceHandle(GCRenderer_GetDevice());

	std::array<VkDescriptorPoolSize, 11> DescriptorPoolSizes =
	{
		{
			{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
			{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
			{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
			{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 },
		}
	};

	VkDescriptorPoolCreateInfo DescriptorPoolInformation{};
	DescriptorPoolInformation.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	DescriptorPoolInformation.maxSets = static_cast<uint32_t>(1000 * DescriptorPoolSizes.size());
	DescriptorPoolInformation.poolSizeCount = static_cast<uint32_t>(DescriptorPoolSizes.size());
	DescriptorPoolInformation.pPoolSizes = DescriptorPoolSizes.data();

	GC_VULKAN_VALIDATE(vkCreateDescriptorPool(DeviceHandle, &DescriptorPoolInformation, NULL, &ImGuiDescriptorPoolHandle), "Failed to create a Vulkan ImGui descriptor pool");
}

void GCImGuiManager_UpdateDescriptorSet(void)
{
	const GCRendererFramebuffer* const RendererFramebuffer = GCRenderer_GetFramebuffer();

	const ImGui_ImplVulkan_InitInfo* const VulkanInformation = static_cast<const ImGui_ImplVulkan_InitInfo* const>(ImGui::GetIO().BackendRendererUserData);

	VkDescriptorImageInfo DescriptorImageInformation{};
	DescriptorImageInformation.sampler = GCRendererFramebuffer_GetTextureSamplerHandle(RendererFramebuffer);
	DescriptorImageInformation.imageView = GCRendererFramebuffer_GetTextureImageViewHandle(RendererFramebuffer);
	DescriptorImageInformation.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	VkWriteDescriptorSet WriteDescriptorSet{};
	WriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	WriteDescriptorSet.dstSet = ImGuiDescriptorSetHandle;
	WriteDescriptorSet.descriptorCount = 1;
	WriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	WriteDescriptorSet.pImageInfo = &DescriptorImageInformation;

	vkUpdateDescriptorSets(VulkanInformation->Device, 1, &WriteDescriptorSet, 0, nullptr);
}

void GCImGuiManager_CheckVulkanResult(VkResult Result)
{
	if (Result != VK_SUCCESS)
	{
		GC_ASSERT_WITH_MESSAGE(false, "ImGui Vulkan Backend error: %d", Result);
	}
}