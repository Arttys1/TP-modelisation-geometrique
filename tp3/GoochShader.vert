#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;

out vec2 TexCoord;
out vec3 FragPos;
out vec3 Normal;

uniform mat4 MVP;
uniform mat4 MODEL;

void main()
{
    FragPos = vec3(MODEL * vec4(aPos, 1.0));
    Normal = mat3(transpose(inverse(MODEL))) * aNormal;
	gl_Position = MVP * vec4(aPos, 1.0f);
	TexCoord = aTexCoord;
}