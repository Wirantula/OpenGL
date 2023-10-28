#version 330 core
out vec4 FragColor;

in vec2 fragCoord;
uniform sampler2D _MainTex;

uniform bool horizontal = false;
uniform float weight[5] = float[] (0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);

void main()
{
    vec2 fragCoord_offset = 1.0 / textureSize(_MainTex, 0);
    vec3 result = texture(_MainTex, fragCoord).rgb * weight[0];
    if(horizontal)
    {
        for(int i = 1; i < 5; ++i)
        {
            result += texture(_MainTex, fragCoord + vec2(fragCoord_offset.x * i, 0.0)).rgb * weight[i];
            result += texture(_MainTex, fragCoord - vec2(fragCoord_offset.x * i, 0.0)).rgb * weight[i];
        }
    }
    else
    {
        for(int i = 1; i < 5; ++i)
        {
            result += texture(_MainTex, fragCoord + vec2(fragCoord_offset.y * i, 0.0)).rgb * weight[i];
            result += texture(_MainTex, fragCoord - vec2(fragCoord_offset.y * i, 0.0)).rgb * weight[i];
        }
    }
    FragColor = vec4(result, 1.0); //vec4( 1- texture(_MainTex, fragCoord).rgb, 1.0);
}