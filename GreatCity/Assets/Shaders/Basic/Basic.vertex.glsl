#version 450

layout(location = 0) in vec3 Position;
layout(location = 1) in vec4 Color;
layout(location = 2) in vec2 TextureCoordinate;

layout(binding = 0) uniform UniformBuffer
{
	mat4 ViewProjectionMatrix;
} UniformBufferData;

layout(location = 0) out vec4 FragmentColor;
layout(location = 1) out vec2 FragmentTextureCoordinate;

void main()
{
	gl_Position = UniformBufferData.ViewProjectionMatrix * vec4(Position, 1.0);

	FragmentTextureCoordinate = TextureCoordinate;
	FragmentColor = Color;
}