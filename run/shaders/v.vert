#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable
#extension GL_EXT_nonuniform_qualifier : enable

layout(set = 0, binding = 0) readonly buffer FrameSSBO {
	float time;
	mat4 vc_cameraTransform;
} frame;

struct ObjectData {
	mat4 modelMatrix;
	uint textureIndex;
};

layout(set = 1, binding = 0) readonly buffer ObjectSSBO {
	ObjectData objects[];
} objectBuffer;

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec2 a_TexCoord;

layout(location = 0) out vec3 v_Normal;
layout(location = 1) out vec2 v_TexCoord;
layout(location = 2) out flat uint v_TextureIndex;

void main() {
	ObjectData obj = objectBuffer.objects[gl_InstanceIndex];

	v_Normal = a_Normal;
	v_TexCoord = a_TexCoord;
	v_TextureIndex = obj.textureIndex;

	gl_Position = frame.vc_cameraTransform * obj.modelMatrix * vec4(a_Position, 1.0);
}