#version 330 core
out vec4 FragColor;

in vec2 fragCoord;
uniform sampler2D _MainTex;

void main()
{
    float gamma = 2.2;//2.2; 2.2 is default correction, 1 is no correction
    vec3 diffuseColor = pow(texture(_MainTex, fragCoord).rgb, vec3(gamma));
    FragColor = vec4(diffuseColor, 1.0); //vec4( 1- texture(_MainTex, fragCoord).rgb, 1.0);
}