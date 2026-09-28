#pragma once

#include "Vectrix/Rendering/RendererAPI.h"

namespace Vectrix {

	class VulkanRendererAPI : public RendererAPI
	{
	public:
		void setClearColor(const glm::vec4& color) override;
		bool canRender() override;
		void beginFrame() override;
		void endFrame() override;
		void sendFrame() override;
	};


}