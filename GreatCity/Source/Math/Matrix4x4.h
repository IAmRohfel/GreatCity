#ifndef GC_MATH_MATRIX_4X4_H
#define GC_MATH_MATRIX_4X4_H

#include "Math/Vector3.h"

#ifdef __cplusplus
extern "C"
{
#endif

	typedef struct GCMatrix4x4
	{
		float Data[4][4];
	} GCMatrix4x4;

	GCMatrix4x4 GCMatrix4x4_Create(const float* const Data);
	GCMatrix4x4 GCMatrix4x4_CreateZero(void);
	GCMatrix4x4 GCMatrix4x4_CreateIdentity(void);

	GCMatrix4x4 GCMatrix4x4_CreateTranslation(const GCVector3 Translation);
	GCMatrix4x4 GCMatrix4x4_CreateRotation(const float Angle, const GCVector3 Axis);
	GCMatrix4x4 GCMatrix4x4_CreateScale(const GCVector3 Scale);

	GCMatrix4x4 GCMatrix4x4_CreatePerspective(const float FoV, const float AspectRatio, const float Near, const float Far);

	GCMatrix4x4 GCMatrix4x4_Multiply(const GCMatrix4x4* const Matrix1, const GCMatrix4x4* const Matrix2);
	GCMatrix4x4 GCMatrix4x4_MultiplyByScalar(const GCMatrix4x4* const Matrix, const float Scalar);

	float GCMatrix4x4_Determinant(const GCMatrix4x4* const Matrix);
	GCMatrix4x4 GCMatrix4x4_Inverse(const GCMatrix4x4* const Matrix);
	GCMatrix4x4 GCMatrix4x4_Transpose(const GCMatrix4x4* const Matrix);

	char* GCMatrix4x4_ToString(const GCMatrix4x4* const Matrix);

#ifdef __cplusplus
}
#endif

#endif