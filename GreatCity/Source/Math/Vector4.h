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

#ifndef GC_MATH_VECTOR_4_H
#define GC_MATH_VECTOR_4_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct GCVector4
    {
        float X, Y, Z, W;
    } GCVector4;

    GCVector4 GCVector4_Create(const float X, const float Y, const float Z, const float W);
    GCVector4 GCVector4_CreateZero(void);

    GCVector4 GCVector4_Add(const GCVector4 Vector1, const GCVector4 Vector2);
    GCVector4 GCVector4_Subtract(const GCVector4 Vector1, const GCVector4 Vector2);
    GCVector4 GCVector4_Multiply(const GCVector4 Vector1, const GCVector4 Vector2);
    GCVector4 GCVector4_MultiplyByScalar(const GCVector4 Vector, const float Scalar);
    GCVector4 GCVector4_Divide(const GCVector4 Vector1, const GCVector4 Vector2);

    float GCVector4_Dot(const GCVector4 Vector1, const GCVector4 Vector2);

    float GCVector4_Magnitude(const GCVector4 Vector);
    GCVector4 GCVector4_Normalize(const GCVector4 Vector);

    char* GCVector4_ToString(const GCVector4 Vector);
    bool GCVector4_IsEqual(const GCVector4 Vector1, const GCVector4 Vector2);

#ifdef __cplusplus
}
#endif

#endif