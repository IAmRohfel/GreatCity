#ifndef GC_RENDERER_RENDERER_MODEL_H
#define GC_RENDERER_RENDERER_MODEL_H

#include "Renderer/Renderer.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

	typedef struct GCRendererModel
	{
		GCRendererVertex* Vertices;
		uint32_t VertexCount;

		uint32_t* Indices;
		uint32_t IndexCount;
	} GCRendererModel;

	GCRendererModel* GCRendererModel_CreateFromFile(const char* const* const ModelPaths, const char* const* const MaterialPaths, const uint32_t ModelCount);
	void GCRendererModel_Destroy(GCRendererModel* Model);

#ifdef __cplusplus
}
#endif

#endif