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

#ifndef GC_MATH_VECTOR_3_H
#define GC_MATH_VECTOR_3_H

#include <stdbool.h>

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
	GCVector3 GCVector3_MultiplyByScalar(const GCVector3 Vector, const float Scalar);
	GCVector3 GCVector3_Divide(const GCVector3 Vector1, const GCVector3 Vector2);

	float GCVector3_Dot(const GCVector3 Vector1, const GCVector3 Vector2);
	GCVector3 GCVector3_Cross(const GCVector3 Vector1, const GCVector3 Vector2);

	float GCVector3_Magnitude(const GCVector3 Vector);
	GCVector3 GCVector3_Normalize(const GCVector3 Vector);

	char* GCVector3_ToString(const GCVector3 Vector);
	bool GCVector3_IsEqual(const GCVector3 Vector1, const GCVector3 Vector2);

#ifdef __cplusplus
}
#endif

#endif