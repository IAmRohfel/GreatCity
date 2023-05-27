#version 450

layout(location = 0) in vec3 Position;
layout(location = 1) in vec3 Color;

layout(binding = 0) uniform UniformBuffer
{
	mat4 ViewProjectionMatrix;
} UniformBufferData;

layout(location = 0) out vec3 FragmentColor;

void main()
{
	gl_Position = UniformBufferData.ViewProjectionMatrix * vec4(Position, 1.0);
	FragmentColor = Color;
}