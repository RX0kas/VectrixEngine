#include "ComputeShader.h"

#include "GraphicAPI/Vulkan/VulkanComputeShader.h"
#include "Vectrix/Rendering/Renderer.h"
#include "Vectrix/Utils/Memory.h"

namespace Vectrix {
	std::shared_ptr<ComputeShader> ComputeShader::create(const std::string &name, const std::string &path) {
		return createFromSource(name, readUTF8(path));
	}

	std::shared_ptr<ComputeShader> ComputeShader::createFromSource(const std::string &name, const std::string &source) {
		switch (Renderer::getAPI()) {
			case RendererAPI::API::None:    VC_CORE_ERROR("RendererAPI::None is currently not supported!"); return nullptr;
			case RendererAPI::API::Vulkan: {
				auto shader = std::make_shared<VulkanComputeShader>(name, source);
				if (!shader->isValid()) {
					VC_CORE_ERROR_NO_EXIT("Compute shader {} failed to compile", name);
					return nullptr;
				}
				return shader;
			}
		}

		VC_CORE_ERROR("Unknown RendererAPI!");
		return nullptr;
	}
} // Vectrix