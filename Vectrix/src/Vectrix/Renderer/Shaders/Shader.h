#pragma once

#include <string>

#include "ShaderUniformLayout.h"
#include "Vectrix/Renderer/Buffer.h"
#include "Vectrix/Renderer/Camera/PerspectiveCamera.h"

#define VC_VERIFY_UNIFORM_NAME(name) if (name=="cameraTransform" || name=="modelMat") VC_ERROR("The uniform name \"{}\" is reserved",name)

namespace Vectrix {
	class Shader {
		public:
		virtual ~Shader() = default;

		virtual void bind() const = 0;
		virtual void unbind() const = 0;
		virtual void sentCameraUniform(const PerspectiveCamera& camera) const = 0;
		virtual void setUniformBool(const std::string& name,bool value) const = 0;
		virtual void setUniform1i(const std::string& name,int value) const = 0;
		virtual void setUniform1u(const std::string& name,unsigned int value) const = 0;
		virtual void setUniform1f(const std::string& name,float value) const = 0;
		virtual void setUniform2f(const std::string& name,glm::vec2 value) const = 0;
		virtual void setUniform3f(const std::string& name,glm::vec3 value) const = 0;
		virtual void setUniform4f(const std::string& name,glm::vec4 value) const = 0;
		virtual void setUniformMat4f(const std::string& name,glm::mat4 value) const = 0;
		virtual void setModelMatrix(const glm::mat4& model) const = 0;
	private:
		friend class ShaderManager;
		static Shader* create(const std::string& name, const std::string& vertexPath, const std::string& fragmentPath,ShaderUniformLayout layout,const BufferLayout& buffer_layout,bool affectedByCamera);
	};
}
