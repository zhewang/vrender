#version 330 core

uniform mat4 View;
uniform mat4 Model;
uniform mat4 Project;

layout(location = 0) in vec3 vPos;

out vec4 Color;

void main()
{
    Color = vec4(128, 64, 0, 1.0);
    gl_Position = Project*View*Model*vec4(vPos, 1.0);
}
