#pragma once

#include "Vectrix/Renderer/RendererAPI.h"

namespace Vectrix {

	class VulkanRendererAPI : public RendererAPI
	{
	public:
		void SetClearColor(const glm::vec4& color) override;
		void Clear() override;

		void DrawIndexed(const std::vector<Vertex>& vertexArray) override;
	};


}