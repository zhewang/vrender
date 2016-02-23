#version 330 core

uniform vec3 vColor;

layout(location = 0) in vec4 vPosition;

out vec3 Color;

void
main()
{
    Color = vColor;
    gl_Position = vPosition;
}
