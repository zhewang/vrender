#version 410 core

uniform mat4 View;
uniform mat4 Model;
uniform mat4 Project;

layout(location = 0) in vec3 vPos;
layout(location = 1) in vec3 vtPos;

out vec3 Texcoord;

void main()
{
    Texcoord = vtPos;
    gl_Position = Project*View*Model*vec4(vPos, 1.0);
}
