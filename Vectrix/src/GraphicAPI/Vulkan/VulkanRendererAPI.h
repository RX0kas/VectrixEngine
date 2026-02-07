#pragma once

#include "Vectrix/Renderer/RendererAPI.h"

namespace Vectrix {

	class VulkanRendererAPI : public RendererAPI
	{
	public:
		bool setupScene() override;
		void sendScene() override;
		void SetClearColor(const glm::vec4& color) override;
		void Clear() override;

		void DrawIndexed(const VertexArray& vertexArray) override;
	private:
		friend class VulkanRenderer;
		static uint32_t s_drawCalls;
	};


}