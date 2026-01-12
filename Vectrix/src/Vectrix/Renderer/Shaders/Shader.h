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
		~Shader();

		void bind() const;
		void unbind();
	private:
		Shader(const std::string& name, const std::string& vertexPath, const std::string& fragmentPath, BufferLayout layout);
		void createPipelineLayout();
		void createPipeline(VkRenderPass renderPass, const std::string& vertexSrc, const std::string& fragmentSrc, BufferLayout layout);

		Device& _device;

		std::unique_ptr<Pipeline> _pipeline;
		VkPipelineLayout _pipelineLayout;

		bool _enable = true;
		friend class VulkanContext;
		friend class ShaderManager;
		friend class Renderer;
		// Info
		const char* name;
	};

}
