#version 330 core

uniform mat4 View;
uniform mat4 Model;
uniform mat4 Project;

layout(location = 0) in vec3 vertexPosition;

out vec3 Color;

void main()
{
    Color = vec3(1.0, 0.0, 0.0);

    gl_Position = Project*View*Model*vec4(vertexPosition, 1.0);
}
