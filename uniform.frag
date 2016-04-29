#version 410 core

uniform sampler3D tex;

in vec3 Texcoord;

out vec4 fColor;

void main()
{
    vec4 colorSample;

    colorSample = texture(tex, Texcoord);
    if(colorSample.x < 0.3) {
        discard;
    } else {
        fColor = vec4(colorSample.r, colorSample.r, colorSample.r, 1.0);
    }
}
