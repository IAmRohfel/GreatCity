#version 450

layout(location = 0) in vec3 FragmentColor;
layout(location = 1) in vec2 FragmentTextureCoordinate;

layout(binding = 1) uniform sampler2D TextureSampler;

layout(location = 0) out vec4 Color;

void main()
{
	Color = texture(TextureSampler, FragmentTextureCoordinate);
}