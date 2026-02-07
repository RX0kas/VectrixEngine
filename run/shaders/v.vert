#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(set = 0, binding = 0) buffer FrameSSBO {
	float time;
	mat4 cameraTransform;
	mat4 modelMat;
} frame;

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec2 a_TexCoord;

layout(location = 0) out vec3 v_Normal;
layout(location = 1) out vec2 v_TexCoord;

void main() {
	v_Normal = a_Normal;
	v_TexCoord = a_TexCoord;
	gl_Position = frame.cameraTransform * frame.modelMat * vec4(a_Position, 1.0);
}