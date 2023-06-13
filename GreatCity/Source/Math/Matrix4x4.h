/*
	Copyright (C) 2023  Rohfel Adyaraka Christianugrah Puspoasmoro

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef GC_MATH_MATRIX_4X4_H
#define GC_MATH_MATRIX_4X4_H

#include "Math/Vector3.h"
#include "Math/Vector4.h"

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
	GCVector4 GCMatrix4x4_MultiplyByVector(const GCMatrix4x4* const Matrix, const GCVector4 Vector);

	float GCMatrix4x4_Determinant(const GCMatrix4x4* const Matrix);
	GCMatrix4x4 GCMatrix4x4_Inverse(const GCMatrix4x4* const Matrix);
	GCMatrix4x4 GCMatrix4x4_Transpose(const GCMatrix4x4* const Matrix);

	void GCMatrix4x4_Decompose(const GCMatrix4x4* const Matrix, GCVector3* const Translation, GCVector3* const Rotation, GCVector3* const Scale);

	char* GCMatrix4x4_ToString(const GCMatrix4x4* const Matrix);

#ifdef __cplusplus
}
#endif

#endif