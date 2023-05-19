#include "Math/Vector3.h"
#include "Core/Memory/Allocator.h"

#include <stdio.h>
#include <math.h>
#include <stdint.h>

GCVector3 GCVector3_Create(const float X, const float Y, const float Z)
{
	GCVector3 Result;
	Result.X = X;
	Result.Y = Y;
	Result.Z = Z;

	return Result;
}

GCVector3 GCVector3_CreateZero(void)
{
	GCVector3 Result;
	Result.X = 0.0f;
	Result.Y = 0.0f;
	Result.Z = 0.0f;

	return Result;
}

GCVector3 GCVector3_Add(const GCVector3 Vector1, const GCVector3 Vector2)
{
	GCVector3 Result;
	Result.X = Vector1.X + Vector2.X;
	Result.Y = Vector1.Y + Vector2.Y;
	Result.Z = Vector1.Z + Vector2.Z;

	return Result;
}

GCVector3 GCVector3_Subtract(const GCVector3 Vector1, const GCVector3 Vector2)
{
	GCVector3 Result;
	Result.X = Vector1.X - Vector2.X;
	Result.Y = Vector1.Y - Vector2.Y;
	Result.Z = Vector1.Z - Vector2.Z;

	return Result;
}

GCVector3 GCVector3_Multiply(const GCVector3 Vector1, const GCVector3 Vector2)
{
	GCVector3 Result;
	Result.X = Vector1.X * Vector2.X;
	Result.Y = Vector1.Y * Vector2.Y;
	Result.Z = Vector1.Z * Vector2.Z;

	return Result;
}

GCVector3 GCVector3_Divide(const GCVector3 Vector1, const GCVector3 Vector2)
{
	GCVector3 Result;
	Result.X = Vector1.X / Vector2.X;
	Result.Y = Vector1.Y / Vector2.Y;
	Result.Z = Vector1.Z / Vector2.Z;

	return Result;
}

float GCVector3_Dot(const GCVector3 Vector1, const GCVector3 Vector2)
{
	return Vector1.X * Vector2.X + Vector1.Y * Vector2.Y + Vector1.Z * Vector2.Z;
}

GCVector3 GCVector3_Cross(const GCVector3 Vector1, const GCVector3 Vector2)
{
	GCVector3 Result;
	Result.X = Vector1.Y * Vector2.Z - Vector1.Z * Vector2.Y;
	Result.Y = Vector1.Z * Vector2.X - Vector1.X * Vector2.Z;
	Result.Z = Vector1.X * Vector2.Y - Vector1.Y * Vector2.X;

	return Result;
}

float GCVector3_Magnitude(const GCVector3 Vector)
{
	return sqrtf(Vector.X * Vector.X + Vector.Y * Vector.Y + Vector.Z * Vector.Z);
}

GCVector3 GCVector3_Normalize(const GCVector3 Vector)
{
	const float Magnitude = GCVector3_Magnitude(Vector);

	GCVector3 Result;
	Result.X = Vector.X / Magnitude;
	Result.Y = Vector.Y / Magnitude;
	Result.Z = Vector.Z / Magnitude;

	return Result;
}

char* GCVector3_ToString(const GCVector3 Vector)
{
	const int32_t Length = snprintf(NULL, 0, "(%f, %f, %f)", Vector.X, Vector.Y, Vector.Z) + 1;

	char* Buffer = (char*)GCMemory_Allocate(Length * sizeof(char));
	snprintf(Buffer, Length, "(%f, %f, %f)", Vector.X, Vector.Y, Vector.Z);

	return Buffer;
}
