#version 330 core

uniform mat4 View;
uniform mat4 Model;
uniform mat4 Project;

layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec3 vertexNormal;
layout(location = 2) in vec3 ambientColor; 
layout(location = 3) in vec3 diffuseColor;

out vec3 Color;

void
main()
{
    vec3 L = normalize(vec3(0.1, 0.5, 1.0));

    vec3 newNormal = mat3(Model)*vertexNormal;
    vec3 N = normalize(newNormal);

    float NdotL = min(max(0, dot(N, L)), 1); // make sure NdotL is in [0,1]
    Color = ambientColor + NdotL * diffuseColor;

    gl_Position = Project*View*Model*vec4(vertexPosition, 1.0);
}
