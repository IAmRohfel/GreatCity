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

#include "Renderer/Vulkan/VulkanRendererGraphicsPipeline.h"
#include "Renderer/Vulkan/VulkanRendererDevice.h"
#include "Renderer/Vulkan/VulkanRendererSwapChain.h"
#include "Renderer/Vulkan/VulkanRendererCommandList.h"
#include "Renderer/Vulkan/VulkanRendererUniformBuffer.h"
#include "Renderer/Vulkan/VulkanRendererTexture2D.h"
#include "Renderer/Vulkan/VulkanRendererShader.h"
#include "Renderer/Vulkan/VulkanUtilities.h"
#include "Renderer/RendererGraphicsPipeline.h"
#include "Renderer/RendererDevice.h"
#include "Core/Memory/Allocator.h"
#include "Core/Log.h"
#include "Core/Assert.h"

#include <stdbool.h>

#include <vulkan/vulkan.h>

typedef struct GCRendererGraphicsPipeline
{
	const GCRendererDevice* Device;
	const GCRendererSwapChain* SwapChain;
	const GCRendererCommandList* CommandList;
	const GCRendererUniformBuffer* UniformBuffer;
	const GCRendererTexture2D* const* Texture2Ds;
	const GCRendererShader* Shader;

	VkRenderPass TextureRenderPassHandle, SwapChainRenderPassHandle;
	VkDescriptorSetLayout DescriptorSetLayoutHandle;
	VkDescriptorPool DescriptorPoolHandle;
	VkDescriptorSet* DescriptorSetHandles;
	VkPipelineLayout PipelineLayoutHandle;
	VkPipeline PipelineHandle;

	uint32_t Texture2DCount;
	uint32_t DescriptorCount;
} GCRendererGraphicsPipeline;

static void GCRendererGraphicsPipeline_CreateTextureRenderPass(GCRendererGraphicsPipeline* const GraphicsPipeline);
static void GCRendererGraphicsPipeline_CreateSwapChainRenderPass(GCRendererGraphicsPipeline* const GraphicsPipeline);
static void GCRendererGraphicsPipeline_CreateDescriptorSetLayout(GCRendererGraphicsPipeline* const GraphicsPipeline);
static void GCRendererGraphicsPipeline_CreateGraphicsPipeline(GCRendererGraphicsPipeline* const GraphicsPipeline, const GCRendererGraphicsPipelineVertexInput* const VertexInput);
static void GCRendererGraphicsPipeline_CreateDescriptorPool(GCRendererGraphicsPipeline* const GraphicsPipeline);
static void GCRendererGraphicsPipeline_CreateDescriptorSets(GCRendererGraphicsPipeline* const GraphicsPipeline);
static void GCRendererGraphicsPipeline_DestroyObjects(GCRendererGraphicsPipeline* const GraphicsPipeline);

static VkFormat GCRendererGraphicsPipeline_ToVkFormat(const GCRendererGraphicsPipelineVertexInputAttributeFormat Format);

