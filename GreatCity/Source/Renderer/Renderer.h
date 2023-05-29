#ifndef GC_RENDERER_RENDERER_H
#define GC_RENDERER_RENDERER_H

#include "Math/Vector2.h"
#include "Math/Vector3.h"
#include "Math/Vector4.h"

#ifdef __cplusplus
extern "C"
{
#endif

	typedef struct GCRendererVertex
	{
		GCVector3 Position;
		GCVector4 Color;
		GCVector2 TextureCoordinate;

	#ifdef __cplusplus
		bool operator==(const GCRendererVertex& OtherVertex) const noexcept
		{
			return GCVector3_IsEqual(Position, OtherVertex.Position) && GCVector4_IsEqual(Color, OtherVertex.Color) && GCVector2_IsEqual(TextureCoordinate, OtherVertex.TextureCoordinate);
		}
	#endif
	} GCRendererVertex;

	typedef struct GCWorldCamera GCWorldCamera;

	void GCRenderer_Initialize(const GCWorldCamera* const WorldCamera);
	void GCRenderer_Begin(void);
	void GCRenderer_Present(void);
	void GCRenderer_End(void);
	void GCRenderer_Resize(void);
	void GCRenderer_Terminate(void);

#ifdef __cplusplus
}
#endif

#endif