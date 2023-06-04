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

#ifndef GC_RENDERER_RENDERER_COMMAND_LIST_H
#define GC_RENDERER_RENDERER_COMMAND_LIST_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C"
{
#endif

	typedef struct GCRendererCommandList GCRendererCommandList;
	typedef struct GCRendererDevice GCRendererDevice;
	typedef struct GCRendererSwapChain GCRendererSwapChain;
	typedef struct GCRendererVertexBuffer GCRendererVertexBuffer;
	typedef struct GCRendererIndexBuffer GCRendererIndexBuffer;
	typedef struct GCRendererUniformBuffer GCRendererUniformBuffer;
	typedef struct GCRendererGraphicsPipeline GCRendererGraphicsPipeline;
	typedef struct GCRendererFramebuffer GCRendererFramebuffer;

	typedef struct GCRendererCommandListRecordData GCRendererCommandListRecordData;
	typedef void(*GCRendererCommandListRecordFunction)(const GCRendererCommandListRecordData* const);
	typedef void(*GCRendererCommandListResizeCallbackFunction)(void);

	GCRendererCommandList* GCRendererCommandList_Create(const GCRendererDevice* const Device);
	void GCRendererCommandList_SetResize(GCRendererCommandList* const CommandList, const bool IsResized);
	void GCRendererCommandList_SetResizeCallback(GCRendererCommandList* const CommandList, const GCRendererCommandListResizeCallbackFunction ResizeCallbackFunction);
	void GCRendererCommandList_BeginRecord(const GCRendererCommandList* const CommandList);
	void GCRendererCommandList_BeginTextureRenderPass(const GCRendererCommandList* const CommandList, const GCRendererGraphicsPipeline* const GraphicsPipeline, const GCRendererFramebuffer* const Framebuffer, const float* const ClearColor);
	void GCRendererCommandList_BeginSwapChainRenderPass(const GCRendererCommandList* const CommandList, const GCRendererSwapChain* const SwapChain, const GCRendererGraphicsPipeline* const GraphicsPipeline, const GCRendererFramebuffer* const Framebuffer, const GCRendererCommandListRecordData* RecordData, const float* const ClearColor);
	void GCRendererCommandList_BindVertexBuffer(const GCRendererCommandList* const CommandList, const GCRendererVertexBuffer* const VertexBuffer);
	void GCRendererCommandList_BindIndexBuffer(const GCRendererCommandList* const CommandList, const GCRendererIndexBuffer* const IndexBuffer);
	void GCRendererCommandList_UpdateUniformBuffer(const GCRendererCommandList* const CommandList, const GCRendererUniformBuffer* const UniformBuffer, const GCRendererCommandListRecordData* RecordData, const void* const Data, const size_t DataSize);
	void GCRendererCommandList_BindGraphicsPipeline(const GCRendererCommandList* const CommandList, const GCRendererGraphicsPipeline* const GraphicsPipeline);
	void GCRendererCommandList_SetViewport(const GCRendererCommandList* const CommandList, const GCRendererFramebuffer* const Framebuffer);
	void GCRendererCommandList_Draw(const GCRendererCommandList* const CommandList, const uint32_t VertexCount, const uint32_t FirstVertex);
	void GCRendererCommandList_DrawIndexed(const GCRendererCommandList* const CommandList, const uint32_t IndexCount, const uint32_t FirstIndex);
	void GCRendererCommandList_EndTextureRenderPass(const GCRendererCommandList* const CommandList);
	void GCRendererCommandList_EndSwapChainRenderPass(const GCRendererCommandList* const CommandList);
	void GCRendererCommandList_EndRecord(const GCRendererCommandList* const CommandList);
	void GCRendererCommandList_SubmitAndPresent(GCRendererCommandList* const CommandList, const GCRendererSwapChain* const SwapChain, const GCRendererCommandListRecordFunction RecordFunction);
	void GCRendererCommandList_Destroy(GCRendererCommandList* CommandList);

#ifdef __cplusplus
}
#endif

#endif