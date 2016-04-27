#version 330 core

uniform sampler2D tex;

in vec3 Texcoord;

out vec4 fColor;

void
main()
{
    fColor = texture(tex, Texcoord.xy);
}
