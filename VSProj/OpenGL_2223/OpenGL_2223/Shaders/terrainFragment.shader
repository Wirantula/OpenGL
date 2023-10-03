#version 330 core
out vec4 FragColor;

in vec2 uv;
in vec3 normal;
in vec3 worldPosition;

uniform sampler2D mainTex;
uniform sampler2D normalTex;

uniform vec3 lightDirection;
uniform vec3 cameraPosition;

void main()
{
    //normal map
    //vec3 normal = texture(normalTex, uv).rgb;
    //normal = normalize(normal * 2.0 - 1.0);

    // specular data
    //vec3 viewDir = normalize(worldPosition - cameraPosition);
    //vec3 reflDir = normalize(reflect(lightDirection, normal));

    //lighting
    float lightValue = max(-dot(normal, lightDirection), 0.0);
    //float specular = pow(max(-dot(reflDir, viewDir), 0.0),8);

    vec4 output = texture(mainTex, uv);
    output.rgb = output.rgb * min(lightValue + 0.1, 1.0); //* + specular * output.rgb

    FragColor = output;
} 