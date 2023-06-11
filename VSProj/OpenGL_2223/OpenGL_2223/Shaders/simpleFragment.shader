#version 330 core
out vec4 FragColor;

in vec3 color;
in vec2 uv;
in mat3 tbn;
in vec3 worldPosition;

uniform sampler2D mainTex;
uniform sampler2D normalTex;
uniform sampler2D specularTex;
uniform sampler2D displaceTex;

uniform vec3 lightPosition;
uniform vec3 cameraPosition;
uniform float u_time;

void main()
{
    //normal map
    vec3 normal = texture(normalTex, uv).rgb;
    normal = normalize(normal * 2.0 - 1.0);

    normal.rg = normal.rg * 0.6;
    normal = normalize(normal);

    normal = tbn * normal;

    vec3 lightDirection = normalize(worldPosition - lightPosition);

    vec4 displace = texture(displaceTex, uv);
    float val = sin(min(u_time/2, u_time * 2));
    float displaceValue = displace.g * val;
    vec2 uvDisplaced = vec2(uv.x - displaceValue, uv.y - displaceValue);

    // specular data
    vec3 viewDir = normalize(worldPosition - cameraPosition);
    vec3 reflDir = normalize(reflect(lightDirection, normal));
    float spec = pow(max(-dot(reflDir, viewDir), 1.5), 3);
    vec3 specular = spec * vec3(texture(specularTex, uvDisplaced));

    //lighting
    float lightValue = max(-dot(normal, lightDirection), 0.0);

    vec4 output = texture(mainTex, uvDisplaced);
    output.rgb = output.rgb * min(lightValue + 0.3, 1.0) + specular * output.rgb;

    FragColor = output;
} 

//vec4(color, 1.0f) * 