#version 450

layout(location = 0) out vec4 Color;

layout(location = 0) in vec3 FragmentColor;

void main()
{
	Color = vec4(FragmentColor, 1.0);
}