GCRendererGraphicsPipeline* GCRendererGraphicsPipeline_Create(const GCRendererDevice* const Device, const GCRendererSwapChain* const SwapChain, const GCRendererCommandList* const CommandList, const GCRendererGraphicsPipelineVertexInput* const VertexInput, const GCRendererUniformBuffer* const UniformBuffer, const GCRendererTexture2D* const* const Texture2Ds, const uint32_t Texture2DCount, const GCRendererShader* const Shader)
{
	GCRendererGraphicsPipeline* GraphicsPipeline = (GCRendererGraphicsPipeline*)GCMemory_Allocate(sizeof(GCRendererGraphicsPipeline));
	GraphicsPipeline->Device = Device;
	GraphicsPipeline->SwapChain = SwapChain;
	GraphicsPipeline->CommandList = CommandList;
	GraphicsPipeline->UniformBuffer = UniformBuffer;
	GraphicsPipeline->Texture2Ds = Texture2Ds;
	GraphicsPipeline->Shader = Shader;
	GraphicsPipeline->TextureRenderPassHandle = VK_NULL_HANDLE;
	GraphicsPipeline->SwapChainRenderPassHandle = VK_NULL_HANDLE;
	GraphicsPipeline->DescriptorSetLayoutHandle = VK_NULL_HANDLE;
	GraphicsPipeline->DescriptorPoolHandle = VK_NULL_HANDLE;
	GraphicsPipeline->DescriptorSetHandles = NULL;
	GraphicsPipeline->PipelineLayoutHandle = VK_NULL_HANDLE;
	GraphicsPipeline->PipelineHandle = VK_NULL_HANDLE;
	GraphicsPipeline->Texture2DCount = Texture2DCount;
	GraphicsPipeline->DescriptorCount = 1 + GraphicsPipeline->Texture2DCount;

	GCRendererGraphicsPipeline_CreateTextureRenderPass(GraphicsPipeline);
	GCRendererGraphicsPipeline_CreateSwapChainRenderPass(GraphicsPipeline);
	GCRendererGraphicsPipeline_CreateDescriptorSetLayout(GraphicsPipeline);
	GCRendererGraphicsPipeline_CreateGraphicsPipeline(GraphicsPipeline, VertexInput);
	GCRendererGraphicsPipeline_CreateDescriptorPool(GraphicsPipeline);
	GCRendererGraphicsPipeline_CreateDescriptorSets(GraphicsPipeline);

	return GraphicsPipeline;
}

void GCRendererGraphicsPipeline_Destroy(GCRendererGraphicsPipeline* GraphicsPipeline)
{
	GCRendererDevice_WaitIdle(GraphicsPipeline->Device);

	GCRendererGraphicsPipeline_DestroyObjects(GraphicsPipeline);

	GCMemory_Free(GraphicsPipeline->DescriptorSetHandles);
	GCMemory_Free(GraphicsPipeline);
}

VkRenderPass GCRendererGraphicsPipeline_GetTextureRenderPassHandle(const GCRendererGraphicsPipeline* const GraphicsPipeline)
{
	return GraphicsPipeline->TextureRenderPassHandle;
}

VkRenderPass GCRendererGraphicsPipeline_GetSwapChainRenderPassHandle(const GCRendererGraphicsPipeline* const GraphicsPipeline)
{
	return GraphicsPipeline->SwapChainRenderPassHandle;
}

VkPipelineLayout GCRendererGraphicsPipeline_GetPipelineLayoutHandle(const GCRendererGraphicsPipeline* const GraphicsPipeline)
{
	return GraphicsPipeline->PipelineLayoutHandle;
}

VkPipeline GCRendererGraphicsPipeline_GetPipelineHandle(const GCRendererGraphicsPipeline* const GraphicsPipeline)
{
	return GraphicsPipeline->PipelineHandle;
}

VkDescriptorSet* GCRendererGraphicsPipeline_GetDescriptorSetHandles(const GCRendererGraphicsPipeline* const GraphicsPipeline)
{
	return GraphicsPipeline->DescriptorSetHandles;
}

