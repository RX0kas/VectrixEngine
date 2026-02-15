#pragma once

#include "Vectrix/Renderer/RendererAPI.h"

namespace Vectrix {

	class VulkanRendererAPI : public RendererAPI
	{
	public:
		void setClearColor(const glm::vec4& color) override;
		bool prepareFrame() override;
		void endFrame() override;

		void drawIndexed(const VertexArray& vertexArray) override;
	private:
		friend class VulkanRenderer;
		static uint32_t getDrawCalls() { return s_drawCalls; }
		static uint32_t s_drawCalls;
	};


}