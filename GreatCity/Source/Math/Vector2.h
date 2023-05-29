#ifndef GC_MATH_VECTOR_2_H
#define GC_MATH_VECTOR_2_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C"
{
#endif

	typedef struct GCVector2
	{
		float X, Y;
	} GCVector2;

	GCVector2 GCVector2_Create(const float X, const float Y);
	GCVector2 GCVector2_CreateZero(void);
	
	GCVector2 GCVector2_Add(const GCVector2 Vector1, const GCVector2 Vector2);
	GCVector2 GCVector2_Subtract(const GCVector2 Vector1, const GCVector2 Vector2);
	GCVector2 GCVector2_Multiply(const GCVector2 Vector1, const GCVector2 Vector2);
	GCVector2 GCVector2_MultiplyByScalar(const GCVector2 Vector, const float Scalar);
	GCVector2 GCVector2_Divide(const GCVector2 Vector1, const GCVector2 Vector2);

	float GCVector2_Dot(const GCVector2 Vector1, const GCVector2 Vector2);

	float GCVector2_Magnitude(const GCVector2 Vector);
	GCVector2 GCVector2_Normalize(const GCVector2 Vector);

	char* GCVector2_ToString(const GCVector2 Vector);
	bool GCVector2_IsEqual(const GCVector2 Vector1, const GCVector2 Vector2);

#ifdef __cplusplus
}
#endif

#endif