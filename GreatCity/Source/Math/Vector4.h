#ifndef GC_MATH_VECTOR_4_H
#define GC_MATH_VECTOR_4_H

#ifdef __cplusplus
extern "C"
{
#endif

	typedef struct GCVector4
	{
		float X, Y, Z, W;
	} GCVector4;

	GCVector4 GCVector4_Create(const float X, const float Y, const float Z, const float W);
	GCVector4 GCVector4_CreateZero(void);

	GCVector4 GCVector4_Add(const GCVector4 Vector1, const GCVector4 Vector2);
	GCVector4 GCVector4_Subtract(const GCVector4 Vector1, const GCVector4 Vector2);
	GCVector4 GCVector4_Multiply(const GCVector4 Vector1, const GCVector4 Vector2);
	GCVector4 GCVector4_MultiplyByScalar(const GCVector4 Vector, const float Scalar);
	GCVector4 GCVector4_Divide(const GCVector4 Vector1, const GCVector4 Vector2);

	float GCVector4_Dot(const GCVector4 Vector1, const GCVector4 Vector2);

	float GCVector4_Magnitude(const GCVector4 Vector);
	GCVector4 GCVector4_Normalize(const GCVector4 Vector);

	char* GCVector4_ToString(const GCVector4 Vector);

#ifdef __cplusplus
}
#endif

#endif