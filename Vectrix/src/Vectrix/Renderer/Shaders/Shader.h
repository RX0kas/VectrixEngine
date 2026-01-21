#pragma once

#include <string>

#include "ShaderUniformLayout.h"
#include "Vectrix/Renderer/Buffer.h"

namespace Vectrix {
	class Shader {
		public:
		virtual ~Shader() = default;

		virtual void bind() const = 0;
		virtual void unbind() const = 0;
		virtual void setUniform1i(const std::string& name,int value) const = 0;
		virtual void setUniform1f(const std::string& name,float value) const = 0;
		virtual void setUniform2f(const std::string& name,glm::vec2 value) const = 0;
		virtual void setUniform3f(const std::string& name,glm::vec3 value) const = 0;
		virtual void setUniform4f(const std::string& name,glm::vec4 value) const = 0;
	private:
		friend class ShaderManager;
		static Shader* create(const std::string& name, const std::string& vertexPath, const std::string& fragmentPath,const ShaderUniformLayout& layout,const BufferLayout& buffer_layout);
	};
}
