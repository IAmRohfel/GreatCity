#include "Math/Matrix4x4.h"
#include "Core/Memory/Allocator.h"

#include <stdio.h>
#include <math.h>
#include <stdint.h>

GCMatrix4x4 GCMatrix4x4_Create(const float* const Data)
{
	GCMatrix4x4 Result;

	Result.Data[0][0] = Data[0];
	Result.Data[0][1] = Data[1];
	Result.Data[0][2] = Data[2];
	Result.Data[0][3] = Data[3];

	Result.Data[1][0] = Data[4];
	Result.Data[1][1] = Data[5];
	Result.Data[1][2] = Data[6];
	Result.Data[1][3] = Data[7];

	Result.Data[2][0] = Data[8];
	Result.Data[2][1] = Data[9];
	Result.Data[2][2] = Data[10];
	Result.Data[2][3] = Data[11];

	Result.Data[3][0] = Data[12];
	Result.Data[3][1] = Data[13];
	Result.Data[3][2] = Data[14];
	Result.Data[3][3] = Data[15];

	return Result;
}

GCMatrix4x4 GCMatrix4x4_CreateZero(void)
{
	GCMatrix4x4 Result;

	Result.Data[0][0] = 0.0f;
	Result.Data[0][1] = 0.0f;
	Result.Data[0][2] = 0.0f;
	Result.Data[0][3] = 0.0f;

	Result.Data[1][0] = 0.0f;
	Result.Data[1][1] = 0.0f;
	Result.Data[1][2] = 0.0f;
	Result.Data[1][3] = 0.0f;

	Result.Data[2][0] = 0.0f;
	Result.Data[2][1] = 0.0f;
	Result.Data[2][2] = 0.0f;
	Result.Data[2][3] = 0.0f;

	Result.Data[3][0] = 0.0f;
	Result.Data[3][1] = 0.0f;
	Result.Data[3][2] = 0.0f;
	Result.Data[3][3] = 0.0f;

	return Result;
}

GCMatrix4x4 GCMatrix4x4_CreateIdentity(void)
{
	GCMatrix4x4 Result;

	Result.Data[0][0] = 1.0f;
	Result.Data[0][1] = 0.0f;
	Result.Data[0][2] = 0.0f;
	Result.Data[0][3] = 0.0f;

	Result.Data[1][0] = 0.0f;
	Result.Data[1][1] = 1.0f;
	Result.Data[1][2] = 0.0f;
	Result.Data[1][3] = 0.0f;

	Result.Data[2][0] = 0.0f;
	Result.Data[2][1] = 0.0f;
	Result.Data[2][2] = 1.0f;
	Result.Data[2][3] = 0.0f;

	Result.Data[3][0] = 0.0f;
	Result.Data[3][1] = 0.0f;
	Result.Data[3][2] = 0.0f;
	Result.Data[3][3] = 1.0f;

	return Result;
}

GCMatrix4x4 GCMatrix4x4_CreateTranslation(const GCVector3 Translation)
{
	GCMatrix4x4 Result;

	Result.Data[0][0] = 1.0f;
	Result.Data[0][1] = 0.0f;
	Result.Data[0][2] = 0.0f;
	Result.Data[0][3] = 0.0f;

	Result.Data[1][0] = 0.0f;
	Result.Data[1][1] = 1.0f;
	Result.Data[1][2] = 0.0f;
	Result.Data[1][3] = 0.0f;

	Result.Data[2][0] = 0.0f;
	Result.Data[2][1] = 0.0f;
	Result.Data[2][2] = 1.0f;
	Result.Data[2][3] = 0.0f;

	Result.Data[3][0] = Translation.X;
	Result.Data[3][1] = Translation.Y;
	Result.Data[3][2] = Translation.Z;
	Result.Data[3][3] = 1.0f;

	return Result;
}

