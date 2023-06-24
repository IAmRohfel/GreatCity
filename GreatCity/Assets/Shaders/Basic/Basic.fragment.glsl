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
layout(location = 4) in flat uint FragmentTextureIndex;
layout(location = 5) in flat uint FragmentEntityID;

layout(binding = 1) uniform sampler2D TextureSamplers[32];

layout(location = 0) out vec4 Color;
layout(location = 1) out uint EntityID;

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

    switch(FragmentTextureIndex)
    {
        case 0:
            Color *= texture(TextureSamplers[0], FragmentTextureCoordinate);
            break;
        case 1:
            Color *= texture(TextureSamplers[1], FragmentTextureCoordinate);
            break;
        case 2:
            Color *= texture(TextureSamplers[2], FragmentTextureCoordinate);
            break;
        case 3:
            Color *= texture(TextureSamplers[3], FragmentTextureCoordinate);
            break;
        case 4:
            Color *= texture(TextureSamplers[4], FragmentTextureCoordinate);
            break;
        case 5:
            Color *= texture(TextureSamplers[5], FragmentTextureCoordinate);
            break;
        case 6:
            Color *= texture(TextureSamplers[6], FragmentTextureCoordinate);
            break;
        case 7:
            Color *= texture(TextureSamplers[7], FragmentTextureCoordinate);
            break;
        case 8:
            Color *= texture(TextureSamplers[8], FragmentTextureCoordinate);
            break;
        case 9:
            Color *= texture(TextureSamplers[9], FragmentTextureCoordinate);
            break;
        case 10:
            Color *= texture(TextureSamplers[10], FragmentTextureCoordinate);
            break;
        case 11:
            Color *= texture(TextureSamplers[11], FragmentTextureCoordinate);
            break;
        case 12:
            Color *= texture(TextureSamplers[12], FragmentTextureCoordinate);
            break;
        case 13:
            Color *= texture(TextureSamplers[13], FragmentTextureCoordinate);
            break;
        case 14:
            Color *= texture(TextureSamplers[14], FragmentTextureCoordinate);
            break;
        case 15:
            Color *= texture(TextureSamplers[15], FragmentTextureCoordinate);
            break;
        case 16:
            Color *= texture(TextureSamplers[16], FragmentTextureCoordinate);
            break;
        case 17:
            Color *= texture(TextureSamplers[17], FragmentTextureCoordinate);
            break;
        case 18:
            Color *= texture(TextureSamplers[18], FragmentTextureCoordinate);
            break;
        case 19:
            Color *= texture(TextureSamplers[19], FragmentTextureCoordinate);
            break;
        case 20:
            Color *= texture(TextureSamplers[20], FragmentTextureCoordinate);
            break;
        case 21:
            Color *= texture(TextureSamplers[21], FragmentTextureCoordinate);
            break;
        case 22:
            Color *= texture(TextureSamplers[22], FragmentTextureCoordinate);
            break;
        case 23:
            Color *= texture(TextureSamplers[23], FragmentTextureCoordinate);
            break;
        case 24:
            Color *= texture(TextureSamplers[24], FragmentTextureCoordinate);
            break;
        case 25:
            Color *= texture(TextureSamplers[25], FragmentTextureCoordinate);
            break;
        case 26:
            Color *= texture(TextureSamplers[26], FragmentTextureCoordinate);
            break;
        case 27:
            Color *= texture(TextureSamplers[27], FragmentTextureCoordinate);
            break;
        case 28:
            Color *= texture(TextureSamplers[28], FragmentTextureCoordinate);
            break;
        case 29:
            Color *= texture(TextureSamplers[29], FragmentTextureCoordinate);
            break;
        case 30:
            Color *= texture(TextureSamplers[30], FragmentTextureCoordinate);
            break;
        case 31:
            Color *= texture(TextureSamplers[31], FragmentTextureCoordinate);
            break;
    }

    EntityID = FragmentEntityID;
}