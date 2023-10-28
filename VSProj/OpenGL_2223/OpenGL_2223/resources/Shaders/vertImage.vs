#version 330 core
layout(location = 0) in vec3 aPos;

out vec2 fragCoord;

void main()
{
    gl_Position = vec4(aPos, 1.0);
    fragCoord =  (gl_Position.xy / gl_Position.w) * 0.5 + 0.5;
}