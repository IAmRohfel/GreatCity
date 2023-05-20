#include "Math/Quaternion.h"
#include "Core/Memory/Allocator.h"

#include <stdio.h>
#include <math.h>
#include <stdint.h>

GCQuaternion GCQuaternion_Create(const float W, const float X, const float Y, const float Z)
{
	GCQuaternion Result;
	Result.W = W;
	Result.X = X;
	Result.Y = Y;
	Result.Z = Z;

	return Result;
}

GCQuaternion GCQuaternion_CreateZero(void)
{
	GCQuaternion Result;
	Result.W = 0.0f;
	Result.X = 0.0f;
	Result.Y = 0.0f;
	Result.Z = 0.0f;

	return Result;
}

GCQuaternion GCQuaternion_CreateUnit(void)
{
	GCQuaternion Result;
	Result.W = 1.0f;
	Result.X = 0.0f;
	Result.Y = 0.0f;
	Result.Z = 0.0f;

	return Result;
}

GCQuaternion GCQuaternion_CreateFromEulerAngles(const float Pitch, const float Yaw, const float Roll)
{
	GCQuaternion Result;

	const float CosinePitch = cosf(Pitch / 2.0f);
	const float CosineYaw = cosf(Yaw / 2.0f);
	const float CosineRoll = cosf(Roll / 2.0f);
	const float SinePitch = sinf(Pitch / 2.0f);
	const float SineYaw = sinf(Yaw / 2.0f);
	const float SineRoll = sinf(Roll / 2.0f);

	Result.W = CosinePitch * CosineYaw * CosineRoll + SinePitch * SineYaw * SineRoll;
	Result.X = SinePitch * CosineYaw * CosineRoll - CosinePitch * SineYaw * SineRoll;
	Result.Y = CosinePitch * SineYaw * CosineRoll + SinePitch * CosineYaw * SineRoll;
	Result.Z = CosinePitch * CosineYaw * SineRoll - SinePitch * SineYaw * CosineRoll;

	return Result;
}

GCMatrix4x4 GCQuaternion_ToRotationMatrix(const GCQuaternion Quaternion)
{
	GCMatrix4x4 Result;

	Result.Data[0][0] = 2.0f * (Quaternion.W * Quaternion.W + Quaternion.X * Quaternion.X) - 1.0f;
	Result.Data[0][1] = 2.0f * (Quaternion.X * Quaternion.Y + Quaternion.W * Quaternion.Z);
	Result.Data[0][2] = 2.0f * (Quaternion.X * Quaternion.Z - Quaternion.W * Quaternion.Y);
	Result.Data[0][3] = 0.0f;

	Result.Data[1][0] = 2.0f * (Quaternion.X * Quaternion.Y - Quaternion.W * Quaternion.Z);
	Result.Data[1][1] = 2.0f * (Quaternion.W * Quaternion.W + Quaternion.Y * Quaternion.Y) - 1.0f;
	Result.Data[1][2] = 2.0f * (Quaternion.Y * Quaternion.Z + Quaternion.W * Quaternion.X);
	Result.Data[1][3] = 0.0f;

	Result.Data[2][0] = 2.0f * (Quaternion.X * Quaternion.Z + Quaternion.W * Quaternion.Y);
	Result.Data[2][1] = 2.0f * (Quaternion.Y * Quaternion.Z - Quaternion.W * Quaternion.X);
	Result.Data[2][2] = 2.0f * (Quaternion.W * Quaternion.W + Quaternion.Z * Quaternion.Z) - 1.0f;
	Result.Data[2][3] = 0.0f;

	Result.Data[3][0] = 0.0f;
	Result.Data[3][1] = 0.0f;
	Result.Data[3][2] = 0.0f;
	Result.Data[3][3] = 1.0f;

	return Result;
}

GCQuaternion GCQuaternion_Multiply(const GCQuaternion Quaternion1, const GCQuaternion Quaternion2)
{
	GCQuaternion Result;
	Result.W = -Quaternion1.X * Quaternion2.X - Quaternion1.Y * Quaternion2.Y - Quaternion1.Z * Quaternion2.Z + Quaternion1.W * Quaternion2.W;
	Result.X = Quaternion1.X * Quaternion2.W + Quaternion1.Y * Quaternion2.Z - Quaternion1.Z * Quaternion2.Y + Quaternion1.W * Quaternion2.X;
	Result.Y = -Quaternion1.X * Quaternion2.Z + Quaternion1.Y * Quaternion2.W + Quaternion1.Z * Quaternion2.X + Quaternion1.W * Quaternion2.Y;
	Result.Z = Quaternion1.X * Quaternion2.Y - Quaternion1.Y * Quaternion2.X + Quaternion1.Z * Quaternion2.W + Quaternion1.W * Quaternion2.Z;

	return Result;
}

GCQuaternion GCQuaternion_Conjugate(const GCQuaternion Quaternion)
{
	GCQuaternion Result;
	Result.W = Quaternion.W;
	Result.X = -Quaternion.X;
	Result.Y = -Quaternion.Y;
	Result.Z = -Quaternion.Z;

	return Result;
}

float GCQuaternion_Magnitude(const GCQuaternion Quaternion)
{
	return sqrtf(Quaternion.W * Quaternion.W + Quaternion.X * Quaternion.X + Quaternion.Y * Quaternion.Y + Quaternion.Z * Quaternion.Z);
}

GCQuaternion GCQuaternion_Normalize(const GCQuaternion Quaternion)
{
	const float Magnitude = GCQuaternion_Magnitude(Quaternion);

	GCQuaternion Result;
	Result.W = Quaternion.W / Magnitude;
	Result.X = Quaternion.X / Magnitude;
	Result.Y = Quaternion.Y / Magnitude;
	Result.Z = Quaternion.Z / Magnitude;

	return Result;
}

GCVector3 GCQuaternion_RotateVector(const GCQuaternion Quaternion, const GCVector3 Vector)
{
	const float Scalar = Quaternion.W;
	const GCVector3 QuaternionAsVector = GCVector3_Create(Quaternion.X, Quaternion.Y, Quaternion.Z);

	GCVector3 Result = GCVector3_MultiplyByScalar(QuaternionAsVector, 2.0f * GCVector3_Dot(QuaternionAsVector, Vector));
	Result = GCVector3_Add(Result, GCVector3_MultiplyByScalar(Vector, Scalar * Scalar - GCVector3_Dot(QuaternionAsVector, QuaternionAsVector)));
	Result = GCVector3_Add(Result, GCVector3_MultiplyByScalar(GCVector3_Cross(QuaternionAsVector, Vector), 2.0f * Scalar));

	return Result;
}

char* GCQuaternion_ToString(const GCQuaternion Quaternion)
{
	const int32_t Length = snprintf(NULL, 0, "(%f, %f, %f, %f)", Quaternion.W, Quaternion.X, Quaternion.Y, Quaternion.Z) + 1;

	char* Buffer = (char*)GCMemory_Allocate(Length * sizeof(char));
	snprintf(Buffer, Length, "(%f, %f, %f, %f)", Quaternion.W, Quaternion.X, Quaternion.Y, Quaternion.Z);

	return Buffer;
}
