#version 410 core

uniform sampler3D VolumeTex;
uniform sampler1D TransferTex;

in vec3 Texcoord;

out vec4 fColor;

void main()
{
    float value;
    vec4 colorSample;

    //value = texture(VolumeTex, Texcoord).x;
    //if(value == value) {
    //    fColor = vec4(0,1,0,1);
    //} else { 
    //    fColor = vec4(1,0,0,1);
    //    colorSample = texture(TransferFunc, 0.5f);
    //    if(colorSample.r == 0) {
    //        fColor = vec4(1,0,0,1);
    //    } else {
    //        fColor = colorSample;
    //    }
    //}

    //value = texture(VolumeTex, Texcoord).x;
    //texture(TransferTex, 0.5f);
    //if(value < 0.1) {
    //    discard;
    //} else if (value > 0.8){
    //    discard;
    //} else {
    //    fColor = vec4(value, value, value, value);
    //}
    value = texture(VolumeTex, Texcoord).x;
    colorSample = texture(TransferTex, value);
    fColor = vec4(colorSample.r*255, colorSample.g*255, colorSample.b*255, colorSample.r);
}
