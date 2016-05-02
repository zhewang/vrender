#version 410 core

uniform mat4 View;
uniform mat4 Model;
uniform mat4 Project;

uniform mat4 texRotate;

layout(location = 0) in vec3 vPos;
layout(location = 1) in vec3 vtPos;

out vec3 Texcoord;

void main()
{
    vec3 temp;
    temp = vtPos + vec3(-0.5f, -0.5f, -0.5f);
    temp = mat3(texRotate)*temp;
    Texcoord = temp+ vec3(0.5f, 0.5f, 0.5f);
    gl_Position = Project*View*Model*vec4(vPos, 1.0);
}