void GCRendererGraphicsPipeline_CreateTextureRenderPass(GCRendererGraphicsPipeline* const GraphicsPipeline)
{
	const VkFormat SwapChainFormat = GCRendererSwapChain_GetFormat(GraphicsPipeline->SwapChain);

	VkAttachmentDescription ColorAttachmentDescription = { 0 };
	ColorAttachmentDescription.format = SwapChainFormat;
	ColorAttachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;
	ColorAttachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	ColorAttachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	ColorAttachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	ColorAttachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	ColorAttachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	ColorAttachmentDescription.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	VkAttachmentDescription DepthAttachmentDescription = { 0 };
	DepthAttachmentDescription.format = GCRendererSwapChain_GetDepthFormat(GraphicsPipeline->SwapChain);
	DepthAttachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;
	DepthAttachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	DepthAttachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	DepthAttachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	DepthAttachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	DepthAttachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	DepthAttachmentDescription.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentReference ColorAttachmentReference = { 0 };
	ColorAttachmentReference.attachment = 0;
	ColorAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentReference DepthAttachmentReference = { 0 };
	DepthAttachmentReference.attachment = 1;
	DepthAttachmentReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkSubpassDescription SubpassDescription = { 0 };
	SubpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	SubpassDescription.colorAttachmentCount = 1;
	SubpassDescription.pColorAttachments = &ColorAttachmentReference;
	SubpassDescription.pDepthStencilAttachment = &DepthAttachmentReference;

	VkSubpassDependency SubpassDependency = { 0 };
	SubpassDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	SubpassDependency.dstSubpass = 0;
	SubpassDependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	SubpassDependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	SubpassDependency.srcAccessMask = 0;
	SubpassDependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

	const VkAttachmentDescription AttachmentDescriptions[2] = { ColorAttachmentDescription, DepthAttachmentDescription };

	VkRenderPassCreateInfo RenderPassInformation = { 0 };
	RenderPassInformation.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	RenderPassInformation.attachmentCount = 2;
	RenderPassInformation.pAttachments = AttachmentDescriptions;
	RenderPassInformation.subpassCount = 1;
	RenderPassInformation.pSubpasses = &SubpassDescription;
	RenderPassInformation.dependencyCount = 1;
	RenderPassInformation.pDependencies = &SubpassDependency;

	const VkDevice DeviceHandle = GCRendererDevice_GetDeviceHandle(GraphicsPipeline->Device);

	GC_VULKAN_VALIDATE(vkCreateRenderPass(DeviceHandle, &RenderPassInformation, NULL, &GraphicsPipeline->TextureRenderPassHandle), "Failed to create a Vulkan texture render pass");
}

void GCRendererGraphicsPipeline_CreateSwapChainRenderPass(GCRendererGraphicsPipeline* const GraphicsPipeline)
{
	const VkFormat SwapChainFormat = GCRendererSwapChain_GetFormat(GraphicsPipeline->SwapChain);

	VkAttachmentDescription ColorAttachmentDescription = { 0 };
	ColorAttachmentDescription.format = SwapChainFormat;
	ColorAttachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;
	ColorAttachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	ColorAttachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	ColorAttachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	ColorAttachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	ColorAttachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	ColorAttachmentDescription.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentDescription DepthAttachmentDescription = { 0 };
	DepthAttachmentDescription.format = GCRendererSwapChain_GetDepthFormat(GraphicsPipeline->SwapChain);
	DepthAttachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;
	DepthAttachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	DepthAttachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	DepthAttachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	DepthAttachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	DepthAttachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	DepthAttachmentDescription.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentReference ColorAttachmentReference = { 0 };
	ColorAttachmentReference.attachment = 0;
	ColorAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentReference DepthAttachmentReference = { 0 };
	DepthAttachmentReference.attachment = 1;
	DepthAttachmentReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkSubpassDescription SubpassDescription = { 0 };
	SubpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	SubpassDescription.colorAttachmentCount = 1;
	SubpassDescription.pColorAttachments = &ColorAttachmentReference;
	SubpassDescription.pDepthStencilAttachment = &DepthAttachmentReference;

	VkSubpassDependency SubpassDependency = { 0 };
	SubpassDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	SubpassDependency.dstSubpass = 0;
	SubpassDependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	SubpassDependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	SubpassDependency.srcAccessMask = 0;
	SubpassDependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

	const VkAttachmentDescription AttachmentDescriptions[2] = { ColorAttachmentDescription, DepthAttachmentDescription };

	VkRenderPassCreateInfo RenderPassInformation = { 0 };
	RenderPassInformation.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	RenderPassInformation.attachmentCount = 2;
	RenderPassInformation.pAttachments = AttachmentDescriptions;
	RenderPassInformation.subpassCount = 1;
	RenderPassInformation.pSubpasses = &SubpassDescription;
	RenderPassInformation.dependencyCount = 1;
	RenderPassInformation.pDependencies = &SubpassDependency;

	const VkDevice DeviceHandle = GCRendererDevice_GetDeviceHandle(GraphicsPipeline->Device);

	GC_VULKAN_VALIDATE(vkCreateRenderPass(DeviceHandle, &RenderPassInformation, NULL, &GraphicsPipeline->SwapChainRenderPassHandle), "Failed to create a Vulkan swap chain render pass");
}

