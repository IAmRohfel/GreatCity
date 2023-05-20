#ifndef GC_MATH_QUATERNION_H
#define GC_MATH_QUATERNION_H

#include "Math/Vector3.h"
#include "Math/Matrix4x4.h"

#ifdef __cplusplus
extern "C"
{
#endif

	typedef struct GCQuaternion
	{
		float W, X, Y, Z;
	} GCQuaternion;

	GCQuaternion GCQuaternion_Create(const float W, const float X, const float Y, const float Z);
	GCQuaternion GCQuaternion_CreateZero(void);
	GCQuaternion GCQuaternion_CreateUnit(void);
	GCQuaternion GCQuaternion_CreateFromEulerAngles(const float Pitch, const float Yaw, const float Roll);

	GCMatrix4x4 GCQuaternion_ToRotationMatrix(const GCQuaternion Quaternion);

	GCQuaternion GCQuaternion_Multiply(const GCQuaternion Quaternion1, const GCQuaternion Quaternion2);

	GCQuaternion GCQuaternion_Conjugate(const GCQuaternion Quaternion);
	float GCQuaternion_Magnitude(const GCQuaternion Quaternion);
	GCQuaternion GCQuaternion_Normalize(const GCQuaternion Quaternion);
	GCVector3 GCQuaternion_RotateVector(const GCQuaternion Quaternion, const GCVector3 Vector);

	char* GCQuaternion_ToString(const GCQuaternion Quaternion);

#ifdef __cplusplus
}
#endif

#endif