GCMatrix4x4 GCMatrix4x4_CreateRotation(const float Angle, const GCVector3 Axis)
{
	GCMatrix4x4 Result;

	const float CosineAngle = cosf(Angle);
	const float OneMinusCosineAngle = 1.0f - CosineAngle;
	const float SineAngle = sinf(Angle);

	Result.Data[0][0] = CosineAngle + (Axis.X * Axis.X) * OneMinusCosineAngle;
	Result.Data[0][1] = Axis.Y * Axis.X * OneMinusCosineAngle + Axis.Z * SineAngle;
	Result.Data[0][2] = Axis.Z * Axis.X * OneMinusCosineAngle - Axis.Y * SineAngle;
	Result.Data[0][3] = 0.0f;

	Result.Data[1][0] = Axis.X * Axis.Y * OneMinusCosineAngle - Axis.Z * SineAngle;
	Result.Data[1][1] = CosineAngle + (Axis.Y * Axis.Y) * OneMinusCosineAngle;
	Result.Data[1][2] = Axis.Z * Axis.Y * OneMinusCosineAngle + Axis.X * SineAngle;
	Result.Data[1][3] = 0.0f;

	Result.Data[2][0] = Axis.X * Axis.Z * OneMinusCosineAngle + Axis.Y * SineAngle;
	Result.Data[2][1] = Axis.Y * Axis.Z * OneMinusCosineAngle - Axis.X * SineAngle;
	Result.Data[2][2] = CosineAngle + (Axis.Z * Axis.Z) * OneMinusCosineAngle;
	Result.Data[2][3] = 0.0f;

	Result.Data[3][0] = 0.0f;
	Result.Data[3][1] = 0.0f;
	Result.Data[3][2] = 0.0f;
	Result.Data[3][3] = 1.0f;

	return Result;
}

GCMatrix4x4 GCMatrix4x4_CreateScale(const GCVector3 Scale)
{
	GCMatrix4x4 Result;

	Result.Data[0][0] = Scale.X;
	Result.Data[0][1] = 0.0f;
	Result.Data[0][2] = 0.0f;
	Result.Data[0][3] = 0.0f;

	Result.Data[1][0] = 0.0f;
	Result.Data[1][1] = Scale.Y;
	Result.Data[1][2] = 0.0f;
	Result.Data[1][3] = 0.0f;

	Result.Data[2][0] = 0.0f;
	Result.Data[2][1] = 0.0f;
	Result.Data[2][2] = Scale.Z;
	Result.Data[2][3] = 0.0f;

	Result.Data[3][0] = 0.0f;
	Result.Data[3][1] = 0.0f;
	Result.Data[3][2] = 0.0f;
	Result.Data[3][3] = 1.0f;

	return Result;
}

GCMatrix4x4 GCMatrix4x4_CreatePerspective(const float FoV, const float AspectRatio, const float Near, const float Far)
{
	GCMatrix4x4 Result;

	Result.Data[0][0] = 1.0f / (AspectRatio * tanf(FoV / 2.0f));
	Result.Data[0][1] = 0.0f;
	Result.Data[0][2] = 0.0f;
	Result.Data[0][3] = 0.0f;

	Result.Data[1][0] = 0.0f;
	Result.Data[1][1] = 1.0f / tanf(FoV / 2.0f);
	Result.Data[1][2] = 0.0f;
	Result.Data[1][3] = 0.0f;

	Result.Data[2][0] = 0.0f;
	Result.Data[2][1] = 0.0f;
	Result.Data[2][2] = -((Far + Near) / (Far - Near));
	Result.Data[2][3] = -1.0f;

	Result.Data[3][0] = 0.0f;
	Result.Data[3][1] = 0.0f;
	Result.Data[3][2] = -((2.0f * Far * Near) / (Far - Near));
	Result.Data[3][3] = 0.0f;

	return Result;
}

