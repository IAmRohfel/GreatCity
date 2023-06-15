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

#ifndef GC_MATH_VECTOR_2_H
#define GC_MATH_VECTOR_2_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct GCVector2
    {
        float X, Y;
    } GCVector2;

    GCVector2 GCVector2_Create(const float X, const float Y);
    GCVector2 GCVector2_CreateZero(void);

    GCVector2 GCVector2_Add(const GCVector2 Vector1, const GCVector2 Vector2);
    GCVector2 GCVector2_Subtract(const GCVector2 Vector1, const GCVector2 Vector2);
    GCVector2 GCVector2_Multiply(const GCVector2 Vector1, const GCVector2 Vector2);
    GCVector2 GCVector2_MultiplyByScalar(const GCVector2 Vector, const float Scalar);
    GCVector2 GCVector2_Divide(const GCVector2 Vector1, const GCVector2 Vector2);

    float GCVector2_Dot(const GCVector2 Vector1, const GCVector2 Vector2);

    float GCVector2_Magnitude(const GCVector2 Vector);
    GCVector2 GCVector2_Normalize(const GCVector2 Vector);

    char* GCVector2_ToString(const GCVector2 Vector);
    bool GCVector2_IsEqual(const GCVector2 Vector1, const GCVector2 Vector2);

#ifdef __cplusplus
}
#endif

#endif