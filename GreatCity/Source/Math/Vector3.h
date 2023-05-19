#ifndef GC_MATH_VECTOR_3_H
#define GC_MATH_VECTOR_3_H

#ifdef __cplusplus
extern "C"
{
#endif

	typedef struct GCVector3
	{
		float X, Y, Z;
	} GCVector3;

	GCVector3 GCVector3_Create(const float X, const float Y, const float Z);
	GCVector3 GCVector3_CreateZero(void);

	GCVector3 GCVector3_Add(const GCVector3 Vector1, const GCVector3 Vector2);
	GCVector3 GCVector3_Subtract(const GCVector3 Vector1, const GCVector3 Vector2);
	GCVector3 GCVector3_Multiply(const GCVector3 Vector1, const GCVector3 Vector2);
	GCVector3 GCVector3_Divide(const GCVector3 Vector1, const GCVector3 Vector2);

	float GCVector3_Dot(const GCVector3 Vector1, const GCVector3 Vector2);
	GCVector3 GCVector3_Cross(const GCVector3 Vector1, const GCVector3 Vector2);

	float GCVector3_Magnitude(const GCVector3 Vector);
	GCVector3 GCVector3_Normalize(const GCVector3 Vector);

	char* GCVector3_ToString(const GCVector3 Vector);

#ifdef __cplusplus
}
#endif

#endif