GCMatrix4x4 GCMatrix4x4_Multiply(const GCMatrix4x4* const Matrix1, const GCMatrix4x4* const Matrix2)
{
	GCMatrix4x4 Result;

	Result.Data[0][0] = Matrix1->Data[0][0] * Matrix2->Data[0][0] + Matrix1->Data[1][0] * Matrix2->Data[0][1] + Matrix1->Data[2][0] * Matrix2->Data[0][2] + Matrix1->Data[3][0] * Matrix2->Data[0][3];
	Result.Data[1][0] = Matrix1->Data[0][0] * Matrix2->Data[1][0] + Matrix1->Data[1][0] * Matrix2->Data[1][1] + Matrix1->Data[2][0] * Matrix2->Data[1][2] + Matrix1->Data[3][0] * Matrix2->Data[1][3];
	Result.Data[2][0] = Matrix1->Data[0][0] * Matrix2->Data[2][0] + Matrix1->Data[1][0] * Matrix2->Data[2][1] + Matrix1->Data[2][0] * Matrix2->Data[2][2] + Matrix1->Data[3][0] * Matrix2->Data[2][3];
	Result.Data[3][0] = Matrix1->Data[0][0] * Matrix2->Data[3][0] + Matrix1->Data[1][0] * Matrix2->Data[3][1] + Matrix1->Data[2][0] * Matrix2->Data[3][2] + Matrix1->Data[3][0] * Matrix2->Data[3][3];

	Result.Data[0][1] = Matrix1->Data[0][1] * Matrix2->Data[0][0] + Matrix1->Data[1][1] * Matrix2->Data[0][1] + Matrix1->Data[2][1] * Matrix2->Data[0][2] + Matrix1->Data[3][1] * Matrix2->Data[0][3];
	Result.Data[1][1] = Matrix1->Data[0][1] * Matrix2->Data[1][0] + Matrix1->Data[1][1] * Matrix2->Data[1][1] + Matrix1->Data[2][1] * Matrix2->Data[1][2] + Matrix1->Data[3][1] * Matrix2->Data[1][3];
	Result.Data[2][1] = Matrix1->Data[0][1] * Matrix2->Data[2][0] + Matrix1->Data[1][1] * Matrix2->Data[2][1] + Matrix1->Data[2][1] * Matrix2->Data[2][2] + Matrix1->Data[3][1] * Matrix2->Data[2][3];
	Result.Data[3][1] = Matrix1->Data[0][1] * Matrix2->Data[3][0] + Matrix1->Data[1][1] * Matrix2->Data[3][1] + Matrix1->Data[2][1] * Matrix2->Data[3][2] + Matrix1->Data[3][1] * Matrix2->Data[3][3];

	Result.Data[0][2] = Matrix1->Data[0][2] * Matrix2->Data[0][0] + Matrix1->Data[1][2] * Matrix2->Data[0][1] + Matrix1->Data[2][2] * Matrix2->Data[0][2] + Matrix1->Data[3][2] * Matrix2->Data[0][3];
	Result.Data[1][2] = Matrix1->Data[0][2] * Matrix2->Data[1][0] + Matrix1->Data[1][2] * Matrix2->Data[1][1] + Matrix1->Data[2][2] * Matrix2->Data[1][2] + Matrix1->Data[3][2] * Matrix2->Data[1][3];
	Result.Data[2][2] = Matrix1->Data[0][2] * Matrix2->Data[2][0] + Matrix1->Data[1][2] * Matrix2->Data[2][1] + Matrix1->Data[2][2] * Matrix2->Data[2][2] + Matrix1->Data[3][2] * Matrix2->Data[2][3];
	Result.Data[3][2] = Matrix1->Data[0][2] * Matrix2->Data[3][0] + Matrix1->Data[1][2] * Matrix2->Data[3][1] + Matrix1->Data[2][2] * Matrix2->Data[3][2] + Matrix1->Data[3][2] * Matrix2->Data[3][3];

	Result.Data[0][3] = Matrix1->Data[0][3] * Matrix2->Data[0][0] + Matrix1->Data[1][3] * Matrix2->Data[0][1] + Matrix1->Data[2][3] * Matrix2->Data[0][2] + Matrix1->Data[3][3] * Matrix2->Data[0][3];
	Result.Data[1][3] = Matrix1->Data[0][3] * Matrix2->Data[1][0] + Matrix1->Data[1][3] * Matrix2->Data[1][1] + Matrix1->Data[2][3] * Matrix2->Data[1][2] + Matrix1->Data[3][3] * Matrix2->Data[1][3];
	Result.Data[2][3] = Matrix1->Data[0][3] * Matrix2->Data[2][0] + Matrix1->Data[1][3] * Matrix2->Data[2][1] + Matrix1->Data[2][3] * Matrix2->Data[2][2] + Matrix1->Data[3][3] * Matrix2->Data[2][3];
	Result.Data[3][3] = Matrix1->Data[0][3] * Matrix2->Data[3][0] + Matrix1->Data[1][3] * Matrix2->Data[3][1] + Matrix1->Data[2][3] * Matrix2->Data[3][2] + Matrix1->Data[3][3] * Matrix2->Data[3][3];

	return Result;
}

