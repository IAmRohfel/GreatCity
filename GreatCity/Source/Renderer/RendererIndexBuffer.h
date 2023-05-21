#ifndef GC_RENDERER_RENDERER_INDEX_BUFFER_H
#define GC_RENDERER_RENDERER_INDEX_BUFFER_H

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

	typedef struct GCRendererIndexBuffer GCRendererIndexBuffer;
	typedef struct GCRendererDevice GCRendererDevice;
	typedef struct GCRendererCommandList GCRendererCommandList;

	GCRendererIndexBuffer* GCRendererIndexBuffer_Create(const GCRendererDevice* const Device, const GCRendererCommandList* const CommandList, const uint32_t* const Indices, const size_t IndexSize);
	void GCRendererIndexBuffer_Destroy(GCRendererIndexBuffer* IndexBuffer);

#ifdef __cplusplus
}
#endif

#endif