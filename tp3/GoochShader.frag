/**
* Fragment shader to draw Gooch.
* Code found here : https://github.com/castano/qshaderedit/blob/master/data/shaders/gooch.glsl
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
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    vec3 viewDir = normalize(cameraPosition - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);  
    vec3 surfaceColor = texture(texture1, TexCoord).rgb;

    //draw outlines
    float dotVN = dot(viewDir, norm);
    if (dotVN < mix(0.5, 0.5, max(0.0, dot(norm, lightDir)))) {
        fragColor = vec4(0,0,0,0); 
        return;
    }
    
    //color of the shader
    vec3 warmColor = vec3(0.6, 0.6, 0.0);
    vec3 coolColor = vec3(0.0, 0.0, 0.6);

    float NdotL = 0.5 * (dot(lightDir, norm) + 1.0);

    vec3 kcool = min(coolColor + 0.45 * surfaceColor, 1.0);
    vec3 kwarm = min(warmColor + 0.45 * surfaceColor, 1.0); 
    vec3 kfinal = mix(kcool, kwarm, NdotL);

    float spec = pow(max(dot(viewDir, reflectDir), 0.0), materialShininess);

    fragColor = vec4(min(kfinal + spec, 1.0), 1.0);
}