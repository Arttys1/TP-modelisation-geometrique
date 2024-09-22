#version 330 core
out vec4 FragColor;

in vec3 FragPos;  
in vec3 Normal;  
in vec2 TexCoord;
  

// texture samplers
uniform sampler2D texture1;

uniform vec3 cameraPosition;
uniform vec3 materialColor;
uniform vec3 lightPos;
uniform vec3 lightColor;
uniform float lightAmbientCoefficient;
uniform float lightAttenuation;
uniform float materialShininess;

void main()
{
    // ambient
    vec3 ambient = lightColor * texture(texture1, TexCoord).rgb;
  	
    // diffuse 
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = lightColor * diff * texture(texture1, TexCoord).rgb;  
    
    // specular
    vec3 viewDir = normalize(cameraPosition - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), materialShininess);
    vec3 specular = lightColor * spec;  
        
    vec3 result = ambient + diffuse + specular;
    FragColor = vec4(result, 1.0);
}