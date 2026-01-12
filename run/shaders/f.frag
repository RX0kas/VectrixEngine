#version 450

layout(location = 0) in vec3 v_Normal;
layout(location = 1) in vec2 v_TexCoord;

layout(location = 0) out vec4 outColor;

void main()
{
	// Visualisation des normales ([-1,1] → [0,1])
	vec3 n = normalize(v_Normal);
	vec3 color = n * 0.5 + 0.5;

	outColor = vec4(color, 1.0);
}