void GCRendererGraphicsPipeline_CreateDescriptorSetLayout(GCRendererGraphicsPipeline* const GraphicsPipeline)
{
	const VkDevice DeviceHandle = GCRendererDevice_GetDeviceHandle(GraphicsPipeline->Device);

	VkDescriptorSetLayoutBinding* DescriptorSetLayoutBindings = (VkDescriptorSetLayoutBinding*)GCMemory_AllocateZero(GraphicsPipeline->DescriptorCount * sizeof(VkDescriptorSetLayoutBinding));
	DescriptorSetLayoutBindings[0].binding = 0;
	DescriptorSetLayoutBindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	DescriptorSetLayoutBindings[0].descriptorCount = 1;
	DescriptorSetLayoutBindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

	for (uint32_t Counter = 0; Counter < GraphicsPipeline->Texture2DCount; Counter++)
	{
		DescriptorSetLayoutBindings[Counter + 1].binding = Counter + 1;
		DescriptorSetLayoutBindings[Counter + 1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		DescriptorSetLayoutBindings[Counter + 1].descriptorCount = 1;
		DescriptorSetLayoutBindings[Counter + 1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	}

	VkDescriptorSetLayoutCreateInfo DescriptorSetLayoutInformation = { 0 };
	DescriptorSetLayoutInformation.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	DescriptorSetLayoutInformation.bindingCount = GraphicsPipeline->DescriptorCount;
	DescriptorSetLayoutInformation.pBindings = DescriptorSetLayoutBindings;

	GC_VULKAN_VALIDATE(vkCreateDescriptorSetLayout(DeviceHandle, &DescriptorSetLayoutInformation, NULL, &GraphicsPipeline->DescriptorSetLayoutHandle), "Failed to create a Vulkan descriptor set layout");

	GCMemory_Free(DescriptorSetLayoutBindings);
}

void GCRendererGraphicsPipeline_CreateGraphicsPipeline(GCRendererGraphicsPipeline* const GraphicsPipeline, const GCRendererGraphicsPipelineVertexInput* const VertexInput)
{
	const VkDevice DeviceHandle = GCRendererDevice_GetDeviceHandle(GraphicsPipeline->Device);

	VkPipelineLayoutCreateInfo PipelineLayoutInformation = { 0 };
	PipelineLayoutInformation.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	PipelineLayoutInformation.setLayoutCount = 1;
	PipelineLayoutInformation.pSetLayouts = &GraphicsPipeline->DescriptorSetLayoutHandle;

	GC_VULKAN_VALIDATE(vkCreatePipelineLayout(DeviceHandle, &PipelineLayoutInformation, NULL, &GraphicsPipeline->PipelineLayoutHandle), "Failed to create a Vulkan pipeline layout");

	VkPipelineShaderStageCreateInfo PipelineVertexShaderStageInformation = { 0 };
	PipelineVertexShaderStageInformation.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	PipelineVertexShaderStageInformation.stage = VK_SHADER_STAGE_VERTEX_BIT;
	PipelineVertexShaderStageInformation.module = GCRendererShader_GetVertexShaderModuleHandle(GraphicsPipeline->Shader);
	PipelineVertexShaderStageInformation.pName = "main";

	VkPipelineShaderStageCreateInfo PipelineFragmentShaderStageInformation = { 0 };
	PipelineFragmentShaderStageInformation.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	PipelineFragmentShaderStageInformation.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	PipelineFragmentShaderStageInformation.module = GCRendererShader_GetFragmentShaderModuleHandle(GraphicsPipeline->Shader);
	PipelineFragmentShaderStageInformation.pName = "main";

	const VkPipelineShaderStageCreateInfo PipelineShaderStageInformation[2] =
	{
		PipelineVertexShaderStageInformation,
		PipelineFragmentShaderStageInformation
	};

	VkVertexInputBindingDescription VertexInputBindingDescription = { 0 };
	VertexInputBindingDescription.binding = 0;
	VertexInputBindingDescription.stride = VertexInput->Stride;
	VertexInputBindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	VkVertexInputAttributeDescription* VertexInputAttributeDescriptions = (VkVertexInputAttributeDescription*)GCMemory_Allocate(VertexInput->AttributeCount * sizeof(VkVertexInputAttributeDescription));

	for (uint32_t Counter = 0; Counter < VertexInput->AttributeCount; Counter++)
	{
		VertexInputAttributeDescriptions[Counter].location = VertexInput->Attributes[Counter].Location;
		VertexInputAttributeDescriptions[Counter].binding = 0;
		VertexInputAttributeDescriptions[Counter].format = GCRendererGraphicsPipeline_ToVkFormat(VertexInput->Attributes[Counter].Format);
		VertexInputAttributeDescriptions[Counter].offset = VertexInput->Attributes[Counter].Offset;
	}

	VkPipelineVertexInputStateCreateInfo PipelineVertexInputStateInformation = { 0 };
	PipelineVertexInputStateInformation.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	PipelineVertexInputStateInformation.vertexBindingDescriptionCount = 1;
	PipelineVertexInputStateInformation.pVertexBindingDescriptions = &VertexInputBindingDescription;
	PipelineVertexInputStateInformation.vertexAttributeDescriptionCount = VertexInput->AttributeCount;
	PipelineVertexInputStateInformation.pVertexAttributeDescriptions = VertexInputAttributeDescriptions;

	VkPipelineInputAssemblyStateCreateInfo PipelineInputAssemblyStateInformation = { 0 };
	PipelineInputAssemblyStateInformation.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	PipelineInputAssemblyStateInformation.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	PipelineInputAssemblyStateInformation.primitiveRestartEnable = VK_FALSE;

	VkPipelineViewportStateCreateInfo PipelineViewportStateInformation = { 0 };
	PipelineViewportStateInformation.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	PipelineViewportStateInformation.viewportCount = 1;
	PipelineViewportStateInformation.scissorCount = 1;

	VkPipelineRasterizationStateCreateInfo PipelineRasterizationStateInformation = { 0 };
	PipelineRasterizationStateInformation.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	PipelineRasterizationStateInformation.depthClampEnable = VK_FALSE;
	PipelineRasterizationStateInformation.rasterizerDiscardEnable = VK_FALSE;
	PipelineRasterizationStateInformation.polygonMode = VK_POLYGON_MODE_FILL;
	PipelineRasterizationStateInformation.cullMode = VK_CULL_MODE_NONE;
	PipelineRasterizationStateInformation.frontFace = VK_FRONT_FACE_CLOCKWISE;
	PipelineRasterizationStateInformation.depthBiasEnable = VK_FALSE;
	PipelineRasterizationStateInformation.lineWidth = 1.0f;

	VkPipelineMultisampleStateCreateInfo PipelineMultisampleStateInformation = { 0 };
	PipelineMultisampleStateInformation.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	PipelineMultisampleStateInformation.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	PipelineMultisampleStateInformation.sampleShadingEnable = VK_FALSE;
	PipelineMultisampleStateInformation.minSampleShading = 1.0f;

	VkPipelineDepthStencilStateCreateInfo PipelineDepthStencilStateInformation = { 0 };
	PipelineDepthStencilStateInformation.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	PipelineDepthStencilStateInformation.depthTestEnable = VK_TRUE;
	PipelineDepthStencilStateInformation.depthWriteEnable = VK_TRUE;
	PipelineDepthStencilStateInformation.depthCompareOp = VK_COMPARE_OP_LESS;
	PipelineDepthStencilStateInformation.depthBoundsTestEnable = VK_FALSE;
	PipelineDepthStencilStateInformation.stencilTestEnable = VK_FALSE;

	VkPipelineColorBlendAttachmentState PipelineColorBlendAttachmentState = { 0 };
	PipelineColorBlendAttachmentState.blendEnable = VK_TRUE;
	PipelineColorBlendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	PipelineColorBlendAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	PipelineColorBlendAttachmentState.colorBlendOp = VK_BLEND_OP_ADD;
	PipelineColorBlendAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	PipelineColorBlendAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	PipelineColorBlendAttachmentState.alphaBlendOp = VK_BLEND_OP_ADD;
	PipelineColorBlendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

	VkPipelineColorBlendStateCreateInfo PipelineColorBlendStateInformation = { 0 };
	PipelineColorBlendStateInformation.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	PipelineColorBlendStateInformation.logicOpEnable = VK_FALSE;
	PipelineColorBlendStateInformation.attachmentCount = 1;
	PipelineColorBlendStateInformation.pAttachments = &PipelineColorBlendAttachmentState;

	const VkDynamicState DynamicStates[2] =
	{
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR
	};

	VkPipelineDynamicStateCreateInfo PipelineDynamicStateInformation = { 0 };
	PipelineDynamicStateInformation.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	PipelineDynamicStateInformation.dynamicStateCount = 2;
	PipelineDynamicStateInformation.pDynamicStates = DynamicStates;

	VkGraphicsPipelineCreateInfo GraphicsPipelineInformation = { 0 };
	GraphicsPipelineInformation.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	GraphicsPipelineInformation.stageCount = 2;
	GraphicsPipelineInformation.pStages = PipelineShaderStageInformation;
	GraphicsPipelineInformation.pVertexInputState = &PipelineVertexInputStateInformation;
	GraphicsPipelineInformation.pInputAssemblyState = &PipelineInputAssemblyStateInformation;
	GraphicsPipelineInformation.pViewportState = &PipelineViewportStateInformation;
	GraphicsPipelineInformation.pRasterizationState = &PipelineRasterizationStateInformation;
	GraphicsPipelineInformation.pMultisampleState = &PipelineMultisampleStateInformation;
	GraphicsPipelineInformation.pDepthStencilState = &PipelineDepthStencilStateInformation;
	GraphicsPipelineInformation.pColorBlendState = &PipelineColorBlendStateInformation;
	GraphicsPipelineInformation.pDynamicState = &PipelineDynamicStateInformation;
	GraphicsPipelineInformation.layout = GraphicsPipeline->PipelineLayoutHandle;
	GraphicsPipelineInformation.renderPass = GraphicsPipeline->TextureRenderPassHandle;
	GraphicsPipelineInformation.subpass = 0;

	GC_VULKAN_VALIDATE(vkCreateGraphicsPipelines(DeviceHandle, VK_NULL_HANDLE, 1, &GraphicsPipelineInformation, NULL, &GraphicsPipeline->PipelineHandle), "Failed to create a Vulkan graphics pipeline");

	GCMemory_Free(VertexInputAttributeDescriptions);
}

void GCRendererGraphicsPipeline_CreateDescriptorPool(GCRendererGraphicsPipeline* const GraphicsPipeline)
{
	const VkDevice DeviceHandle = GCRendererDevice_GetDeviceHandle(GraphicsPipeline->Device);
	const uint32_t MaximumFramesInFlight = GCRendererCommandList_GetMaximumFramesInFlight(GraphicsPipeline->CommandList);

	VkDescriptorPoolSize* DescriptorPoolSizes = (VkDescriptorPoolSize*)GCMemory_AllocateZero(GraphicsPipeline->DescriptorCount * sizeof(VkDescriptorPoolSize));
	DescriptorPoolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	DescriptorPoolSizes[0].descriptorCount = MaximumFramesInFlight;

	for (uint32_t Counter = 0; Counter < GraphicsPipeline->Texture2DCount; Counter++)
	{
		DescriptorPoolSizes[Counter + 1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		DescriptorPoolSizes[Counter + 1].descriptorCount = MaximumFramesInFlight;
	}

	VkDescriptorPoolCreateInfo DescriptorPoolInformation = { 0 };
	DescriptorPoolInformation.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	DescriptorPoolInformation.maxSets = MaximumFramesInFlight;
	DescriptorPoolInformation.poolSizeCount = GraphicsPipeline->DescriptorCount;
	DescriptorPoolInformation.pPoolSizes = DescriptorPoolSizes;

	GC_VULKAN_VALIDATE(vkCreateDescriptorPool(DeviceHandle, &DescriptorPoolInformation, NULL, &GraphicsPipeline->DescriptorPoolHandle), "Failed to create a Vulkan descriptor pool");

	GCMemory_Free(DescriptorPoolSizes);
}

void GCRendererGraphicsPipeline_CreateDescriptorSets(GCRendererGraphicsPipeline* const GraphicsPipeline)
{
	const VkDevice DeviceHandle = GCRendererDevice_GetDeviceHandle(GraphicsPipeline->Device);
	const uint32_t MaximumFramesInFlight = GCRendererCommandList_GetMaximumFramesInFlight(GraphicsPipeline->CommandList);

	VkDescriptorSetLayout* const DescriptorSetLayoutHandles = (VkDescriptorSetLayout* const)GCMemory_Allocate(MaximumFramesInFlight * sizeof(VkDescriptorSetLayout));

	for (uint32_t Counter = 0; Counter < MaximumFramesInFlight; Counter++)
	{
		DescriptorSetLayoutHandles[Counter] = GraphicsPipeline->DescriptorSetLayoutHandle;
	}

	GraphicsPipeline->DescriptorSetHandles = (VkDescriptorSet*)GCMemory_Allocate(MaximumFramesInFlight * sizeof(VkDescriptorSet));

	VkDescriptorSetAllocateInfo DescriptorSetAllocateInformation = { 0 };
	DescriptorSetAllocateInformation.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	DescriptorSetAllocateInformation.descriptorPool = GraphicsPipeline->DescriptorPoolHandle;
	DescriptorSetAllocateInformation.descriptorSetCount = MaximumFramesInFlight;
	DescriptorSetAllocateInformation.pSetLayouts = DescriptorSetLayoutHandles;

	GC_VULKAN_VALIDATE(vkAllocateDescriptorSets(DeviceHandle, &DescriptorSetAllocateInformation, GraphicsPipeline->DescriptorSetHandles), "Failed to allocate Vulkan descriptor sets");

	for (uint32_t CounterFrame = 0; CounterFrame < MaximumFramesInFlight; CounterFrame++)
	{
		VkWriteDescriptorSet* WriteDescriptorSets = (VkWriteDescriptorSet*)GCMemory_AllocateZero(GraphicsPipeline->DescriptorCount * sizeof(VkWriteDescriptorSet));

		VkDescriptorBufferInfo DescriptorBufferInformation = { 0 };
		DescriptorBufferInformation.buffer = GCRendererUniformBuffer_GetBufferHandles(GraphicsPipeline->UniformBuffer)[CounterFrame];
		DescriptorBufferInformation.offset = 0;
		DescriptorBufferInformation.range = GCRendererUniformBuffer_GetDataSize(GraphicsPipeline->UniformBuffer);

		for (uint32_t Counter = 0; Counter < GraphicsPipeline->Texture2DCount; Counter++)
		{
			VkDescriptorImageInfo DescriptorImageInformation = { 0 };
			DescriptorImageInformation.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			DescriptorImageInformation.imageView = GCRendererTexture2D_GetImageViewHandle(GraphicsPipeline->Texture2Ds[Counter]);
			DescriptorImageInformation.sampler = GCRendererTexture2D_GetSamplerHandle(GraphicsPipeline->Texture2Ds[Counter]);

			WriteDescriptorSets[Counter + 1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			WriteDescriptorSets[Counter + 1].dstSet = GraphicsPipeline->DescriptorSetHandles[CounterFrame];
			WriteDescriptorSets[Counter + 1].dstBinding = Counter + 1;
			WriteDescriptorSets[Counter + 1].dstArrayElement = 0;
			WriteDescriptorSets[Counter + 1].descriptorCount = 1;
			WriteDescriptorSets[Counter + 1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			WriteDescriptorSets[Counter + 1].pImageInfo = &DescriptorImageInformation;
		}

		WriteDescriptorSets[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		WriteDescriptorSets[0].dstSet = GraphicsPipeline->DescriptorSetHandles[CounterFrame];
		WriteDescriptorSets[0].dstBinding = 0;
		WriteDescriptorSets[0].dstArrayElement = 0;
		WriteDescriptorSets[0].descriptorCount = 1;
		WriteDescriptorSets[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		WriteDescriptorSets[0].pBufferInfo = &DescriptorBufferInformation;

		vkUpdateDescriptorSets(DeviceHandle, GraphicsPipeline->DescriptorCount, WriteDescriptorSets, 0, NULL);

		GCMemory_Free(WriteDescriptorSets);
	}

	GCMemory_Free(DescriptorSetLayoutHandles);
}

void GCRendererGraphicsPipeline_DestroyObjects(GCRendererGraphicsPipeline* const GraphicsPipeline)
{
	const VkDevice DeviceHandle = GCRendererDevice_GetDeviceHandle(GraphicsPipeline->Device);

	vkDestroyDescriptorPool(DeviceHandle, GraphicsPipeline->DescriptorPoolHandle, NULL);

	vkDestroyPipeline(DeviceHandle, GraphicsPipeline->PipelineHandle, NULL);
	vkDestroyPipelineLayout(DeviceHandle, GraphicsPipeline->PipelineLayoutHandle, NULL);
	vkDestroyDescriptorSetLayout(DeviceHandle, GraphicsPipeline->DescriptorSetLayoutHandle, NULL);
	vkDestroyRenderPass(DeviceHandle, GraphicsPipeline->SwapChainRenderPassHandle, NULL);
	vkDestroyRenderPass(DeviceHandle, GraphicsPipeline->TextureRenderPassHandle, NULL);
}

VkFormat GCRendererGraphicsPipeline_ToVkFormat(const GCRendererGraphicsPipelineVertexInputAttributeFormat Format)
{
	switch (Format)
	{
		case GCRendererGraphicsPipelineVertexInputAttributeFormat_Float:
		{
			return VK_FORMAT_R32_SFLOAT;

			break;
		}
		case GCRendererGraphicsPipelineVertexInputAttributeFormat_Vector2:
		{
			return VK_FORMAT_R32G32_SFLOAT;

			break;
		}
		case GCRendererGraphicsPipelineVertexInputAttributeFormat_Vector3:
		{
			return VK_FORMAT_R32G32B32_SFLOAT;

			break;
		}
		case GCRendererGraphicsPipelineVertexInputAttributeFormat_Vector4:
		{
			return VK_FORMAT_R32G32B32A32_SFLOAT;

			break;
		}
	}

	GC_ASSERT_WITH_MESSAGE(false, "'%d': Invalid GCRendererGraphicsPipelineVertexInputAttributeFormat", Format);
	return VK_FORMAT_UNDEFINED;
}