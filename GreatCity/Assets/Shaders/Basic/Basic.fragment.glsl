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

#version 450

layout(location = 0) in vec3 FragmentPosition;
layout(location = 1) in vec4 FragmentColor;
layout(location = 2) in vec3 FragmentNormal;
layout(location = 3) in vec2 FragmentTextureCoordinate;
layout(location = 4) in flat int FragmentEntityID;

layout(binding = 1) uniform sampler2D TextureSampler;

layout(location = 0) out vec4 Color;
layout(location = 1) out int EntityID;

void main()
{
    const vec3 LightPosition = vec3(0.0f, -2.0f, 0.0f);
    const vec3 LightColor = vec3(1.0f);

    const float AmbientStrength = 0.1;
    const vec3 Ambient = AmbientStrength * LightColor;

    const vec3 NormalizedNormal = normalize(FragmentNormal);
    const vec3 LightDirection = normalize(LightPosition - FragmentPosition);

    const vec3 Diffuse = max(dot(NormalizedNormal, LightDirection), 0.0) * LightColor;
    const vec3 ColorResult = (Ambient + Diffuse) * vec3(FragmentColor);

	Color = vec4(ColorResult, 1.0);
    EntityID = FragmentEntityID;
}