#ifndef GC_RENDERER_RENDERER_UNIFORM_BUFFER_H
#define GC_RENDERER_RENDERER_UNIFORM_BUFFER_H

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

	typedef struct GCRendererUniformBuffer GCRendererUniformBuffer;
	typedef struct GCRendererDevice GCRendererDevice;
	typedef struct GCRendererCommandList GCRendererCommandList;

	GCRendererUniformBuffer* GCRendererUniformBuffer_Create(const GCRendererDevice* const Device, const GCRendererCommandList* const CommandList, const size_t DataSize);
	void GCRendererUniformBuffer_Destroy(GCRendererUniformBuffer* UniformBuffer);

#ifdef __cplusplus
}
#endif

#endif