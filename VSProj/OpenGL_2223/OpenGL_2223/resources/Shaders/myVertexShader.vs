#version 330 core
layout(location = 0) in vec3 vPos;

out vec2 fragCoord;

uniform vec2 iResolution;

void main()
{
    gl_Position = vec4(vPos.xyz, 1.0);
    fragCoord =  (vPos.xy * 0.5 + 0.5) * iResolution;
}