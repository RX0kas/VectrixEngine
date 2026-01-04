#pragma once

#include "Vectrix/Renderer/RendererAPI.h"

namespace Vectrix {

	class VulkanRendererAPI : public RendererAPI
	{
	public:
		void setupScene() override;
		void sendScene() override;
		void SetClearColor(const glm::vec4& color) override;
		void Clear() override;

		void DrawIndexed(const std::shared_ptr<VertexArray>& vertexArray) override;
	};


}