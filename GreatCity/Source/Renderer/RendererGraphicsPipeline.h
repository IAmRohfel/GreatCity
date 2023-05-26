#ifndef GC_RENDERER_RENDERER_GRAPHICS_PIPELINE_H
#define GC_RENDERER_RENDERER_GRAPHICS_PIPELINE_H

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

	typedef struct GCRendererGraphicsPipeline GCRendererGraphicsPipeline;
	typedef struct GCRendererDevice GCRendererDevice;
	typedef struct GCRendererSwapChain GCRendererSwapChain;
	typedef struct GCRendererUniformBuffer GCRendererUniformBuffer;
	typedef struct GCRendererShader GCRendererShader;

	typedef enum GCRendererGraphicsPipelineVertexInputAttributeFormat
	{
		GCRendererGraphicsPipelineVertexInputAttributeFormat_Float,
		GCRendererGraphicsPipelineVertexInputAttributeFormat_Vector2,
		GCRendererGraphicsPipelineVertexInputAttributeFormat_Vector3,
		GCRendererGraphicsPipelineVertexInputAttributeFormat_Vector4,
	} GCRendererGraphicsPipelineVertexInputAttributeFormat;

	typedef struct GCRendererGraphicsPipelineVertexInputAttribute
	{
		uint32_t Location;
		GCRendererGraphicsPipelineVertexInputAttributeFormat Format;
		uint32_t Offset;
	} GCRendererGraphicsPipelineVertexInputAttribute;

	typedef struct GCRendererGraphicsPipelineVertexInput
	{
		uint32_t Stride;
		GCRendererGraphicsPipelineVertexInputAttribute* Attributes;
		uint32_t AttributeCount;
	} GCRendererGraphicsPipelineVertexInput;

	GCRendererGraphicsPipeline* GCRendererGraphicsPipeline_Create(const GCRendererDevice* const Device, const GCRendererSwapChain* const SwapChain, const GCRendererGraphicsPipelineVertexInput* const VertexInput, const GCRendererUniformBuffer* const UniformBuffer, const GCRendererShader* const Shader);
	void GCRendererGraphicsPipeline_Destroy(GCRendererGraphicsPipeline* GraphicsPipeline);

#ifdef __cplusplus
}
#endif

#endif