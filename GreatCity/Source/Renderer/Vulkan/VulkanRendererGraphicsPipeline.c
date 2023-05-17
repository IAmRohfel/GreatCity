#include "Renderer/Vulkan/VulkanUtilities.h"
#include "Renderer/RendererGraphicsPipeline.h"
#include "Core/Memory/Allocator.h"

#include <vulkan/vulkan.h>

typedef struct GCRendererGraphicsPipeline
{
	const GCRendererDevice* Device;
	const GCRendererShader* Shader;
} GCRendererGraphicsPipeline;

extern VkShaderModule GCRendererShader_GetVertexShaderModuleHandle(const GCRendererShader* const Shader);
extern VkShaderModule GCRendererShader_GetFragmentShaderModuleHandle(const GCRendererShader* const Shader);

static void GCRendererGraphicsPipeline_CreateGraphicsPipeline(GCRendererGraphicsPipeline* const GraphicsPipeline);

GCRendererGraphicsPipeline* GCRendererGraphicsPipeline_Create(const GCRendererDevice* const Device, const GCRendererShader* const Shader)
{
	GCRendererGraphicsPipeline* GraphicsPipeline = (GCRendererGraphicsPipeline*)GCMemory_Allocate(sizeof(GCRendererGraphicsPipeline));
	GraphicsPipeline->Device = Device;
	GraphicsPipeline->Shader = Shader;

	GCRendererGraphicsPipeline_CreateGraphicsPipeline(GraphicsPipeline);

	return GraphicsPipeline;
}

void GCRendererGraphicsPipeline_Destroy(GCRendererGraphicsPipeline* GraphicsPipeline)
{
	GCMemory_Free(GraphicsPipeline);
}

void GCRendererGraphicsPipeline_CreateGraphicsPipeline(GCRendererGraphicsPipeline* const GraphicsPipeline)
{
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
}