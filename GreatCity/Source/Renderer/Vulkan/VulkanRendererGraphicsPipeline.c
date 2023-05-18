#include "Renderer/Vulkan/VulkanUtilities.h"
#include "Renderer/RendererGraphicsPipeline.h"
#include "Core/Memory/Allocator.h"
#include "Core/Log.h"
#include "Core/Assert.h"

#include <stdbool.h>

#include <vulkan/vulkan.h>

typedef struct GCRendererGraphicsPipeline
{
	const GCRendererDevice* Device;
	const GCRendererSwapChain* SwapChain;
	const GCRendererShader* Shader;

	VkRenderPass RenderPassHandle;
	VkPipelineLayout PipelineLayoutHandle;
	VkPipeline PipelineHandle;
} GCRendererGraphicsPipeline;

extern VkDevice GCRendererDevice_GetDeviceHandle(const GCRendererDevice* const Device);
extern VkFormat GCRendererSwapChain_GetFormat(const GCRendererSwapChain* const SwapChain);
extern VkExtent2D GCRendererSwapChain_GetExtent(const GCRendererSwapChain* const SwapChain);
extern VkShaderModule GCRendererShader_GetVertexShaderModuleHandle(const GCRendererShader* const Shader);
extern VkShaderModule GCRendererShader_GetFragmentShaderModuleHandle(const GCRendererShader* const Shader);

static void GCRendererGraphicsPipeline_CreateRenderPass(GCRendererGraphicsPipeline* const GraphicsPipeline);
static void GCRendererGraphicsPipeline_CreateGraphicsPipeline(GCRendererGraphicsPipeline* const GraphicsPipeline);

GCRendererGraphicsPipeline* GCRendererGraphicsPipeline_Create(const GCRendererDevice* const Device, const GCRendererSwapChain* const SwapChain, const GCRendererShader* const Shader)
{
	GCRendererGraphicsPipeline* GraphicsPipeline = (GCRendererGraphicsPipeline*)GCMemory_Allocate(sizeof(GCRendererGraphicsPipeline));
	GraphicsPipeline->Device = Device;
	GraphicsPipeline->SwapChain = SwapChain;
	GraphicsPipeline->Shader = Shader;
	GraphicsPipeline->RenderPassHandle = VK_NULL_HANDLE;
	GraphicsPipeline->PipelineLayoutHandle = VK_NULL_HANDLE;
	GraphicsPipeline->PipelineHandle = VK_NULL_HANDLE;

	GCRendererGraphicsPipeline_CreateRenderPass(GraphicsPipeline);
	GCRendererGraphicsPipeline_CreateGraphicsPipeline(GraphicsPipeline);

	return GraphicsPipeline;
}

void GCRendererGraphicsPipeline_Destroy(GCRendererGraphicsPipeline* GraphicsPipeline)
{
	const VkDevice DeviceHandle = GCRendererDevice_GetDeviceHandle(GraphicsPipeline->Device);

	vkDestroyPipeline(DeviceHandle, GraphicsPipeline->PipelineHandle, NULL);
	vkDestroyPipelineLayout(DeviceHandle, GraphicsPipeline->PipelineLayoutHandle, NULL);
	vkDestroyRenderPass(DeviceHandle, GraphicsPipeline->RenderPassHandle, NULL);

	GCMemory_Free(GraphicsPipeline);
}

void GCRendererGraphicsPipeline_CreateRenderPass(GCRendererGraphicsPipeline* const GraphicsPipeline)
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

	VkAttachmentReference ColorAttachmentReference = { 0 };
	ColorAttachmentReference.attachment = 0;
	ColorAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription SubpassDescription = { 0 };
	SubpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	SubpassDescription.colorAttachmentCount = 1;
	SubpassDescription.pColorAttachments = &ColorAttachmentReference;

	VkRenderPassCreateInfo RenderPassInformation = { 0 };
	RenderPassInformation.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	RenderPassInformation.attachmentCount = 1;
	RenderPassInformation.pAttachments = &ColorAttachmentDescription;
	RenderPassInformation.subpassCount = 1;
	RenderPassInformation.pSubpasses = &SubpassDescription;

	const VkDevice DeviceHandle = GCRendererDevice_GetDeviceHandle(GraphicsPipeline->Device);

	GC_VULKAN_VALIDATE(vkCreateRenderPass(DeviceHandle, &RenderPassInformation, NULL, &GraphicsPipeline->RenderPassHandle), "Failed to create a Vulkan render pass");
}