GCMatrix4x4 GCMatrix4x4_MultiplyByScalar(const GCMatrix4x4* const Matrix, const float Scalar)
{
	GCMatrix4x4 Result;

	Result.Data[0][0] = Matrix->Data[0][0] * Scalar;
	Result.Data[0][1] = Matrix->Data[0][1] * Scalar;
	Result.Data[0][2] = Matrix->Data[0][2] * Scalar;
	Result.Data[0][3] = Matrix->Data[0][3] * Scalar;

	Result.Data[1][0] = Matrix->Data[1][0] * Scalar;
	Result.Data[1][1] = Matrix->Data[1][1] * Scalar;
	Result.Data[1][2] = Matrix->Data[1][2] * Scalar;
	Result.Data[1][3] = Matrix->Data[1][3] * Scalar;

	Result.Data[2][0] = Matrix->Data[2][0] * Scalar;
	Result.Data[2][1] = Matrix->Data[2][1] * Scalar;
	Result.Data[2][2] = Matrix->Data[2][2] * Scalar;
	Result.Data[2][3] = Matrix->Data[2][3] * Scalar;

	Result.Data[3][0] = Matrix->Data[3][0] * Scalar;
	Result.Data[3][1] = Matrix->Data[3][1] * Scalar;
	Result.Data[3][2] = Matrix->Data[3][2] * Scalar;
	Result.Data[3][3] = Matrix->Data[3][3] * Scalar;

	return Result;
}

