#ifndef GC_RENDERER_RENDERER_COMMAND_LIST_H
#define GC_RENDERER_RENDERER_COMMAND_LIST_H

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

	typedef struct GCRendererCommandList GCRendererCommandList;
	typedef struct GCRendererDevice GCRendererDevice;
	typedef struct GCRendererSwapChain GCRendererSwapChain;
	typedef struct GCRendererGraphicsPipeline GCRendererGraphicsPipeline;
	typedef struct GCRendererFramebuffer GCRendererFramebuffer;

	typedef struct GCRendererCommandListRecordData GCRendererCommandListRecordData;
	typedef void(*GCRendererCommandListRecordFunction)(const GCRendererCommandListRecordData* const);

	GCRendererCommandList* GCRendererCommandList_Create(const GCRendererDevice* const Device, const GCRendererSwapChain* const SwapChain, const GCRendererGraphicsPipeline* const GraphicsPipeline, const GCRendererFramebuffer* const Framebuffer);
	void GCRendererCommandList_BeginRecord(const GCRendererCommandList* const CommandList);
	void GCRendererCommandList_BeginRenderPass(const GCRendererCommandList* const CommandList, const GCRendererCommandListRecordData* RecordData, const float* const ClearColor);
	void GCRendererCommandList_BindGraphicsPipeline(const GCRendererCommandList* const CommandList);
	void GCRendererCommandList_SetViewport(const GCRendererCommandList* const CommandList);
	void GCRendererCommandList_Draw(const GCRendererCommandList* const CommandList, const uint32_t VertexCount, const uint32_t FirstVertex);
	void GCRendererCommandList_EndRenderPass(const GCRendererCommandList* const CommandList);
	void GCRendererCommandList_EndRecord(const GCRendererCommandList* const CommandList);
	void GCRendererCommandList_SubmitAndPresent(const GCRendererCommandList* const CommandList, const GCRendererCommandListRecordFunction RecordFunction);
	void GCRendererCommandList_Destroy(GCRendererCommandList* CommandList);

#ifdef __cplusplus
}
#endif

#endif