void GCRendererGraphicsPipeline_CreateGraphicsPipeline(GCRendererGraphicsPipeline* const GraphicsPipeline)
{
	const VkDevice DeviceHandle = GCRendererDevice_GetDeviceHandle(GraphicsPipeline->Device);

	VkPipelineLayoutCreateInfo PipelineLayoutInformation = { 0 };
	PipelineLayoutInformation.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

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

	VkPipelineVertexInputStateCreateInfo PipelineVertexInputStateInformation = { 0 };
	PipelineVertexInputStateInformation.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

	VkPipelineInputAssemblyStateCreateInfo PipelineInputAssemblyStateInformation = { 0 };
	PipelineInputAssemblyStateInformation.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	PipelineInputAssemblyStateInformation.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	PipelineInputAssemblyStateInformation.primitiveRestartEnable = VK_FALSE;

	VkViewport Viewport = { 0 };
	Viewport.x = 0.0f;
	Viewport.y = 0.0f;

	const VkExtent2D SwapChainExtent = GCRendererSwapChain_GetExtent(GraphicsPipeline->SwapChain);

	Viewport.width = (float)SwapChainExtent.width;
	Viewport.height = (float)SwapChainExtent.height;
	Viewport.minDepth = 0.0f;
	Viewport.maxDepth = 1.0f;

	VkRect2D Scissor = { 0 };
	Scissor.offset = (VkOffset2D){ 0 };
	Scissor.extent = SwapChainExtent;

	VkPipelineViewportStateCreateInfo PipelineViewportStateInformation = { 0 };
	PipelineViewportStateInformation.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	PipelineViewportStateInformation.viewportCount = 1;
	PipelineViewportStateInformation.pViewports = &Viewport;
	PipelineViewportStateInformation.scissorCount = 1;
	PipelineViewportStateInformation.pScissors = &Scissor;

	VkPipelineRasterizationStateCreateInfo PipelineRasterizationStateInformation = { 0 };
	PipelineRasterizationStateInformation.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	PipelineRasterizationStateInformation.depthClampEnable = VK_FALSE;
	PipelineRasterizationStateInformation.rasterizerDiscardEnable = VK_FALSE;
	PipelineRasterizationStateInformation.polygonMode = VK_POLYGON_MODE_FILL;
	PipelineRasterizationStateInformation.cullMode = VK_CULL_MODE_BACK_BIT;
	PipelineRasterizationStateInformation.frontFace = VK_FRONT_FACE_CLOCKWISE;
	PipelineRasterizationStateInformation.depthBiasEnable = VK_FALSE;
	PipelineRasterizationStateInformation.lineWidth = 1.0f;

	VkPipelineMultisampleStateCreateInfo PipelineMultisampleStateInformation = { 0 };
	PipelineMultisampleStateInformation.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	PipelineMultisampleStateInformation.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	PipelineMultisampleStateInformation.sampleShadingEnable = VK_FALSE;
	PipelineMultisampleStateInformation.minSampleShading = 1.0f;

	VkPipelineColorBlendAttachmentState PipelineColorBlendAttachmentState = { 0 };
	PipelineColorBlendAttachmentState.blendEnable = VK_FALSE;
	PipelineColorBlendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

	VkPipelineColorBlendStateCreateInfo PipelineColorBlendStateInformation = { 0 };
	PipelineColorBlendStateInformation.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	PipelineColorBlendStateInformation.logicOpEnable = VK_FALSE;
	PipelineColorBlendStateInformation.attachmentCount = 1;
	PipelineColorBlendStateInformation.pAttachments = &PipelineColorBlendAttachmentState;

	VkPipelineDynamicStateCreateInfo PipelineDynamicStateInformation = { 0 };
	PipelineDynamicStateInformation.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;

	VkGraphicsPipelineCreateInfo GraphicsPipelineInformation = { 0 };
	GraphicsPipelineInformation.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	GraphicsPipelineInformation.stageCount = 2;
	GraphicsPipelineInformation.pStages = PipelineShaderStageInformation;
	GraphicsPipelineInformation.pVertexInputState = &PipelineVertexInputStateInformation;
	GraphicsPipelineInformation.pInputAssemblyState = &PipelineInputAssemblyStateInformation;
	GraphicsPipelineInformation.pViewportState = &PipelineViewportStateInformation;
	GraphicsPipelineInformation.pRasterizationState = &PipelineRasterizationStateInformation;
	GraphicsPipelineInformation.pMultisampleState = &PipelineMultisampleStateInformation;
	GraphicsPipelineInformation.pColorBlendState = &PipelineColorBlendStateInformation;
	GraphicsPipelineInformation.layout = GraphicsPipeline->PipelineLayoutHandle;
	GraphicsPipelineInformation.renderPass = GraphicsPipeline->RenderPassHandle;
	GraphicsPipelineInformation.subpass = 0;

	GC_VULKAN_VALIDATE(vkCreateGraphicsPipelines(DeviceHandle, VK_NULL_HANDLE, 1, &GraphicsPipelineInformation, NULL, &GraphicsPipeline->PipelineHandle), "Failed to create a Vulkan graphics pipeline");
}