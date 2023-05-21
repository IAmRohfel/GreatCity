#ifndef GC_RENDERER_RENDERER_VERTEX_BUFFER_H
#define GC_RENDERER_RENDERER_VERTEX_BUFFER_H

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

	typedef struct GCRendererVertexBuffer GCRendererVertexBuffer;
	typedef struct GCRendererDevice GCRendererDevice;
	typedef struct GCRendererCommandList GCRendererCommandList;

	GCRendererVertexBuffer* GCRendererVertexBuffer_Create(const GCRendererDevice* const Device, const GCRendererCommandList* const CommandList, const void* const Vertices, const size_t VertexSize);
	void GCRendererVertexBuffer_Destroy(GCRendererVertexBuffer* VertexBuffer);

#ifdef __cplusplus
}
#endif

#endif