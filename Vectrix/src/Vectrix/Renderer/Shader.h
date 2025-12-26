#pragma once

#include "GraphicAPI/Vulkan/rendering/Pipeline.h"

#include "Vectrix/Renderer/Buffer.h"

#include <glm/glm.hpp>

#include <string>
#include <memory>
#include <vector>

namespace Vectrix {
	// TODO: Make a Vulkan class for it
	class Shader
	{
	public:
		Shader(const std::string& vertexSrc, const std::string& fragmentSrc, BufferLayout layout);
		~Shader();

		void bind() const;
		void unbind();

		void uploadUniformMat4(const std::string& name, const glm::mat4& matrix);

	private:
		void createPipelineLayout();
		void createPipeline(VkRenderPass renderPass, const std::string& vertexSrc, const std::string& fragmentSrc, BufferLayout layout);

		Device& _device;

		std::unique_ptr<Pipeline> _pipeline;
		VkPipelineLayout _pipelineLayout;

		bool _enable = true;
		friend class VulkanContext;
	};

}
