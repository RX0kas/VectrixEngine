#version 450

layout(push_constant) uniform Push {
	mat4 transform; // view * proj
	mat4 modelMat; // Model Matrix
} push;

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec2 a_TexCoord;

layout(location = 0) out vec3 v_Normal;
layout(location = 1) out vec2 v_TexCoord;

void main()
{
	v_Normal = a_Normal;
	v_TexCoord = a_TexCoord;
	gl_Position = push.transform * push.modelMat * vec4(a_Position, 1.0);
}