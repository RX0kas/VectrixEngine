#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable
#extension GL_EXT_nonuniform_qualifier : enable

layout(set = 0, binding = 0) readonly buffer FrameSSBO {
	mat4 vc_cameraTransform;
} frame;

layout(binding = 1) uniform sampler2D u_Textures[8];

layout(location = 0) in vec3 v_Normal;
layout(location = 1) in vec2 v_TexCoord;
layout(location = 2) in flat uint v_TextureIndex;

layout(location = 0) out vec4 outColor;

void main() {
	outColor = texture(u_Textures[nonuniformEXT(v_TextureIndex)], v_TexCoord);
}
