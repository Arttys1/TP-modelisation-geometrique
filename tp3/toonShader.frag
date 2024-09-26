/**
* Fragment shader to draw TOON.
* Code found here : https://github.com/ms0g/glToonShader/blob/main/shaders/toon.frag.glsl
*/
#version 410 core

in vec3 Normal;
in vec3 FragPos;
in vec2 TexCoord;

out vec4 fragColor;

uniform sampler2D texture1;

uniform vec3 cameraPosition;
uniform vec3 lightPos;
uniform vec3 lightColor;
uniform float materialShininess;

void main() {
    vec3 modelColor = texture(texture1, TexCoord).rgb;
    vec3 normal = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    vec3 viewDir = normalize(cameraPosition - FragPos);
    float dotVN = dot(viewDir, normal);

    //draw outlines
    if (dotVN < mix(0.5, 0.5, max(0.0, dot(normal, lightDir)))) {
        fragColor = vec4(0,0,0,0); 
        return;
    }

    // Ambient
    vec3 ambient = vec3(1.0);

    // Diffuse lighting
    float dp = max(0.0, dot(lightDir, normal));

    float fullShadow = smoothstep(0.5, 0.505, dp);
    float partialShadow = mix(0.5, 1.0, smoothstep(0.65, 0.655, dp));
    dp = min(partialShadow, fullShadow);
    vec3 diffuse = dp * lightColor;

    // Specular
    vec3 r = normalize(reflect(-lightDir, normal));
    float phongValue = max(0.0, dot(viewDir, r));
    float specular = pow(phongValue, 128.0);

    // Fresnel
    float fresnel = 1.0 - max(0.0, dotVN);
    fresnel = pow(fresnel, 2.0);
    fresnel *= step(0.7, fresnel);

    specular += phongValue;
    specular = smoothstep(0.5, 0.51, specular);

    vec3 lighting = (fresnel + 0.2) + diffuse * 0.8;

    vec3 color = modelColor * lighting + specular;
    vec4 result = vec4(pow(color, vec3(1.0 / 2.2)), 1.0);
    fragColor = result;



}