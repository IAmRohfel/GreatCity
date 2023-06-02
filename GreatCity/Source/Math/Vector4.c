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

#include "Math/Vector4.h"
#include "Core/Memory/Allocator.h"

#include <stdio.h>
#include <math.h>
#include <stdint.h>
#include <stdbool.h>

GCVector4 GCVector4_Create(const float X, const float Y, const float Z, const float W)
{
	GCVector4 Result;
	Result.X = X;
	Result.Y = Y;
	Result.Z = Z;
	Result.W = W;

	return Result;
}

GCVector4 GCVector4_CreateZero(void)
{
	GCVector4 Result;
	Result.X = 0.0f;
	Result.Y = 0.0f;
	Result.Z = 0.0f;
	Result.W = 0.0f;

	return Result;
}

GCVector4 GCVector4_Add(const GCVector4 Vector1, const GCVector4 Vector2)
{
	GCVector4 Result;
	Result.X = Vector1.X + Vector2.X;
	Result.Y = Vector1.Y + Vector2.Y;
	Result.Z = Vector1.Z + Vector2.Z;
	Result.W = Vector1.W + Vector2.W;

	return Result;
}

GCVector4 GCVector4_Subtract(const GCVector4 Vector1, const GCVector4 Vector2)
{
	GCVector4 Result;
	Result.X = Vector1.X - Vector2.X;
	Result.Y = Vector1.Y - Vector2.Y;
	Result.Z = Vector1.Z - Vector2.Z;
	Result.W = Vector1.W - Vector2.W;

	return Result;
}

GCVector4 GCVector4_Multiply(const GCVector4 Vector1, const GCVector4 Vector2)
{
	GCVector4 Result;
	Result.X = Vector1.X * Vector2.X;
	Result.Y = Vector1.Y * Vector2.Y;
	Result.Z = Vector1.Z * Vector2.Z;
	Result.W = Vector1.W * Vector2.W;

	return Result;
}

GCVector4 GCVector4_MultiplyByScalar(const GCVector4 Vector, const float Scalar)
{
	GCVector4 Result;
	Result.X = Vector.X * Scalar;
	Result.Y = Vector.Y * Scalar;
	Result.Z = Vector.Z * Scalar;
	Result.W = Vector.W * Scalar;

	return Result;
}

GCVector4 GCVector4_Divide(const GCVector4 Vector1, const GCVector4 Vector2)
{
	GCVector4 Result;
	Result.X = Vector1.X / Vector2.X;
	Result.Y = Vector1.Y / Vector2.Y;
	Result.Z = Vector1.Z / Vector2.Z;
	Result.W = Vector1.W / Vector2.W;

	return Result;
}

float GCVector4_Dot(const GCVector4 Vector1, const GCVector4 Vector2)
{
	return Vector1.X * Vector2.X + Vector1.Y * Vector2.Y + Vector1.Z * Vector2.Z + Vector1.W * Vector2.W;
}

float GCVector4_Magnitude(const GCVector4 Vector)
{
	return sqrtf(Vector.X * Vector.X + Vector.Y * Vector.Y + Vector.Z * Vector.Z + Vector.W * Vector.W);
}

GCVector4 GCVector4_Normalize(const GCVector4 Vector)
{
	const float Magnitude = GCVector4_Magnitude(Vector);

	GCVector4 Result;
	Result.X = Vector.X / Magnitude;
	Result.Y = Vector.Y / Magnitude;
	Result.Z = Vector.Z / Magnitude;
	Result.W = Vector.W / Magnitude;

	return Result;
}

char* GCVector4_ToString(const GCVector4 Vector)
{
	const int32_t Length = snprintf(NULL, 0, "(%f, %f, %f, %f)", Vector.X, Vector.Y, Vector.Z, Vector.W) + 1;

	char* Buffer = (char*)GCMemory_Allocate(Length * sizeof(char));
	snprintf(Buffer, Length, "(%f, %f, %f, %f)", Vector.X, Vector.Y, Vector.Z, Vector.W);

	return Buffer;
}

bool GCVector4_IsEqual(const GCVector4 Vector1, const GCVector4 Vector2)
{
	return Vector1.X == Vector2.X && Vector1.Y == Vector2.Y && Vector1.Z == Vector2.Z && Vector1.W == Vector2.W;
}
