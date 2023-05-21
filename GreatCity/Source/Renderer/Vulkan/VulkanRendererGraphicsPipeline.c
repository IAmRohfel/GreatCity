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

VkRenderPass GCRendererGraphicsPipeline_GetRenderPassHandle(const GCRendererGraphicsPipeline* const GraphicsPipeline);
VkPipeline GCRendererGraphicsPipeline_GetPipelineHandle(const GCRendererGraphicsPipeline* const GraphicsPipeline);

extern VkDevice GCRendererDevice_GetDeviceHandle(const GCRendererDevice* const Device);
extern VkFormat GCRendererSwapChain_GetFormat(const GCRendererSwapChain* const SwapChain);
extern VkShaderModule GCRendererShader_GetVertexShaderModuleHandle(const GCRendererShader* const Shader);
extern VkShaderModule GCRendererShader_GetFragmentShaderModuleHandle(const GCRendererShader* const Shader);

static void GCRendererGraphicsPipeline_CreateRenderPass(GCRendererGraphicsPipeline* const GraphicsPipeline);
static void GCRendererGraphicsPipeline_CreateGraphicsPipeline(GCRendererGraphicsPipeline* const GraphicsPipeline, const GCRendererGraphicsPipelineVertexInput* const VertexInput);
static void GCRendererGraphicsPipeline_DestroyObjects(GCRendererGraphicsPipeline* const GraphicsPipeline);

static VkFormat GCRendererGraphicsPipeline_ToVkFormat(const GCRendererGraphicsPipelineVertexInputAttributeFormat Format);

GCRendererGraphicsPipeline* GCRendererGraphicsPipeline_Create(const GCRendererDevice* const Device, const GCRendererSwapChain* const SwapChain, const GCRendererGraphicsPipelineVertexInput* const VertexInput, const GCRendererShader* const Shader)
{
	GCRendererGraphicsPipeline* GraphicsPipeline = (GCRendererGraphicsPipeline*)GCMemory_Allocate(sizeof(GCRendererGraphicsPipeline));
	GraphicsPipeline->Device = Device;
	GraphicsPipeline->SwapChain = SwapChain;
	GraphicsPipeline->Shader = Shader;
	GraphicsPipeline->RenderPassHandle = VK_NULL_HANDLE;
	GraphicsPipeline->PipelineLayoutHandle = VK_NULL_HANDLE;
	GraphicsPipeline->PipelineHandle = VK_NULL_HANDLE;

	GCRendererGraphicsPipeline_CreateRenderPass(GraphicsPipeline);
	GCRendererGraphicsPipeline_CreateGraphicsPipeline(GraphicsPipeline, VertexInput);

	return GraphicsPipeline;
}

void GCRendererGraphicsPipeline_Destroy(GCRendererGraphicsPipeline* GraphicsPipeline)
{
	GCRendererGraphicsPipeline_DestroyObjects(GraphicsPipeline);

	GCMemory_Free(GraphicsPipeline);
}

VkRenderPass GCRendererGraphicsPipeline_GetRenderPassHandle(const GCRendererGraphicsPipeline* const GraphicsPipeline)
{
	return GraphicsPipeline->RenderPassHandle;
}

VkPipeline GCRendererGraphicsPipeline_GetPipelineHandle(const GCRendererGraphicsPipeline* const GraphicsPipeline)
{
	return GraphicsPipeline->PipelineHandle;
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

	VkSubpassDependency SubpassDependency = { 0 };
	SubpassDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	SubpassDependency.dstSubpass = 0;
	SubpassDependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	SubpassDependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	SubpassDependency.srcAccessMask = 0;
	SubpassDependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	VkRenderPassCreateInfo RenderPassInformation = { 0 };
	RenderPassInformation.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	RenderPassInformation.attachmentCount = 1;
	RenderPassInformation.pAttachments = &ColorAttachmentDescription;
	RenderPassInformation.subpassCount = 1;
	RenderPassInformation.pSubpasses = &SubpassDescription;
	RenderPassInformation.dependencyCount = 1;
	RenderPassInformation.pDependencies = &SubpassDependency;

	const VkDevice DeviceHandle = GCRendererDevice_GetDeviceHandle(GraphicsPipeline->Device);

	GC_VULKAN_VALIDATE(vkCreateRenderPass(DeviceHandle, &RenderPassInformation, NULL, &GraphicsPipeline->RenderPassHandle), "Failed to create a Vulkan render pass");
}

void GCRendererGraphicsPipeline_CreateGraphicsPipeline(GCRendererGraphicsPipeline* const GraphicsPipeline, const GCRendererGraphicsPipelineVertexInput* const VertexInput)
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
	GraphicsPipelineInformation.pColorBlendState = &PipelineColorBlendStateInformation;
	GraphicsPipelineInformation.pDynamicState = &PipelineDynamicStateInformation;
	GraphicsPipelineInformation.layout = GraphicsPipeline->PipelineLayoutHandle;
	GraphicsPipelineInformation.renderPass = GraphicsPipeline->RenderPassHandle;
	GraphicsPipelineInformation.subpass = 0;

	GC_VULKAN_VALIDATE(vkCreateGraphicsPipelines(DeviceHandle, VK_NULL_HANDLE, 1, &GraphicsPipelineInformation, NULL, &GraphicsPipeline->PipelineHandle), "Failed to create a Vulkan graphics pipeline");

	GCMemory_Free(VertexInputAttributeDescriptions);
}

void GCRendererGraphicsPipeline_DestroyObjects(GCRendererGraphicsPipeline* const GraphicsPipeline)
{
	const VkDevice DeviceHandle = GCRendererDevice_GetDeviceHandle(GraphicsPipeline->Device);

	vkDestroyPipeline(DeviceHandle, GraphicsPipeline->PipelineHandle, NULL);
	vkDestroyPipelineLayout(DeviceHandle, GraphicsPipeline->PipelineLayoutHandle, NULL);
	vkDestroyRenderPass(DeviceHandle, GraphicsPipeline->RenderPassHandle, NULL);
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