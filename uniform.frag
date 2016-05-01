#version 410 core

uniform sampler3D VolumeTex;
uniform sampler1D TransferTex;

in vec3 Texcoord;

out vec4 fColor;

void main()
{
    float value;
    vec4 colorSample;

    value = texture(VolumeTex, Texcoord).x;
    colorSample = texture(TransferTex, value);
    if(value > 0.2) {
        fColor = vec4(colorSample.r, colorSample.g, colorSample.b, colorSample.a+0.1);
    } else {
        discard;
    }
}