float GCMatrix4x4_Determinant(const GCMatrix4x4* const Matrix)
{
	float Result = 0.0f, InnerMatrixDeterminant = 0.0f;

	InnerMatrixDeterminant += Matrix->Data[1][1] * Matrix->Data[2][2] * Matrix->Data[3][3];
	InnerMatrixDeterminant += Matrix->Data[2][1] * Matrix->Data[3][2] * Matrix->Data[1][3];
	InnerMatrixDeterminant += Matrix->Data[3][1] * Matrix->Data[1][2] * Matrix->Data[2][3];
	InnerMatrixDeterminant -= Matrix->Data[3][1] * Matrix->Data[2][2] * Matrix->Data[1][3];
	InnerMatrixDeterminant -= Matrix->Data[2][1] * Matrix->Data[1][2] * Matrix->Data[3][3];
	InnerMatrixDeterminant -= Matrix->Data[1][1] * Matrix->Data[3][2] * Matrix->Data[2][3];

	Result += Matrix->Data[0][0] * InnerMatrixDeterminant;
	InnerMatrixDeterminant = 0.0f;

	InnerMatrixDeterminant += Matrix->Data[0][1] * Matrix->Data[2][2] * Matrix->Data[3][3];
	InnerMatrixDeterminant += Matrix->Data[2][1] * Matrix->Data[3][2] * Matrix->Data[0][3];
	InnerMatrixDeterminant += Matrix->Data[3][1] * Matrix->Data[0][2] * Matrix->Data[2][3];
	InnerMatrixDeterminant -= Matrix->Data[3][1] * Matrix->Data[2][2] * Matrix->Data[0][3];
	InnerMatrixDeterminant -= Matrix->Data[2][1] * Matrix->Data[0][2] * Matrix->Data[3][3];
	InnerMatrixDeterminant -= Matrix->Data[0][1] * Matrix->Data[3][2] * Matrix->Data[2][3];

	Result -= Matrix->Data[1][0] * InnerMatrixDeterminant;
	InnerMatrixDeterminant = 0.0f;

	InnerMatrixDeterminant += Matrix->Data[0][1] * Matrix->Data[1][2] * Matrix->Data[3][3];
	InnerMatrixDeterminant += Matrix->Data[1][1] * Matrix->Data[3][2] * Matrix->Data[0][3];
	InnerMatrixDeterminant += Matrix->Data[3][1] * Matrix->Data[0][2] * Matrix->Data[1][3];
	InnerMatrixDeterminant -= Matrix->Data[3][1] * Matrix->Data[1][2] * Matrix->Data[0][3];
	InnerMatrixDeterminant -= Matrix->Data[1][1] * Matrix->Data[0][2] * Matrix->Data[3][3];
	InnerMatrixDeterminant -= Matrix->Data[0][1] * Matrix->Data[3][2] * Matrix->Data[1][3];

	Result += Matrix->Data[2][0] * InnerMatrixDeterminant;
	InnerMatrixDeterminant = 0.0f;

	InnerMatrixDeterminant += Matrix->Data[0][1] * Matrix->Data[1][2] * Matrix->Data[2][3];
	InnerMatrixDeterminant += Matrix->Data[1][1] * Matrix->Data[2][2] * Matrix->Data[0][3];
	InnerMatrixDeterminant += Matrix->Data[2][1] * Matrix->Data[0][2] * Matrix->Data[1][3];
	InnerMatrixDeterminant -= Matrix->Data[2][1] * Matrix->Data[1][2] * Matrix->Data[0][3];
	InnerMatrixDeterminant -= Matrix->Data[1][1] * Matrix->Data[0][2] * Matrix->Data[2][3];
	InnerMatrixDeterminant -= Matrix->Data[0][1] * Matrix->Data[2][2] * Matrix->Data[1][3];

	Result -= Matrix->Data[3][0] * InnerMatrixDeterminant;

	return Result;
}

