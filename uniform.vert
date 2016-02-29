#version 330 core

uniform mat4 trans;

layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec3 vertexNormal;

void
main()
{
    gl_Position = trans*vec4(vertexPosition, 1.0);
}
