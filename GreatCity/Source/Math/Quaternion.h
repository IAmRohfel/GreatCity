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

#ifndef GC_MATH_QUATERNION_H
#define GC_MATH_QUATERNION_H

#include "Math/Matrix4x4.h"
#include "Math/Vector3.h"

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