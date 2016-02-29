#version 330 core

uniform mat4 trans;

layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec3 vertexNormal;
layout(location = 2) in vec3 ambientColor; 
layout(location = 3) in vec3 diffuseColor;

out vec3 Color;

void
main()
{
    Color = ambientColor+diffuseColor;
    gl_Position = trans*vec4(vertexPosition, 1.0);
}
