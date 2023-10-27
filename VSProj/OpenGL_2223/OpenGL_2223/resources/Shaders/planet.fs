#version 330 core
out vec4 FragColor;

in vec2 TexCoords;
in vec3 Normals;
in vec4 FragPos;

uniform sampler2D day, night, clouds;

uniform vec3 cameraPosition;
uniform vec3 lightDirection;

uniform float time;

float lerp(float a, float b, float t) { return a + (b - a) * t;}
vec3 lerp(vec3 a, vec3 b, float t){return a + (b-a) * t;}
vec4 lerp(vec4 a, vec4 b, float t){return a + (b-a) * t;}

void main()
{  
    vec4 dayColor = texture(day, TexCoords);
    vec4 nightColor = texture(night, TexCoords);
    vec4 CloudsColor = texture(clouds, TexCoords + vec2(-time / (360 * 6.28), 0));

    float light = max(dot(-lightDirection, Normals), 0.0);
    light = pow(light * 16, 2.0) / 16;
    light = max(min( light, 1.0), 0.0);

    vec3 viewDir = normalize(FragPos.rgb - cameraPosition);
    vec3 refl = reflect(lightDirection, Normals);

    float spec = pow(max(dot(-viewDir, refl), 0.0), 6.0);

    float fresnel = pow(1.0 - max(dot(-viewDir, Normals), 0.0), 3.0);

    vec3 specular = (spec + fresnel * 2 * light)* vec3(0.2,0.3,0.6);
    
    vec4 output = lerp(lerp(nightColor, dayColor, light), CloudsColor * 1.2 * light, CloudsColor.r) + vec4(specular, 0);

    FragColor = output;
}