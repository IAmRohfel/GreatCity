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

#include "World/Components.h"
#include "Math/Matrix4x4.h"
#include "Math/Quaternion.h"
#include "Math/Vector3.h"

GCMatrix4x4 GCTransformComponent_GetTransform(const GCTransformComponent* const TransformComponent)
{
    const GCMatrix4x4 Translation = GCMatrix4x4_CreateTranslation(TransformComponent->Translation);
    const GCMatrix4x4 Rotation = GCQuaternion_ToRotationMatrix(GCQuaternion_CreateFromEulerAngles(
        TransformComponent->Rotation.X, TransformComponent->Rotation.Y, TransformComponent->Rotation.Z
    ));
    const GCMatrix4x4 Scale = GCMatrix4x4_CreateScale(TransformComponent->Scale);

    GCMatrix4x4 Transform = GCMatrix4x4_Multiply(&Translation, &Rotation);
    Transform = GCMatrix4x4_Multiply(&Transform, &Scale);

    return Transform;
}