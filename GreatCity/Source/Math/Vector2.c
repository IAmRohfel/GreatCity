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

#include "Math/Vector2.h"
#include "Core/Memory/Allocator.h"

#include <stdio.h>
#include <math.h>
#include <stdint.h>
#include <stdbool.h>

GCVector2 GCVector2_Create(const float X, const float Y)
{
	GCVector2 Result;
	Result.X = X;
	Result.Y = Y;

	return Result;
}

GCVector2 GCVector2_CreateZero(void)
{
	GCVector2 Result;
	Result.X = 0.0f;
	Result.Y = 0.0f;

	return Result;
}

GCVector2 GCVector2_Add(const GCVector2 Vector1, const GCVector2 Vector2)
{
	GCVector2 Result;
	Result.X = Vector1.X + Vector2.X;
	Result.Y = Vector1.Y + Vector2.Y;

	return Result;
}

GCVector2 GCVector2_Subtract(const GCVector2 Vector1, const GCVector2 Vector2)
{
	GCVector2 Result;
	Result.X = Vector1.X - Vector2.X;
	Result.Y = Vector1.Y - Vector2.Y;

	return Result;
}

GCVector2 GCVector2_Multiply(const GCVector2 Vector1, const GCVector2 Vector2)
{
	GCVector2 Result;
	Result.X = Vector1.X * Vector2.X;
	Result.Y = Vector1.Y * Vector2.Y;

	return Result;
}

GCVector2 GCVector2_MultiplyByScalar(const GCVector2 Vector, const float Scalar)
{
	GCVector2 Result;
	Result.X = Vector.X * Scalar;
	Result.Y = Vector.Y * Scalar;

	return Result;
}

GCVector2 GCVector2_Divide(const GCVector2 Vector1, const GCVector2 Vector2)
{
	GCVector2 Result;
	Result.X = Vector1.X / Vector2.X;
	Result.Y = Vector1.Y / Vector2.Y;

	return Result;
}

float GCVector2_Dot(const GCVector2 Vector1, const GCVector2 Vector2)
{
	return Vector1.X * Vector2.X + Vector1.Y * Vector2.Y;
}

float GCVector2_Magnitude(const GCVector2 Vector)
{
	return sqrtf(Vector.X * Vector.X + Vector.Y * Vector.Y);
}

GCVector2 GCVector2_Normalize(const GCVector2 Vector)
{
	const float Magnitude = GCVector2_Magnitude(Vector);

	GCVector2 Result;
	Result.X = Vector.X / Magnitude;
	Result.Y = Vector.Y / Magnitude;

	return Result;
}

char* GCVector2_ToString(const GCVector2 Vector)
{
	const int32_t Length = snprintf(NULL, 0, "(%f, %f)", Vector.X, Vector.Y) + 1;

	char* Buffer = (char*)GCMemory_Allocate(Length * sizeof(char));
	snprintf(Buffer, Length, "(%f, %f)", Vector.X, Vector.Y);

	return Buffer;
}

bool GCVector2_IsEqual(const GCVector2 Vector1, const GCVector2 Vector2)
{
	return Vector1.X == Vector2.X && Vector1.Y == Vector2.Y;
}
