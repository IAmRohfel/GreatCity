#ifndef GC_RENDERER_RENDERER_MODEL_H
#define GC_RENDERER_RENDERER_MODEL_H

#include "Renderer/Renderer.h"
#include "Math/Matrix4x4.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

	typedef struct GCRendererVertexBuffer GCRendererVertexBuffer;
	typedef struct GCRendererIndexBuffer GCRendererIndexBuffer;

	typedef struct GCRendererModel
	{
		GCRendererVertexBuffer* VertexBuffer;
		GCRendererVertex* Vertices;
		uint32_t VertexCount;

		GCRendererIndexBuffer* IndexBuffer;
		uint32_t IndexCount;
	} GCRendererModel;

	GCRendererModel* GCRendererModel_CreateFromFile(const char* const ModelPath, const char* const MaterialPath);
	GCRendererModel* GCRendererModel_CreateFromFiles(const char* const* const ModelPaths, const char* const* const MaterialPaths, const uint32_t ModelCount);
	void GCRendererModel_SetTransform(GCRendererModel* const Model, const GCMatrix4x4* const Transform);
	void GCRendererModel_Destroy(GCRendererModel* Model);

#ifdef __cplusplus
}
#endif

#endif