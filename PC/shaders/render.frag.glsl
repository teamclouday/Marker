#version 450 core

layout (location = 0) in vec2 imgUV;
layout (location = 0) out vec4 color;

uniform sampler2D image;
uniform bool debug;

void main()
{
    if(debug)
    {
        color = vec4(texture(image, imgUV).rrr, 1.0);
    }
    else
    {
        color = vec4(texture(image, imgUV).rgb, 1.0);
    }
}