GCMatrix4x4 GCMatrix4x4_Inverse(const GCMatrix4x4* const Matrix)
{
	GCMatrix4x4 Result;

	Result.Data[0][0] = +(Matrix->Data[1][1] * Matrix->Data[2][2] * Matrix->Data[3][3] + Matrix->Data[2][1] * Matrix->Data[3][2] * Matrix->Data[1][3] + Matrix->Data[3][1] * Matrix->Data[1][2] * Matrix->Data[2][3] - Matrix->Data[3][1] * Matrix->Data[2][2] * Matrix->Data[1][3] - Matrix->Data[2][1] * Matrix->Data[1][2] * Matrix->Data[3][3] - Matrix->Data[1][1] * Matrix->Data[3][2] * Matrix->Data[2][3]);
	Result.Data[1][0] = -(Matrix->Data[0][1] * Matrix->Data[2][2] * Matrix->Data[3][3] + Matrix->Data[2][1] * Matrix->Data[3][2] * Matrix->Data[0][3] + Matrix->Data[3][1] * Matrix->Data[0][2] * Matrix->Data[2][3] - Matrix->Data[3][1] * Matrix->Data[2][2] * Matrix->Data[0][3] - Matrix->Data[2][1] * Matrix->Data[0][2] * Matrix->Data[3][3] - Matrix->Data[0][1] * Matrix->Data[3][2] * Matrix->Data[2][3]);
	Result.Data[2][0] = +(Matrix->Data[0][1] * Matrix->Data[1][2] * Matrix->Data[3][3] + Matrix->Data[1][1] * Matrix->Data[3][2] * Matrix->Data[0][3] + Matrix->Data[3][1] * Matrix->Data[0][2] * Matrix->Data[1][3] - Matrix->Data[3][1] * Matrix->Data[1][2] * Matrix->Data[0][3] - Matrix->Data[1][1] * Matrix->Data[0][2] * Matrix->Data[3][3] - Matrix->Data[0][1] * Matrix->Data[3][2] * Matrix->Data[1][3]);
	Result.Data[3][0] = -(Matrix->Data[0][1] * Matrix->Data[1][2] * Matrix->Data[2][3] + Matrix->Data[1][1] * Matrix->Data[2][2] * Matrix->Data[0][3] + Matrix->Data[2][1] * Matrix->Data[0][2] * Matrix->Data[1][3] - Matrix->Data[2][1] * Matrix->Data[1][2] * Matrix->Data[0][3] - Matrix->Data[1][1] * Matrix->Data[0][2] * Matrix->Data[2][3] - Matrix->Data[0][1] * Matrix->Data[2][2] * Matrix->Data[1][3]);

	Result.Data[0][1] = -(Matrix->Data[1][0] * Matrix->Data[2][2] * Matrix->Data[3][3] + Matrix->Data[2][0] * Matrix->Data[3][2] * Matrix->Data[1][3] + Matrix->Data[3][0] * Matrix->Data[1][2] * Matrix->Data[2][3] - Matrix->Data[3][0] * Matrix->Data[2][2] * Matrix->Data[1][3] - Matrix->Data[2][0] * Matrix->Data[1][2] * Matrix->Data[3][3] - Matrix->Data[1][0] * Matrix->Data[3][2] * Matrix->Data[2][3]);
	Result.Data[1][1] = +(Matrix->Data[0][0] * Matrix->Data[2][2] * Matrix->Data[3][3] + Matrix->Data[2][0] * Matrix->Data[3][2] * Matrix->Data[0][3] + Matrix->Data[3][0] * Matrix->Data[0][2] * Matrix->Data[2][3] - Matrix->Data[3][0] * Matrix->Data[2][2] * Matrix->Data[0][3] - Matrix->Data[2][0] * Matrix->Data[0][2] * Matrix->Data[3][3] - Matrix->Data[0][0] * Matrix->Data[3][2] * Matrix->Data[2][3]);
	Result.Data[2][1] = -(Matrix->Data[0][0] * Matrix->Data[1][2] * Matrix->Data[3][3] + Matrix->Data[1][0] * Matrix->Data[3][2] * Matrix->Data[0][3] + Matrix->Data[3][0] * Matrix->Data[0][2] * Matrix->Data[1][3] - Matrix->Data[3][0] * Matrix->Data[1][2] * Matrix->Data[0][3] - Matrix->Data[1][0] * Matrix->Data[0][2] * Matrix->Data[3][3] - Matrix->Data[0][0] * Matrix->Data[3][2] * Matrix->Data[1][3]);
	Result.Data[3][1] = +(Matrix->Data[0][0] * Matrix->Data[1][2] * Matrix->Data[2][3] + Matrix->Data[1][0] * Matrix->Data[2][2] * Matrix->Data[0][3] + Matrix->Data[2][0] * Matrix->Data[0][2] * Matrix->Data[1][3] - Matrix->Data[2][0] * Matrix->Data[1][2] * Matrix->Data[0][3] - Matrix->Data[1][0] * Matrix->Data[0][2] * Matrix->Data[2][3] - Matrix->Data[0][0] * Matrix->Data[2][2] * Matrix->Data[1][3]);

	Result.Data[0][2] = +(Matrix->Data[1][0] * Matrix->Data[2][1] * Matrix->Data[3][3] + Matrix->Data[2][0] * Matrix->Data[3][1] * Matrix->Data[1][3] + Matrix->Data[3][0] * Matrix->Data[1][1] * Matrix->Data[2][3] - Matrix->Data[3][0] * Matrix->Data[2][1] * Matrix->Data[1][3] - Matrix->Data[2][0] * Matrix->Data[1][1] * Matrix->Data[3][3] - Matrix->Data[1][0] * Matrix->Data[3][1] * Matrix->Data[2][3]);
	Result.Data[1][2] = -(Matrix->Data[0][0] * Matrix->Data[2][1] * Matrix->Data[3][3] + Matrix->Data[2][0] * Matrix->Data[3][1] * Matrix->Data[0][3] + Matrix->Data[3][0] * Matrix->Data[0][1] * Matrix->Data[2][3] - Matrix->Data[3][0] * Matrix->Data[2][1] * Matrix->Data[0][3] - Matrix->Data[2][0] * Matrix->Data[0][1] * Matrix->Data[3][3] - Matrix->Data[0][0] * Matrix->Data[3][1] * Matrix->Data[2][3]);
	Result.Data[2][2] = +(Matrix->Data[0][0] * Matrix->Data[1][1] * Matrix->Data[3][3] + Matrix->Data[1][0] * Matrix->Data[3][1] * Matrix->Data[0][3] + Matrix->Data[3][0] * Matrix->Data[0][1] * Matrix->Data[1][3] - Matrix->Data[3][0] * Matrix->Data[1][1] * Matrix->Data[0][3] - Matrix->Data[1][0] * Matrix->Data[0][1] * Matrix->Data[3][3] - Matrix->Data[0][0] * Matrix->Data[3][1] * Matrix->Data[1][3]);
	Result.Data[3][2] = -(Matrix->Data[0][0] * Matrix->Data[1][1] * Matrix->Data[2][3] + Matrix->Data[1][0] * Matrix->Data[2][1] * Matrix->Data[0][3] + Matrix->Data[2][0] * Matrix->Data[0][1] * Matrix->Data[1][3] - Matrix->Data[2][0] * Matrix->Data[1][1] * Matrix->Data[0][3] - Matrix->Data[1][0] * Matrix->Data[0][1] * Matrix->Data[2][3] - Matrix->Data[0][0] * Matrix->Data[2][1] * Matrix->Data[1][3]);

	Result.Data[0][3] = -(Matrix->Data[1][0] * Matrix->Data[2][1] * Matrix->Data[3][2] + Matrix->Data[2][0] * Matrix->Data[3][1] * Matrix->Data[1][2] + Matrix->Data[3][0] * Matrix->Data[1][1] * Matrix->Data[2][2] - Matrix->Data[3][0] * Matrix->Data[2][1] * Matrix->Data[1][2] - Matrix->Data[2][0] * Matrix->Data[1][1] * Matrix->Data[3][2] - Matrix->Data[1][0] * Matrix->Data[3][1] * Matrix->Data[2][2]);
	Result.Data[1][3] = +(Matrix->Data[0][0] * Matrix->Data[2][1] * Matrix->Data[3][2] + Matrix->Data[2][0] * Matrix->Data[3][1] * Matrix->Data[0][2] + Matrix->Data[3][0] * Matrix->Data[0][1] * Matrix->Data[2][2] - Matrix->Data[3][0] * Matrix->Data[2][1] * Matrix->Data[0][2] - Matrix->Data[2][0] * Matrix->Data[0][1] * Matrix->Data[3][2] - Matrix->Data[0][0] * Matrix->Data[3][1] * Matrix->Data[2][2]);
	Result.Data[2][3] = -(Matrix->Data[0][0] * Matrix->Data[1][1] * Matrix->Data[3][2] + Matrix->Data[1][0] * Matrix->Data[3][1] * Matrix->Data[0][2] + Matrix->Data[3][0] * Matrix->Data[0][1] * Matrix->Data[1][2] - Matrix->Data[3][0] * Matrix->Data[1][1] * Matrix->Data[0][2] - Matrix->Data[1][0] * Matrix->Data[0][1] * Matrix->Data[3][2] - Matrix->Data[0][0] * Matrix->Data[3][1] * Matrix->Data[1][2]);
	Result.Data[3][3] = +(Matrix->Data[0][0] * Matrix->Data[1][1] * Matrix->Data[2][2] + Matrix->Data[1][0] * Matrix->Data[2][1] * Matrix->Data[0][2] + Matrix->Data[2][0] * Matrix->Data[0][1] * Matrix->Data[1][2] - Matrix->Data[2][0] * Matrix->Data[1][1] * Matrix->Data[0][2] - Matrix->Data[1][0] * Matrix->Data[0][1] * Matrix->Data[2][2] - Matrix->Data[0][0] * Matrix->Data[2][1] * Matrix->Data[1][2]);

	Result = GCMatrix4x4_Transpose(&Result);
	Result = GCMatrix4x4_MultiplyByScalar(&Result, 1.0f / GCMatrix4x4_Determinant(Matrix));

	return Result;
}

