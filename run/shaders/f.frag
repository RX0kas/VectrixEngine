#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(set = 0, binding = 0) readonly buffer FrameSSBO {
	float time;
	mat4 vc_cameraTransform;
} frame;

layout(push_constant) uniform PushConstants {
	mat4 vc_modelMat;
	uint vc_textureIndex;
} pc;

layout(binding = 1) uniform sampler2D u_Textures[8];

layout(location = 0) in vec3 v_Normal;
layout(location = 1) in vec2 v_TexCoord;

layout(location = 0) out vec4 outColor;

void main()
{
	outColor = texture(u_Textures[pc.vc_textureIndex], v_TexCoord);
}
