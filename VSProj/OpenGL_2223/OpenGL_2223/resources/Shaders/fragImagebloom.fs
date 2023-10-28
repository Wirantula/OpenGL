#version 330 core
out (location = 0) vec4 FragColor;
out (location = 1) vec4 BrightColor;

in vec2 fragCoord;
uniform sampler2D _MainTex;

void main()
{
    FragColor = vec4(texture(_MainTex, fragCoord).rgb, 1.0); //vec4( 1- texture(_MainTex, fragCoord).rgb, 1.0);
    float brightness = dot(FragColor.rgb, vec3(0.2126, 0.7152, 0.0722));
    if(brightness > 1.0)
        BrightColor = vec4(FragColor.rgb, 1.0);
    else
        BrightColor = vec4(0.0, 0.0, 0.0, 1.0);
}