GCMatrix4x4 GCMatrix4x4_Transpose(const GCMatrix4x4* const Matrix)
{
	GCMatrix4x4 Result;

	Result.Data[0][0] = Matrix->Data[0][0];
	Result.Data[0][1] = Matrix->Data[1][0];
	Result.Data[0][2] = Matrix->Data[2][0];
	Result.Data[0][3] = Matrix->Data[3][0];

	Result.Data[1][0] = Matrix->Data[0][1];
	Result.Data[1][1] = Matrix->Data[1][1];
	Result.Data[1][2] = Matrix->Data[2][1];
	Result.Data[1][3] = Matrix->Data[3][1];

	Result.Data[2][0] = Matrix->Data[0][2];
	Result.Data[2][1] = Matrix->Data[1][2];
	Result.Data[2][2] = Matrix->Data[2][2];
	Result.Data[2][3] = Matrix->Data[3][2];

	Result.Data[3][0] = Matrix->Data[0][3];
	Result.Data[3][1] = Matrix->Data[1][3];
	Result.Data[3][2] = Matrix->Data[2][3];
	Result.Data[3][3] = Matrix->Data[3][3];

	return Result;
}

char* GCMatrix4x4_ToString(const GCMatrix4x4* const Matrix)
{
	const int32_t Length = snprintf(NULL, 0, "(%f, %f, %f, %f)\n(%f, %f, %f, %f)\n(%f, %f, %f, %f)\n(%f, %f, %f, %f)",
                                    Matrix->Data[0][0], Matrix->Data[0][1], Matrix->Data[0][2], Matrix->Data[0][3],
                                    Matrix->Data[1][0], Matrix->Data[1][1], Matrix->Data[1][2], Matrix->Data[1][3],
                                    Matrix->Data[2][0], Matrix->Data[2][1], Matrix->Data[2][2], Matrix->Data[2][3],
                                    Matrix->Data[3][0], Matrix->Data[3][1], Matrix->Data[3][2], Matrix->Data[3][3]) + 1;

	char* Buffer = (char*)GCMemory_Allocate(Length * sizeof(char));
	snprintf(Buffer, Length, "(%f, %f, %f, %f)\n(%f, %f, %f, %f)\n(%f, %f, %f, %f)\n(%f, %f, %f, %f)",
             Matrix->Data[0][0], Matrix->Data[0][1], Matrix->Data[0][2], Matrix->Data[0][3],
             Matrix->Data[1][0], Matrix->Data[1][1], Matrix->Data[1][2], Matrix->Data[1][3],
             Matrix->Data[2][0], Matrix->Data[2][1], Matrix->Data[2][2], Matrix->Data[2][3],
             Matrix->Data[3][0], Matrix->Data[3][1], Matrix->Data[3][2], Matrix->Data[3][3]);

	return Buffer;
}
