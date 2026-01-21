#version 450

layout(location = 0) in vec3 v_Normal;
layout(location = 1) in vec2 v_TexCoord;

layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 0) readonly buffer FrameSSBO {
	float time;
} frame;

void main()
{
	// Visualisation des normales ([-1,1] → [0,1])
	vec3 n = normalize(v_Normal);
	vec3 color = n * 0.5 + 0.5;

	outColor = vec4(mix(color,vec3(0),(sin(frame.time)/2)+0.5), 1.0);
}
