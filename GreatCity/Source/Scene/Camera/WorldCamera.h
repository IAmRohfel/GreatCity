#ifndef GC_SCENE_CAMERA_WORLD_CAMERA_H
#define GC_SCENE_CAMERA_WORLD_CAMERA_H

#include "Math/Matrix4x4.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

	typedef struct GCWorldCamera GCWorldCamera;
	typedef struct GCEvent GCEvent;

	GCWorldCamera* GCWorldCamera_Create(const float FoV, const float AspectRatio, const float Near, const float Far);
	void GCWorldCamera_Update(GCWorldCamera* const WorldCamera);
	void GCWorldCemera_OnEvent(GCWorldCamera* const WorldCamera, GCEvent* const Event);
	void GCWorldCamera_SetSize(GCWorldCamera* const WorldCamera, const uint32_t Width, const uint32_t Height);
	GCMatrix4x4 GCWorldCamera_GetViewProjectionMatrix(const GCWorldCamera* const WorldCamera);

#ifdef __cplusplus
}
#endif

#endif