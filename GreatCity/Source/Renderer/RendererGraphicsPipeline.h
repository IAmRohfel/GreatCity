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
	typedef struct GCRendererCommandList GCRendererCommandList;
	typedef struct GCRendererUniformBuffer GCRendererUniformBuffer;
	typedef struct GCRendererTexture2D GCRendererTexture2D;
	typedef struct GCRendererShader GCRendererShader;

	typedef enum GCRendererGraphicsPipelineVertexInputAttributeFormat
	{
		GCRendererGraphicsPipelineVertexInputAttributeFormat_Float,
		GCRendererGraphicsPipelineVertexInputAttributeFormat_Vector2,
		GCRendererGraphicsPipelineVertexInputAttributeFormat_Vector3,
		GCRendererGraphicsPipelineVertexInputAttributeFormat_Vector4,
	} GCRendererGraphicsPipelineVertexInputAttributeFormat;

	typedef struct GCRendererGraphicsPipelineVertexInputBinding
	{
		uint32_t Binding;
		uint32_t Stride;
	} GCRendererGraphicsPipelineVertexInputBinding;

	typedef struct GCRendererGraphicsPipelineVertexInputAttribute
	{
		uint32_t Location;
		GCRendererGraphicsPipelineVertexInputAttributeFormat Format;
		uint32_t Offset;
	} GCRendererGraphicsPipelineVertexInputAttribute;

	typedef struct GCRendererGraphicsPipelineVertexInput
	{
		GCRendererGraphicsPipelineVertexInputBinding* Bindings;
		uint32_t BindingCount;

		GCRendererGraphicsPipelineVertexInputAttribute* Attributes;
		uint32_t AttributeCount;
	} GCRendererGraphicsPipelineVertexInput;

	GCRendererGraphicsPipeline* GCRendererGraphicsPipeline_Create(const GCRendererDevice* const Device, const GCRendererSwapChain* const SwapChain, const GCRendererCommandList* const CommandList, const GCRendererGraphicsPipelineVertexInput* const VertexInput, const GCRendererUniformBuffer* const UniformBuffer, const GCRendererTexture2D* const* const Texture2Ds, const uint32_t Texture2DCount, const GCRendererShader* const Shader);
	void GCRendererGraphicsPipeline_Destroy(GCRendererGraphicsPipeline* GraphicsPipeline);

#ifdef __cplusplus
}
#endif

#endif