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
    vec3 L = normalize(vec3(0.3, 0.5, 0.5));
    vec3 N = normalize(vertexNormal);
    Color = ambientColor+dot(N, L)*diffuseColor;
    gl_Position = trans*vec4(vertexPosition, 1.0);
}
