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

layout(location = 0) in vec3 Position;
layout(location = 1) in vec3 Normal;
layout(location = 2) in vec4 Color;
layout(location = 3) in vec2 TextureCoordinate;
layout(location = 4) in int EntityID;

layout(binding = 0) uniform UniformBuffer
{
	mat4 ViewProjectionMatrix;
} UniformBufferData;

layout(location = 0) out vec3 FragmentPosition;
layout(location = 1) out vec4 FragmentColor;
layout(location = 2) out vec3 FragmentNormal;
layout(location = 3) out vec2 FragmentTextureCoordinate;
layout(location = 4) out int FragmentEntityID;

void main()
{
	gl_Position = UniformBufferData.ViewProjectionMatrix * vec4(Position, 1.0);

	FragmentTextureCoordinate = TextureCoordinate;
    FragmentNormal = Normal;
	FragmentColor = Color;
    FragmentEntityID = EntityID;
}